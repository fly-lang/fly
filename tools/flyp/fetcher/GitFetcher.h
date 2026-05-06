#pragma once

#include "Cache.h"
#include "../manifest/Manifest.h"
#include <filesystem>
#include <string>
#include <vector>

namespace flyp {

// Result of a successful fetch.
struct FetchResult {
    std::string           rev;        // full 40-char commit hash
    std::string           version;    // semver parsed from tag, else "0.0.0"
    std::filesystem::path local_path; // cache entry directory
    std::string           checksum;   // sha256: of the fly.toml inside
};

class GitFetcher {
public:
    explicit GitFetcher(Cache& cache) : cache_(cache) {}

    // Clone/fetch the repo, resolve the ref to a commit, store in cache.
    // Throws std::runtime_error on network or git failure.
    FetchResult fetch(const std::string& name, const GitDep& dep);

    // Read fly.toml from a cached entry.
    // Returns a parsed Manifest or throws.
    Manifest read_manifest(const std::string& name,
                           const std::filesystem::path& entry) const;

    // List remote refs (tags/branches) for error messages.
    std::vector<std::string> list_tags(const std::string& git_url) const;
    std::vector<std::string> list_branches(const std::string& git_url) const;

private:
    Cache& cache_;

    // Run a shell command, capture stdout. Throws on non-zero exit.
    static std::string run(const std::string& cmd);

    // Run command, return stdout without throwing (returns "" on error).
    static std::string run_quiet(const std::string& cmd);
};

} // namespace flyp
