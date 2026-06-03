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
    std::string git_url;
    GitRef      ref;
};

// ── Build targets ─────────────────────────────────────────────────────────────

struct BinTarget {
    std::string name;
    std::string path; // relative to project root
};

struct LibTarget {
    std::string name;
    std::string path;
    std::string type; // "static" | "dynamic" | "both"
};

struct TestTarget {
    std::string name;
    std::string path;
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
    std::string                    version;    // semver MAJOR.MINOR.PATCH
    std::vector<std::string>       authors;
    std::string                    description;
    std::string                    license;
    std::string                    fly_version;
    std::optional<std::string>     homepage;
    std::optional<std::string>     repository;

    // targets
    std::vector<BinTarget>         bins;
    std::vector<LibTarget>         libs;
    std::vector<TestTarget>        tests;
    TestConfig                     test_config;

    // dependencies
    std::map<std::string, GitDep>  dependencies;
    std::map<std::string, GitDep>  dev_dependencies;

    // profiles
    BuildProfile                   profile_debug;
    BuildProfile                   profile_release;

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
