#include "Commands.h"

#include "../builder/Builder.h"
#include "../fetcher/Cache.h"
#include "../fetcher/GitFetcher.h"
#include "../fetcher/RegistryFetcher.h"
#include "../manifest/Lockfile.h"
#include "../manifest/Manifest.h"
#include "../resolver/MVSResolver.h"
#include "../resolver/VersionConstraint.h"
#include "../util/Checksum.h"
#include "../util/Error.h"

#ifdef _WIN32
#  define popen  _popen
#  define pclose _pclose
#endif

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace flyp::commands {

namespace {

static std::string shell_quote(const std::string& s) {
    std::string out = "'";
    for (char c : s) {
        if (c == '\'') out += "'\\''";
        else           out += c;
    }
    return out + "'";
}

// ── helpers ─────────────────────────────────────────────────────────────────

// Find fly.toml upward from CWD. Throws if not found.
std::filesystem::path find_toml() {
    auto dir = std::filesystem::current_path();
    while (true) {
        auto p = dir / "fly.toml";
        if (std::filesystem::exists(p)) return p;
        auto parent = dir.parent_path();
        if (parent == dir) break;
        dir = parent;
    }
    throw std::runtime_error("could not find fly.toml in current directory or any parent");
}

// ── Workspace helpers ────────────────────────────────────────────────────────

// Load all member manifests of a workspace root.
// Returns members in declaration order (topological sort applied later).
std::vector<Manifest> load_workspace_members(const Manifest& root) {
    std::vector<Manifest> members;
    for (const auto& rel : root.workspace->members) {
        auto member_toml = root.root_dir / rel / "fly.toml";
        if (!std::filesystem::exists(member_toml))
            throw std::runtime_error(
                "workspace member not found: " + member_toml.string());
        members.push_back(Manifest::parse(member_toml));
    }
    return members;
}

// Topological sort of workspace members based on path dependencies.
// Returns member indices in build order. Throws on cycles.
std::vector<size_t> workspace_topo_sort(const std::vector<Manifest>& members) {
    const size_t n = members.size();

    // Map member root_dir → index.
    std::map<std::filesystem::path, size_t> dir_to_idx;
    for (size_t i = 0; i < n; ++i)
        dir_to_idx[members[i].root_dir] = i;

    // Build adjacency list: edges[i] = members that i depends on.
    std::vector<std::vector<size_t>> deps(n);
    for (size_t i = 0; i < n; ++i) {
        for (const auto& [name, dep] : members[i].dependencies) {
            if (!dep.is_path_dep()) continue;
            auto resolved = std::filesystem::canonical(
                members[i].root_dir / dep.path);
            auto it = dir_to_idx.find(resolved);
            if (it != dir_to_idx.end())
                deps[i].push_back(it->second);
        }
    }

    // Kahn's algorithm.
    std::vector<size_t> in_degree(n, 0);
    for (size_t i = 0; i < n; ++i)
        for (auto d : deps[i]) ++in_degree[d]; // d must be built before i

    // Actually: if i depends on j, j must come first → edge i→j means j before i.
    // in_degree[i] = number of members that must be built before i.
    std::vector<size_t> in_deg(n, 0);
    std::vector<std::vector<size_t>> rev(n); // rev[j] = members waiting on j
    for (size_t i = 0; i < n; ++i)
        for (auto j : deps[i]) { in_deg[i]++; rev[j].push_back(i); }

    std::vector<size_t> queue, order;
    for (size_t i = 0; i < n; ++i)
        if (in_deg[i] == 0) queue.push_back(i);

    while (!queue.empty()) {
        size_t cur = queue.back(); queue.pop_back();
        order.push_back(cur);
        for (auto nxt : rev[cur])
            if (--in_deg[nxt] == 0) queue.push_back(nxt);
    }

    if (order.size() != n)
        throw std::runtime_error(
            "workspace: circular path dependency detected");

    return order;
}

// Return the output directory of path-dep members that `m` depends on.
std::vector<std::filesystem::path> member_extra_includes(
    const Manifest& m,
    const std::vector<Manifest>& members,
    const std::string& profile)
{
    std::vector<std::filesystem::path> includes;
    std::map<std::filesystem::path, size_t> dir_to_idx;
    for (size_t i = 0; i < members.size(); ++i)
        dir_to_idx[members[i].root_dir] = i;

    for (const auto& [name, dep] : m.dependencies) {
        if (!dep.is_path_dep()) continue;
        auto resolved = std::filesystem::weakly_canonical(m.root_dir / dep.path);
        auto it = dir_to_idx.find(resolved);
        if (it != dir_to_idx.end())
            includes.push_back(members[it->second].root_dir / "target" / profile);
    }
    return includes;
}

// Aggregate all git dependencies from a set of manifests (for workspace lock).
Manifest make_workspace_virtual_root(const Manifest& ws_root,
                                     const std::vector<Manifest>& members) {
    Manifest virt = ws_root;
    virt.targets.clear(); // virtual root has no targets of its own
    for (const auto& m : members) {
        for (const auto& [name, dep] : m.dependencies)
            if (!dep.is_path_dep() && !virt.dependencies.count(name))
                virt.dependencies[name] = dep;
        for (const auto& [name, dep] : m.dev_dependencies)
            if (!dep.is_path_dep() && !virt.dev_dependencies.count(name))
                virt.dev_dependencies[name] = dep;
    }
    return virt;
}

// Perform full resolution + fetch + lockfile write. Returns exit code.
int do_lock(const std::filesystem::path& toml_path,
            const Manifest& root,
            bool verbose = false)
{
    Cache           cache;
    GitFetcher      git_fetcher(cache);
    RegistryFetcher reg_fetcher(cache);

    std::map<std::string, FetchResult>           fetch_results;
    std::map<std::string, std::vector<std::string>> pkg_deps;
    std::vector<ResolveError>                    extra_errors;

    // Helper: resolve registry alias → URL using root manifest's [repo] map.
    auto resolve_registry_url = [&](const std::string& alias) -> std::string {
        auto it = root.repos.find(alias);
        if (it == root.repos.end())
            throw std::runtime_error("unknown registry alias '" + alias
                                     + "' — declare it in [repo]");
        return it->second;
    };

    auto fetch_manifest = [&](const std::string& name,
                               const GitDep& dep) -> std::optional<Manifest>
    {
        try {
            FetchResult result;
            if (dep.is_registry_dep()) {
                if (verbose)
                    std::cout << "  fetching " << name
                              << " from registry '" << dep.registry_name
                              << "' version=" << dep.version_req << "...\n";
                auto reg_url = resolve_registry_url(dep.registry_name);
                result = reg_fetcher.fetch(name, dep, reg_url);
                fetch_results[name] = result;
                auto m = reg_fetcher.read_manifest(name, result.local_path);
                std::vector<std::string> dn;
                for (auto& [k, _] : m.dependencies) dn.push_back(k);
                pkg_deps[name] = std::move(dn);
                return m;
            } else {
                if (verbose)
                    std::cout << "  fetching " << name
                              << " (" << dep.ref.value << ")...\n";
                result = git_fetcher.fetch(name, dep);
                fetch_results[name] = result;
                auto m = git_fetcher.read_manifest(name, result.local_path);
                std::vector<std::string> dn;
                for (auto& [k, _] : m.dependencies) dn.push_back(k);
                pkg_deps[name] = std::move(dn);
                return m;
            }
        } catch (const std::exception& e) {
            ResolveError err;
            err.is_conflict = false;
            err.package     = name;
            if (dep.is_registry_dep()) {
                err.ref_kind  = "version";
                err.ref_value = dep.version_req;
                err.git_url   = dep.registry_name; // reuse field for display
            } else {
                err.ref_kind  = (dep.ref.kind == GitRefKind::Tag    ? "tag"    :
                                 dep.ref.kind == GitRefKind::Branch ? "branch" : "rev");
                err.ref_value = dep.ref.value;
                err.git_url   = dep.git_url;
                try { err.available = git_fetcher.list_tags(dep.git_url); } catch (...) {}
            }
            extra_errors.push_back(std::move(err));
            return std::nullopt;
        }
    };

    MVSResolver resolver;
    auto resolved = resolver.resolve(root, fetch_manifest);

    // Collect and report all errors
    auto all_errors = resolver.resolve_errors();
    for (auto& e : extra_errors) all_errors.push_back(e);

    if (!all_errors.empty()) {
        for (const auto& e : all_errors) {
            FlypError fe;
            if (e.is_conflict) {
                fe = make_conflict(e.package, e.conflict_chain);
            } else {
                fe = make_not_found(e.package, e.ref_kind, e.ref_value,
                                    e.git_url, e.available);
            }
            report(fe);
        }
        return 1;
    }

    // Build lockfile
    Lockfile lf;
    lf.format_version = 1;
    lf.flyp_version   = FLYP_VERSION;
    lf.toml_checksum  = sha256_file(toml_path);

    for (auto& [name, rp] : resolved) {
        LockedPackage p;
        p.name    = name;
        p.version = rp.version;
        // source: "git+<url>" for git deps, "registry+<url>/<pkg>/<ver>" for registry
        if (rp.git_url.empty()) {
            // Registry dep — reconstruct source from fetch result path hint
            // We use rev field to store the resolved version for registry deps
            p.source = "registry+" + name + "/" + rp.version;
        } else {
            p.source = "git+" + rp.git_url;
        }
        p.tag = (rp.ref.kind == GitRefKind::Tag ? rp.ref.value : "");

        if (auto it = fetch_results.find(name); it != fetch_results.end()) {
            p.rev      = it->second.rev;
            p.checksum = it->second.checksum;
        }
        if (auto it = pkg_deps.find(name); it != pkg_deps.end())
            p.deps = it->second;

        lf.packages.push_back(std::move(p));
    }

    auto lock_path = toml_path.parent_path() / "fly.lock";
    lf.write(lock_path);
    std::cout << "  wrote " << lock_path.string() << "\n";
    return 0;
}

// Verify every locked package is present in cache. Returns exit code.
int check_offline_cache(const Lockfile& lockfile) {
    Cache cache;
    bool ok = true;
    for (const auto& pkg : lockfile.packages) {
        // source is stored as "git+<url>"
        const std::string url = pkg.source.substr(4);
        auto entry = cache.entry_path(url, pkg.rev);
        if (!std::filesystem::exists(entry) || std::filesystem::is_empty(entry)) {
            std::cerr << "error [offline]: package '" << pkg.name
                      << "' (" << pkg.rev.substr(0, 8) << ") is not in cache.\n"
                      << "  Run `flyp lock` (without --offline) to fetch it first.\n";
            ok = false;
        }
    }
    return ok ? 0 : 1;
}

// Load manifest + lockfile. In offline mode skip re-locking and verify cache.
int load_project(Manifest& manifest, Lockfile& lockfile, bool offline = false) {
    auto toml_path = find_toml();
    manifest       = Manifest::parse(toml_path);
    auto lock_path = toml_path.parent_path() / "fly.lock";
    lockfile       = Lockfile::read(lock_path);

    if (lockfile.is_stale(toml_path)) {
        if (offline) {
            std::cerr << "warning [offline]: fly.lock is out of date but network "
                      << "access is disabled — using stale lockfile.\n";
        } else {
            std::cout << "fly.lock is out of date, re-locking...\n";
            int rc = do_lock(toml_path, manifest, true);
            if (rc != 0) return rc;
            lockfile = Lockfile::read(lock_path);
        }
    }

    if (offline) {
        int rc = check_offline_cache(lockfile);
        if (rc != 0) return rc;
    }
    return 0;
}

} // anonymous namespace

// ── cmd_version ──────────────────────────────────────────────────────────────

int cmd_version() {
    std::cout << "flyp " << FLYP_VERSION << "\n";
    return 0;
}

// ── cmd_init ─────────────────────────────────────────────────────────────────

int cmd_init(const std::string& name_arg, const std::string& version_arg) {
    std::filesystem::path dir = std::filesystem::current_path();

    std::string pkg_name = name_arg.empty()
        ? dir.filename().string()
        : name_arg;
    std::string pkg_ver  = version_arg.empty() ? "0.1.0" : version_arg;

    // If a name was given and differs from CWD, create subdirectory.
    if (!name_arg.empty() && name_arg != dir.filename().string()) {
        dir = dir / name_arg;
        std::filesystem::create_directories(dir);
    }

    auto toml_path = dir / "fly.toml";
    if (std::filesystem::exists(toml_path)) {
        std::cerr << "error: fly.toml already exists in " << dir.string() << "\n";
        return 3;
    }

    // Create src/main.fly
    auto src_dir = dir / "src";
    std::filesystem::create_directories(src_dir);
    {
        std::ofstream f(src_dir / "main.fly");
        f << "// Hello, Fly!\n";
        f << "import fly.io\n\n";
        f << "main() {\n";
        f << "    io.println(\"Hello, world!\")\n";
        f << "}\n";
    }

    // Write fly.toml
    {
        std::ofstream f(toml_path);
        f << "# fly.toml — Fly project manifest\n";
        f << "#\n";
        f << "# [targets] is a key→value table. The key is the target ID used by the CLI\n";
        f << "# (flyp build --target key, flyp run --bin key). A target without a 'lib'\n";
        f << "# field is a binary; adding lib = \"static\" or lib = \"dynamic\" makes it a lib.\n";
        f << "#\n";
        f << "#   [targets]\n";
        f << "#   app    = { path = \"src/main.fly\" }                    # bin\n";
        f << "#   mylib  = { path = \"src/lib.fly\", lib = \"static\" }    # lib\n";
        f << "#\n";
        f << "# If no [targets] is declared, flyp auto-detects src/main.fly and src/lib.fly.\n";
        f << "\n";
        f << "[package]\n";
        f << "name        = \"" << pkg_name << "\"\n";
        f << "version     = \"" << pkg_ver  << "\"\n";
        f << "description = \"\"\n";
        f << "license     = \"\"\n";
        f << "fly-version = \"0.1.0\"\n";
        f << "\n";
        f << "[profiles]\n";
        f << "debug   = { opt-level = 0, debug-info = true,  assertions = true,  lto = false, strip = false }\n";
        f << "release = { opt-level = 3, debug-info = false, assertions = false, lto = false, strip = false }\n";
    }

    std::cout << "created " << (dir / "fly.toml").string() << "\n";
    std::cout << "created " << (src_dir / "main.fly").string() << "\n";
    return 0;
}

// ── cmd_workspace_init ───────────────────────────────────────────────────────

int cmd_workspace_init(const std::string& name,
                       const std::vector<std::string>& members) {
    std::filesystem::path ws_dir = std::filesystem::current_path();

    if (!name.empty() && name != ws_dir.filename().string()) {
        ws_dir = ws_dir / name;
        std::filesystem::create_directories(ws_dir);
    }

    auto ws_toml = ws_dir / "fly.toml";
    if (std::filesystem::exists(ws_toml)) {
        std::cerr << "error: fly.toml already exists in " << ws_dir.string() << "\n";
        return 3;
    }

    // Create workspace root fly.toml.
    {
        std::ofstream f(ws_toml);
        f << "# fly.toml — Fly workspace root\n\n";
        f << "[workspace]\n";
        f << "members = [";
        for (size_t i = 0; i < members.size(); ++i) {
            if (i) f << ", ";
            f << "\"" << members[i] << "\"";
        }
        f << "]\n";
    }
    std::cout << "created workspace: " << ws_toml.string() << "\n";

    // Create each member package.
    for (const auto& m : members) {
        auto member_dir = ws_dir / m;
        std::filesystem::create_directories(member_dir / "src");
        auto member_toml = member_dir / "fly.toml";
        if (!std::filesystem::exists(member_toml)) {
            std::ofstream f(member_toml);
            f << "[package]\n";
            f << "name        = \"" << m << "\"\n";
            f << "version     = \"0.1.0\"\n";
            f << "description = \"\"\n";
            f << "license     = \"\"\n";
            f << "fly-version = \"0.1.0\"\n";
            f << "\n[profiles]\n";
            f << "debug   = { opt-level = 0, debug-info = true,  assertions = true,  lto = false, strip = false }\n";
            f << "release = { opt-level = 3, debug-info = false, assertions = false, lto = false, strip = false }\n";
            std::cout << "created member: " << member_toml.string() << "\n";
        }
        auto main_fly = member_dir / "src" / "main.fly";
        if (!std::filesystem::exists(main_fly)) {
            std::ofstream f(main_fly);
            f << "// " << m << "\n";
            f << "import fly.io\n\nmain() {\n    io.println(\"Hello from " << m << "!\")\n}\n";
        }
    }

    return 0;
}

// ── cmd_lock ─────────────────────────────────────────────────────────────────

int cmd_lock() {
    auto toml_path = find_toml();
    auto root      = Manifest::parse(toml_path);
    return do_lock(toml_path, root, true);
}

// ── cmd_clean ────────────────────────────────────────────────────────────────

int cmd_clean(const std::string& profile) {
    auto toml_path  = find_toml();
    auto root_dir   = toml_path.parent_path();
    auto target_dir = root_dir / "target";

    if (!std::filesystem::exists(target_dir)) {
        std::cout << "nothing to clean (target/ does not exist)\n";
        return 0;
    }

    if (profile.empty()) {
        std::filesystem::remove_all(target_dir);
        std::cout << "removed " << target_dir.string() << "\n";
    } else {
        auto profile_dir = target_dir / profile;
        if (!std::filesystem::exists(profile_dir)) {
            std::cout << "nothing to clean for profile '" << profile << "'"
                      << " (target/" << profile << "/ does not exist)\n";
            return 0;
        }
        std::filesystem::remove_all(profile_dir);
        std::cout << "removed " << profile_dir.string() << "\n";
    }
    return 0;
}

// ── cmd_build ────────────────────────────────────────────────────────────────

int cmd_build(const std::string& profile, const std::vector<std::string>& targets,
              int jobs, bool offline, const std::string& triple) {
    Manifest manifest;
    Lockfile lockfile;
    int rc = load_project(manifest, lockfile, offline);
    if (rc != 0) return rc;

    // ── Workspace build ──────────────────────────────────────────────────────
    if (manifest.is_workspace_root()) {
        auto members = load_workspace_members(manifest);
        auto order   = workspace_topo_sort(members);

        // Re-lock using aggregated deps if stale.
        auto toml_path = manifest.root_dir / "fly.toml";
        if (lockfile.is_stale(toml_path) && !offline) {
            auto virt = make_workspace_virtual_root(manifest, members);
            rc = do_lock(toml_path, virt, true);
            if (rc != 0) return rc;
            lockfile = Lockfile::read(toml_path.parent_path() / "fly.lock");
        }

        bool ok = true;
        for (auto idx : order) {
            const auto& m = members[idx];
            if (!m.profiles.count(profile)) {
                // Member may not have this profile declared; use defaults.
            }
            std::cout << "  building member: " << m.name << "\n";
            auto includes = member_extra_includes(m, members, profile);
            Builder builder(m, lockfile, profile, false, jobs, triple, includes);
            bool r = builder.build_all(jobs);
            ok = r && ok;
            if (!r) break;
        }
        return ok ? 0 : 1;
    }

    // ── Single-package build ─────────────────────────────────────────────────
    if (!manifest.profiles.count(profile)) {
        std::cerr << "error: unknown profile '" << profile << "'. Available:";
        for (auto& [k, _] : manifest.profiles) std::cerr << " " << k;
        std::cerr << "\n";
        return 1;
    }

    if (!triple.empty())
        std::cout << "  cross-compiling for " << triple << "\n";

    Builder builder(manifest, lockfile, profile, false, jobs, triple);
    bool ok = targets.empty() ? builder.build_all(jobs)
                               : builder.build_subset(targets, jobs);
    return ok ? 0 : 1;
}

// ── cmd_run ──────────────────────────────────────────────────────────────────

int cmd_run(const std::string& profile, const std::string& bin_key,
            const std::vector<std::string>& run_args, bool offline)
{
    Manifest manifest;
    Lockfile lockfile;
    int rc = load_project(manifest, lockfile, offline);
    if (rc != 0) return rc;

    if (!manifest.profiles.count(profile)) {
        std::cerr << "error: unknown profile '" << profile << "'. Available:";
        for (auto& [k, _] : manifest.profiles) std::cerr << " " << k;
        std::cerr << "\n";
        return 1;
    }

    Builder builder(manifest, lockfile, profile);

    std::string key = bin_key;
    if (key.empty()) {
        auto it = std::find_if(manifest.targets.begin(), manifest.targets.end(),
                               [](const Target& t){ return t.is_bin(); });
        if (it == manifest.targets.end()) {
            std::cerr << "error: no bin targets defined in [targets]\n";
            return 1;
        }
        key = it->key;
    } else {
        auto it = std::find_if(manifest.targets.begin(), manifest.targets.end(),
                               [&](const Target& t){ return t.key == key; });
        if (it == manifest.targets.end()) {
            std::cerr << "error: no target named '" << key << "'\n";
            return 1;
        }
        if (it->is_lib()) {
            std::cerr << "error: '" << key << "' is a library and cannot be run\n";
            return 1;
        }
    }

    if (!builder.build_target(key)) return 1;
    return builder.run_target(key, run_args) ? 0 : 1;
}

// ── cmd_test ─────────────────────────────────────────────────────────────────

int cmd_test(const std::string& profile, const std::string& suite, bool offline) {
    Manifest manifest;
    Lockfile lockfile;
    int rc = load_project(manifest, lockfile, offline);
    if (rc != 0) return rc;

    // ── Workspace test ───────────────────────────────────────────────────────
    if (manifest.is_workspace_root()) {
        auto members = load_workspace_members(manifest);
        auto order   = workspace_topo_sort(members);
        bool ok = true;
        for (auto idx : order) {
            const auto& m = members[idx];
            if (m.test_config.suites.empty()) continue;
            std::cout << "  testing member: " << m.name << "\n";
            auto includes = member_extra_includes(m, members, profile);
            Builder builder(m, lockfile, profile, false, 0, "", includes);
            bool r = builder.run_tests(suite);
            ok = r && ok;
        }
        return ok ? 0 : 1;
    }

    // ── Single-package test ──────────────────────────────────────────────────
    if (!manifest.profiles.count(profile)) {
        std::cerr << "error: unknown profile '" << profile << "'. Available:";
        for (auto& [k, _] : manifest.profiles) std::cerr << " " << k;
        std::cerr << "\n";
        return 1;
    }

    Builder builder(manifest, lockfile, profile);
    return builder.run_tests(suite) ? 0 : 1;
}

// ── cmd_add ──────────────────────────────────────────────────────────────────

int cmd_add(const std::string& name,
            const std::string& git_url,
            const std::string& tag,
            const std::string& branch,
            const std::string& rev,
            bool dev)
{
    auto toml_path = find_toml();
    auto manifest  = Manifest::parse(toml_path);

    // Validate: exactly one ref
    int ref_count = (tag.empty() ? 0 : 1)
                  + (branch.empty() ? 0 : 1)
                  + (rev.empty() ? 0 : 1);
    if (ref_count != 1) {
        report(make_args("specify exactly one of --tag, --branch, or --rev"));
        return 3;
    }

    GitDep dep;
    dep.git_url = git_url;
    if      (!tag.empty())    dep.ref = {GitRefKind::Tag,    tag};
    else if (!branch.empty()) dep.ref = {GitRefKind::Branch, branch};
    else                      dep.ref = {GitRefKind::Rev,    rev};

    if (dev) manifest.dev_dependencies[name] = dep;
    else     manifest.dependencies[name]     = dep;
    manifest.save();

    std::cout << "added " << name << " to fly.toml\n";

    // Re-lock
    return do_lock(toml_path, manifest, true);
}

// ── cmd_remove ───────────────────────────────────────────────────────────────

int cmd_remove(const std::string& name) {
    auto toml_path = find_toml();
    auto manifest  = Manifest::parse(toml_path);

    bool found = manifest.dependencies.count(name)
              || manifest.dev_dependencies.count(name);
    if (!found) {
        std::cerr << "error: '" << name << "' is not a dependency\n";
        return 1;
    }

    manifest.remove_dependency(name);
    std::cout << "removed " << name << " from fly.toml\n";

    // Re-lock (with remaining deps)
    return do_lock(toml_path, manifest, true);
}

// ── cmd_update ───────────────────────────────────────────────────────────────

int cmd_update(const std::string& name) {
    auto toml_path = find_toml();
    auto manifest  = Manifest::parse(toml_path);

    // Delete the lock file (or the relevant package entry) and re-lock.
    auto lock_path = toml_path.parent_path() / "fly.lock";

    if (!name.empty()) {
        // Partial update: remove cached entry for this package to force re-fetch.
        auto lf = Lockfile::read(lock_path);
        if (auto* p = lf.find(name)) {
            Cache cache;
            auto entry = cache.entry_path(p->source.substr(4), p->rev);
            std::filesystem::remove_all(entry);
            std::cout << "cleared cache for " << name << "\n";
        }
    } else {
        // Full update: wipe lock file.
        std::filesystem::remove(lock_path);
        std::cout << "cleared fly.lock, re-resolving all dependencies\n";
    }

    return do_lock(toml_path, manifest, true);
}

// ── cmd_upgrade ──────────────────────────────────────────────────────────────

int cmd_upgrade(const std::string& name, bool dry_run) {
    auto toml_path = find_toml();
    auto manifest  = Manifest::parse(toml_path);

    Cache      cache;
    GitFetcher fetcher(cache);

    int updated = 0;

    auto try_upgrade = [&](const std::string& dep_name, GitDep& dep) {
        if (!name.empty() && dep_name != name) return;

        if (dep.is_path_dep()) {
            std::cout << "  skip (path dep):    " << dep_name << "\n";
            return;
        }
        if (dep.is_registry_dep()) {
            std::cout << "  skip (registry dep): " << dep_name << "\n";
            return;
        }
        if (dep.ref.kind == GitRefKind::Rev) {
            std::cout << "  skip (pinned rev):  " << dep_name << "\n";
            return;
        }
        if (dep.ref.kind == GitRefKind::Branch) {
            std::cout << "  skip (branch — use flyp update): " << dep_name << "\n";
            return;
        }

        // Tag dep: find the highest semver tag on the remote.
        std::vector<std::string> tags;
        try { tags = fetcher.list_tags(dep.git_url); }
        catch (...) {
            std::cerr << "  warning: could not list tags for " << dep_name << "\n";
            return;
        }

        SemVer current  = SemVer::parse(dep.ref.value);
        SemVer best     = current;
        std::string best_tag = dep.ref.value;
        for (const auto& t : tags) {
            auto sv = SemVer::parse(t);
            if (sv > best) { best = sv; best_tag = t; }
        }

        if (best_tag == dep.ref.value) {
            std::cout << "  up to date: " << dep_name << " " << dep.ref.value << "\n";
            return;
        }

        std::cout << "  " << (dry_run ? "[dry-run] " : "")
                  << "upgrading: " << dep_name
                  << "  " << dep.ref.value << " → " << best_tag << "\n";

        if (!dry_run) {
            dep.ref.value = best_tag;
            ++updated;
        }
    };

    for (auto& [n, d] : manifest.dependencies)     try_upgrade(n, d);
    for (auto& [n, d] : manifest.dev_dependencies) try_upgrade(n, d);

    if (dry_run) {
        std::cout << "(dry-run: no changes written)\n";
        return 0;
    }
    if (updated == 0) {
        std::cout << "all dependencies are up to date\n";
        return 0;
    }

    manifest.save();
    std::cout << "updated " << updated << " dependency/dependencies — re-locking...\n";
    return do_lock(toml_path, manifest, true);
}

// ── cmd_doctor ────────────────────────────────────────────────────────────────

int cmd_doctor() {
    bool all_ok = true;

    // Helper to run a command and capture first line of stdout.
    auto capture = [](const std::string& cmd) -> std::string {
        std::array<char,256> buf{};
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> p{
            popen((cmd + " 2>/dev/null").c_str(), "r"), pclose};
        if (p && fgets(buf.data(), buf.size(), p.get()))
            result = buf.data();
        while (!result.empty() &&
               (result.back() == '\n' || result.back() == '\r'))
            result.pop_back();
        return result;
    };

    auto chk  = [&](bool ok, const std::string& msg) {
        std::cout << "  " << (ok ? "✓" : "✗") << " " << msg << "\n";
        if (!ok) all_ok = false;
    };
    auto warn = [](const std::string& msg) {
        std::cout << "  ⚠ " << msg << "\n";
    };

    // ── Compiler ──────────────────────────────────────────────────────────────
    std::cout << "Compiler:\n";
    std::string fly_ver = capture("fly --version");
    chk(!fly_ver.empty(), fly_ver.empty()
        ? "fly compiler not found in PATH"
        : "fly compiler: " + fly_ver);

    // ── flyp ──────────────────────────────────────────────────────────────────
    std::cout << "\nflyp:\n";
    std::cout << "  ✓ flyp " << FLYP_VERSION << "\n";

    // ── Manifest ──────────────────────────────────────────────────────────────
    std::cout << "\nManifest:\n";
    std::filesystem::path toml_path;
    Manifest manifest;
    bool has_manifest = false;
    try {
        toml_path    = find_toml();
        manifest     = Manifest::parse(toml_path);
        has_manifest = true;
        chk(true, "fly.toml: " + toml_path.string());
    } catch (...) {
        warn("no fly.toml found — run flyp init to create one");
    }

    if (has_manifest) {
        // fly-version compatibility
        if (!manifest.fly_version.empty() && !fly_ver.empty()) {
            auto required = SemVer::parse(manifest.fly_version);
            auto actual   = SemVer::parse(fly_ver);
            chk(actual >= required,
                actual >= required
                    ? "fly-version satisfied (" + fly_ver + " >= " + manifest.fly_version + ")"
                    : "fly-version mismatch: have " + fly_ver
                      + ", need >= " + manifest.fly_version);
        }

        // ── Lockfile ──────────────────────────────────────────────────────────
        std::cout << "\nLockfile:\n";
        auto lock_path = toml_path.parent_path() / "fly.lock";
        auto lockfile  = Lockfile::read(lock_path);

        if (!std::filesystem::exists(lock_path)) {
            if (manifest.dependencies.empty() && manifest.dev_dependencies.empty()) {
                std::cout << "  ✓ no dependencies — fly.lock not needed\n";
            } else {
                chk(false, "fly.lock missing — run flyp lock");
            }
        } else if (lockfile.is_stale(toml_path)) {
            chk(false, "fly.lock is stale — run flyp lock");
        } else {
            chk(true, "fly.lock up to date ("
                + std::to_string(lockfile.packages.size()) + " packages)");
        }

        // ── Cache ─────────────────────────────────────────────────────────────
        if (!lockfile.packages.empty()) {
            std::cout << "\nCache:\n";
            Cache cache;
            std::cout << "  ✓ cache root: " << cache.root().string() << "\n";
            int missing = 0;
            for (const auto& pkg : lockfile.packages) {
                std::string url = pkg.source.rfind("git+", 0) == 0
                    ? pkg.source.substr(4) : pkg.source.substr(9);
                auto entry = cache.entry_path(url, pkg.rev);
                if (!std::filesystem::exists(entry)) {
                    warn("not cached: " + pkg.name + " " + pkg.version);
                    ++missing;
                }
            }
            if (missing == 0)
                std::cout << "  ✓ all " << lockfile.packages.size()
                          << " packages in cache\n";
            else
                chk(false, std::to_string(missing)
                    + " package(s) missing from cache — run flyp lock");
        }

        // ── Registries ────────────────────────────────────────────────────────
        if (!manifest.repos.empty()) {
            std::cout << "\nRegistries:\n";
            for (const auto& [alias, url] : manifest.repos) {
                std::string probe = capture("curl -fsSL --connect-timeout 3 "
                                            + shell_quote(url + "/v1/search?q="));
                bool reachable = !probe.empty();
                chk(reachable,
                    reachable ? alias + " reachable: " + url
                              : alias + " unreachable: " + url);
            }
        }
    }

    std::cout << "\n" << (all_ok ? "✓ all checks passed\n" : "⚠ some issues found\n");
    return all_ok ? 0 : 1;
}

// ── cmd_why ──────────────────────────────────────────────────────────────────

int cmd_why(const std::string& name) {
    auto toml_path = find_toml();
    auto root      = Manifest::parse(toml_path);

    Cache      cache;
    GitFetcher fetcher(cache);

    auto fetch_manifest = [&](const std::string& n,
                               const GitDep& dep) -> std::optional<Manifest>
    {
        try {
            auto result = fetcher.fetch(n, dep);
            return fetcher.read_manifest(n, result.local_path);
        } catch (...) {
            return std::nullopt;
        }
    };

    MVSResolver resolver;
    auto resolved = resolver.resolve(root, fetch_manifest);

    if (!resolver.resolve_errors().empty()) {
        std::cerr << "error: resolution failed, run `flyp lock` for details\n";
        return 1;
    }

    std::string version = "0.0.0";
    if (auto it = resolved.find(name); it != resolved.end())
        version = it->second.version;

    resolver.graph().print_why(name, version);
    return 0;
}

// ── cmd_cache_clean / stats ──────────────────────────────────────────────────

int cmd_cache_clean() {
    Cache cache;
    cache.clean();
    return 0;
}

int cmd_cache_stats() {
    Cache cache;
    cache.stats();
    return 0;
}

// ── cmd_deploy ───────────────────────────────────────────────────────────────

int cmd_deploy(const std::string& registry_alias, const std::string& version_override,
               const std::string& token) {
    auto toml_path = find_toml();
    auto manifest  = Manifest::parse(toml_path);

    // Resolve alias → URL
    std::string alias = registry_alias.empty() ? manifest.default_registry : registry_alias;
    if (alias.empty()) {
        std::cerr << "error: no registry specified. Use --registry or set"
                     " 'registry' in [package].\n";
        return 1;
    }
    auto it = manifest.repos.find(alias);
    if (it == manifest.repos.end()) {
        std::cerr << "error: unknown registry alias '" << alias
                  << "'. Declare it in [repo].\n";
        return 1;
    }
    const std::string& registry_url = it->second;

    std::string version = version_override.empty() ? manifest.version : version_override;
    if (version.empty()) {
        std::cerr << "error: package version not set in [package].\n";
        return 1;
    }

    // Create tarball of project root (exclude target/, .git/, vendor/).
    auto tmp_tarball = std::filesystem::temp_directory_path()
                     / (manifest.name + "-" + version + ".tar.gz");
    std::string tar_cmd =
        "tar -czf " + shell_quote(tmp_tarball.string())
        + " --exclude=./target --exclude=./.git --exclude=./vendor"
        + " -C " + shell_quote(manifest.root_dir.string()) + " .";
    if (std::system(tar_cmd.c_str()) != 0) {
        std::cerr << "error: failed to create tarball\n";
        return 1;
    }

    // Upload.
    Cache           cache;
    RegistryFetcher reg(cache);
    try {
        reg.publish(manifest.name, version, tmp_tarball, registry_url, token);
        std::cout << "deployed " << manifest.name << " " << version
                  << " to '" << alias << "' (" << registry_url << ")\n";
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        std::filesystem::remove(tmp_tarball);
        return 1;
    }
    std::filesystem::remove(tmp_tarball);
    return 0;
}

// ── cmd_vendor ───────────────────────────────────────────────────────────────

int cmd_vendor(const std::string& registry_alias, const std::string& token) {
    auto toml_path = find_toml();
    auto manifest  = Manifest::parse(toml_path);
    auto lock_path = toml_path.parent_path() / "fly.lock";
    auto lockfile  = Lockfile::read(lock_path);

    if (lockfile.packages.empty()) {
        std::cout << "nothing to vendor (fly.lock is empty)\n";
        return 0;
    }

    // Resolve alias → URL
    std::string alias = registry_alias.empty() ? manifest.default_registry : registry_alias;
    if (alias.empty()) {
        std::cerr << "error: no registry specified. Use --registry or set"
                     " 'registry' in [package].\n";
        return 1;
    }
    auto it = manifest.repos.find(alias);
    if (it == manifest.repos.end()) {
        std::cerr << "error: unknown registry alias '" << alias
                  << "'. Declare it in [repo].\n";
        return 1;
    }
    const std::string& registry_url = it->second;

    Cache           cache;
    RegistryFetcher reg(cache);

    bool ok = true;
    for (const auto& pkg : lockfile.packages) {
        // source is "git+<url>" or "registry+…"
        if (pkg.source.rfind("registry+", 0) == 0) {
            std::cout << "  skipping (already registry): " << pkg.name << "\n";
            continue;
        }

        // Cache entry for git dep.
        std::string git_url = pkg.source.substr(4); // strip "git+"
        auto entry = cache.entry_path(git_url, pkg.rev);
        if (!std::filesystem::exists(entry)) {
            std::cerr << "  warning: " << pkg.name
                      << " not in cache — run flyp lock first\n";
            ok = false;
            continue;
        }

        // Package as tarball.
        auto tmp = std::filesystem::temp_directory_path()
                 / (pkg.name + "-" + pkg.version + ".tar.gz");
        std::string tar_cmd =
            "tar -czf " + shell_quote(tmp.string())
            + " --exclude=./target --exclude=./.git"
            + " -C " + shell_quote(entry.string()) + " .";
        if (std::system(tar_cmd.c_str()) != 0) {
            std::cerr << "  error: failed to package " << pkg.name << "\n";
            ok = false;
            continue;
        }

        // Upload.
        try {
            reg.publish(pkg.name, pkg.version, tmp, registry_url, token);
            std::cout << "  vendored: " << pkg.name << " " << pkg.version << "\n";
        } catch (const std::exception& e) {
            std::cerr << "  error: " << pkg.name << ": " << e.what() << "\n";
            ok = false;
        }
        std::filesystem::remove(tmp);
    }

    if (ok) std::cout << "vendor complete: " << lockfile.packages.size()
                      << " packages pushed to '" << alias << "'\n";
    return ok ? 0 : 1;
}

// ── cmd_search / cmd_publish ──────────────────────────────────────────────────

int cmd_search(const std::string& query) {
    auto toml_path = find_toml();
    auto manifest  = Manifest::parse(toml_path);

    // Use default registry if configured.
    std::string alias = manifest.default_registry;
    if (alias.empty() || manifest.repos.empty()) {
        std::cout << "flyp search: no registry configured in [repo] / [package].registry\n";
        return 0;
    }
    auto it = manifest.repos.find(alias);
    if (it == manifest.repos.end()) {
        std::cerr << "error: unknown registry alias '" << alias << "'\n";
        return 1;
    }

    Cache           cache;
    RegistryFetcher reg(cache);
    try {
        std::string url = it->second + "/v1/search?q=" + query;
        std::cout << reg.http_get(url);
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

int cmd_publish() {
    // flyp publish is an alias for flyp deploy using the default registry.
    const char* env = std::getenv("FLYP_TOKEN");
    return cmd_deploy("", "", env ? env : "");
}

} // namespace flyp::commands
