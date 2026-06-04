#include "Commands.h"

#include "../builder/Builder.h"
#include "../fetcher/Cache.h"
#include "../fetcher/GitFetcher.h"
#include "../manifest/Lockfile.h"
#include "../manifest/Manifest.h"
#include "../resolver/MVSResolver.h"
#include "../util/Checksum.h"
#include "../util/Error.h"

#include <algorithm>
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
    Cache      cache;
    GitFetcher fetcher(cache);

    std::map<std::string, FetchResult>           fetch_results;
    std::map<std::string, std::vector<std::string>> pkg_deps;
    std::vector<ResolveError>                    extra_errors;

    auto fetch_manifest = [&](const std::string& name,
                               const GitDep& dep) -> std::optional<Manifest>
    {
        try {
            if (verbose)
                std::cout << "  fetching " << name << " (" << dep.ref.value << ")...\n";
            auto result = fetcher.fetch(name, dep);
            fetch_results[name] = result;
            auto m = fetcher.read_manifest(name, result.local_path);
            std::vector<std::string> dn;
            for (auto& [k, _] : m.dependencies)   dn.push_back(k);
            for (auto& [k, _] : m.dev_dependencies) {} // dev deps not propagated
            pkg_deps[name] = std::move(dn);
            return m;
        } catch (const std::exception& e) {
            ResolveError err;
            err.is_conflict = false;
            err.package     = name;
            err.ref_kind    = (dep.ref.kind == GitRefKind::Tag    ? "tag"    :
                               dep.ref.kind == GitRefKind::Branch ? "branch" : "rev");
            err.ref_value   = dep.ref.value;
            err.git_url     = dep.git_url;
            try { err.available = fetcher.list_tags(dep.git_url); } catch (...) {}
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
        p.source  = "git+" + rp.git_url;
        p.tag     = (rp.ref.kind == GitRefKind::Tag ? rp.ref.value : "");

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

// ── stubs ────────────────────────────────────────────────────────────────────

int cmd_search(const std::string& query) {
    std::cout << "flyp search: package registry not yet available\n";
    std::cout << "  (searched for: " << query << ")\n";
    return 0;
}

int cmd_publish() {
    std::cout << "flyp publish: package registry not yet available\n";
    return 0;
}

} // namespace flyp::commands
