#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace flyp {

struct LockedPackage {
    std::string name;
    std::string version;
    std::string source;            // "git+<url>"
    std::string rev;               // full 40-char commit hash
    std::string tag;               // informational only (may be empty)
    std::string checksum;          // "sha256:<hex>"
    std::vector<std::string> deps; // "name version" strings
};

struct Lockfile {
    int         format_version = 1;
    std::string flyp_version;      // version of flyp that wrote this file
    std::string toml_checksum;     // sha256: of the fly.toml input

    std::vector<LockedPackage> packages;

    // Read fly.lock at path. Returns empty Lockfile if file doesn't exist.
    static Lockfile read(const std::filesystem::path& path);

    // Write fly.lock to path, creating/overwriting the file.
    void write(const std::filesystem::path& path) const;

    // True if toml_checksum no longer matches the on-disk fly.toml.
    bool is_stale(const std::filesystem::path& toml_path) const;

    // Find a locked package by name. Returns nullptr if not found.
    const LockedPackage* find(const std::string& name) const;
};

} // namespace flyp
