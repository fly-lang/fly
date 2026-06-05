#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace flyp {

// ── Git dependency ref ────────────────────────────────────────────────────────

enum class GitRefKind { Tag, Branch, Rev };

struct GitRef {
    GitRefKind  kind;
    std::string value; // tag name, branch name, or commit hash
};

struct GitDep {
    std::string git_url;       // git dep (empty for path/registry deps)
    GitRef      ref;
    std::string path;          // non-empty = path dependency
    std::string registry_name; // non-empty = registry dependency (alias from [repo])
    std::string pkg_name;      // package name on registry (defaults to map key)
    std::string version_req;   // version requirement for registry deps ("1.0.0", "^1.2")

    bool is_path_dep()     const { return !path.empty(); }
    bool is_registry_dep() const { return !registry_name.empty(); }
    bool is_git_dep()      const { return !git_url.empty(); }
};

// ── Workspace ─────────────────────────────────────────────────────────────────

struct WorkspaceConfig {
    std::vector<std::string> members; // relative paths to member directories
};

// ── Build targets ─────────────────────────────────────────────────────────────

struct Target {
    std::string key;   // TOML map key → CLI lookup ID
    std::string name;  // output filename (defaults to key)
    std::string path;  // relative to project root
    std::string lib;   // "" = bin; "static"/"dynamic"/"both" = lib

    bool is_lib() const { return !lib.empty(); }
    bool is_bin() const { return lib.empty(); }
};

// ── Build hooks ───────────────────────────────────────────────────────────────

struct HooksConfig {
    std::string pre_build;   // shell command run before compilation (empty = none)
    std::string post_build;  // shell command run after successful compilation
};

// [test] plain-table configuration for the suite-based test system
struct TestConfig {
    std::vector<std::string> suites;  // glob patterns or explicit paths
    bool   parallel   = false;
    int    timeout_ms = 0;            // 0 = no timeout
    bool   fail_fast  = false;
};

// ── Build profile ─────────────────────────────────────────────────────────────

struct BuildProfile {
    int  opt_level  = 0;
    bool debug_info = true;
    bool assertions = true;
    bool lto        = false;
    bool strip      = false;
};

// ── Package manifest ──────────────────────────────────────────────────────────

struct Manifest {
    // [package]
    std::string                    name;
    std::string                    version;          // semver MAJOR.MINOR.PATCH
    std::vector<std::string>       authors;
    std::string                    description;
    std::string                    license;
    std::string                    fly_version;
    std::optional<std::string>     homepage;
    std::optional<std::string>     repository;
    std::string                    default_registry; // alias from [repo] for deploy/publish

    // [repo] — named registry aliases: name → URL
    std::map<std::string, std::string> repos;

    // targets
    std::vector<Target>            targets;
    TestConfig                     test_config;
    HooksConfig                    hooks;

    // dependencies
    std::map<std::string, GitDep>  dependencies;
    std::map<std::string, GitDep>  dev_dependencies;

    // profiles — keyed by profile name ("debug", "release", or custom)
    std::map<std::string, BuildProfile> profiles;

    // [link] — native C library dependencies passed as -l flags to the fly linker
    std::vector<std::string> link_libs;  // e.g. ["LLVM-20"] → -lLLVM-20

    // workspace — present if this manifest is a workspace root
    std::optional<WorkspaceConfig> workspace;

    bool is_workspace_root() const { return workspace.has_value(); }

    // Filesystem location of this manifest
    std::filesystem::path          root_dir;

    // Parse fly.toml at the given path. Throws std::runtime_error on failure.
    static Manifest parse(const std::filesystem::path& toml_path);

    // Write back to the same fly.toml (updates [dependencies] section).
    void save() const;

    // Add / remove a dependency by name.
    void add_dependency(const std::string& name, const GitDep& dep);
    void remove_dependency(const std::string& name);
};

} // namespace flyp
