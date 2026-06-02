#include "Commands.h"

#include "../builder/Builder.h"
#include "../fetcher/Cache.h"
#include "../fetcher/GitFetcher.h"
#include "../manifest/Lockfile.h"
#include "../manifest/Manifest.h"
#include "../resolver/MVSResolver.h"
#include "../util/Checksum.h"
#include "../util/Error.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
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

// Load manifest + lockfile, auto-lock if stale. Returns exit code.
int load_project(Manifest& manifest, Lockfile& lockfile) {
    auto toml_path = find_toml();
    manifest       = Manifest::parse(toml_path);
    auto lock_path = toml_path.parent_path() / "fly.lock";
    lockfile       = Lockfile::read(lock_path);

    if (lockfile.is_stale(toml_path)) {
        std::cout << "fly.lock is out of date, re-locking...\n";
        int rc = do_lock(toml_path, manifest, true);
        if (rc != 0) return rc;
        lockfile = Lockfile::read(lock_path);
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
        f << "# Sections that use single brackets [ ] appear exactly once (e.g. [package]).\n";
        f << "# Sections that use double brackets [[ ]] define an *array of tables*:\n";
        f << "# you may repeat [[bin]], [[lib]], or [[test]] as many times as you need,\n";
        f << "# each occurrence adds one target. Example:\n";
        f << "#\n";
        f << "#   [[bin]]\n";
        f << "#   name = \"app\"\n";
        f << "#   path = \"src/main.fly\"\n";
        f << "#\n";
        f << "#   [[bin]]\n";
        f << "#   name = \"cli\"\n";
        f << "#   path = \"src/cli.fly\"\n";
        f << "#\n";
        f << "# If no [[bin]] or [[lib]] is declared, flyp auto-detects src/main.fly\n";
        f << "# and src/lib.fly when they exist.\n";
        f << "\n";
        f << "[package]\n";
        f << "name        = \"" << pkg_name << "\"\n";
        f << "version     = \"" << pkg_ver  << "\"\n";
        f << "description = \"\"\n";
        f << "license     = \"\"\n";
        f << "fly-version = \"0.1.0\"\n";
        f << "\n";
        f << "[profile.debug]\n";
        f << "opt-level  = 0\n";
        f << "debug-info = true\n";
        f << "assertions = true\n";
        f << "\n";
        f << "[profile.release]\n";
        f << "opt-level  = 3\n";
        f << "debug-info = false\n";
        f << "lto        = false\n";
        f << "strip      = false\n";
        f << "assertions = false\n";
    }

    std::cout << "created " << (dir / "fly.toml").string() << "\n";
    std::cout << "created " << (src_dir / "main.fly").string() << "\n";
    return 0;
}

// ── cmd_lock ─────────────────────────────────────────────────────────────────

int cmd_lock() {
    auto toml_path = find_toml();
    auto root      = Manifest::parse(toml_path);
    return do_lock(toml_path, root, true);
}

// ── cmd_build ────────────────────────────────────────────────────────────────

int cmd_build(bool release, const std::string& target) {
    Manifest manifest;
    Lockfile lockfile;
    int rc = load_project(manifest, lockfile);
    if (rc != 0) return rc;

    BuildMode mode = release ? BuildMode::Release : BuildMode::Debug;
    Builder builder(manifest, lockfile, mode);

    bool ok;
    if (target.empty()) {
        ok = builder.build_all();
    } else {
        // Try bin first, then lib
        ok = builder.build_bin(target);
        if (!ok) ok = builder.build_lib(target);
    }
    return ok ? 0 : 1;
}

// ── cmd_run ──────────────────────────────────────────────────────────────────

int cmd_run(bool release, const std::string& bin_name,
            const std::vector<std::string>& run_args)
{
    Manifest manifest;
    Lockfile lockfile;
    int rc = load_project(manifest, lockfile);
    if (rc != 0) return rc;

    BuildMode mode = release ? BuildMode::Release : BuildMode::Debug;
    Builder builder(manifest, lockfile, mode);

    // Determine which bin to run
    std::string name = bin_name;
    if (name.empty()) {
        if (manifest.bins.empty()) {
            std::cerr << "error: no [[bin]] targets defined\n";
            return 1;
        }
        name = manifest.bins[0].name;
    }

    if (!builder.build_bin(name)) return 1;
    return builder.run_bin(name, run_args) ? 0 : 1;
}

// ── cmd_test ─────────────────────────────────────────────────────────────────

int cmd_test(bool release, const std::string& suite) {
    Manifest manifest;
    Lockfile lockfile;
    int rc = load_project(manifest, lockfile);
    if (rc != 0) return rc;

    BuildMode mode = release ? BuildMode::Release : BuildMode::Debug;
    Builder builder(manifest, lockfile, mode);
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
