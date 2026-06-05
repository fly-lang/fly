#pragma once

#include "../manifest/Lockfile.h"
#include "../manifest/Manifest.h"
#include <mutex>
#include <string>
#include <vector>

namespace flyp {

class Builder {
public:
    Builder(const Manifest& manifest,
            const Lockfile& lockfile,
            const std::string& profile = "debug",
            bool is_test = false,
            int jobs = 0,
            const std::string& target_triple = "",
            std::vector<std::filesystem::path> extra_includes = {});

    // Build all [targets] in parallel.
    // jobs: max concurrent compilations (0 = hardware_concurrency, 1 = sequential).
    bool build_all(int jobs = 0);

    // Build a subset of targets by key (same parallel logic as build_all).
    bool build_subset(const std::vector<std::string>& keys, int jobs = 0);

    // Build a single target by key.
    bool build_target(const std::string& key);

    // Build and run test suites from [test] config.
    // If suite_name is non-empty, runs only that suite.
    bool run_tests(const std::string& suite_name = "");

    // Build and run a bin target by key.
    bool run_target(const std::string& key,
                    const std::vector<std::string>& args = {});

private:
    const Manifest& manifest_;
    const Lockfile& lockfile_;
    std::string     profile_name_;
    bool            is_test_;
    int             jobs_;          // forwarded as --jobs to each fly invocation
    std::string     target_triple_; // cross-compilation target (empty = native)
    std::vector<std::filesystem::path> extra_includes_; // path-dep output dirs
    mutable std::mutex log_mutex_;  // serialises progress lines during parallel builds

    // Build output directory: target/<profile>/ or target/<profile>/<triple>/
    std::filesystem::path out_dir() const;

    // Compiler flags derived from the active profile.
    std::vector<std::string> profile_flags() const;

    // Collect -I include paths from locked packages.
    std::vector<std::string> include_paths() const;

    // Invoke the fly compiler. Returns true on success.
    bool invoke_fly(const std::string& source,
                    const std::string& output,
                    const std::vector<std::string>& extra_flags = {}) const;

    // Run a build hook command with FLYP_* env vars set. Returns true on success.
    bool run_hook(const std::string& cmd, const std::string& phase) const;

    // Incremental build helpers.
    std::string             compute_fingerprint(const Target& t) const;
    std::filesystem::path   fp_path(const Target& t) const;
    bool                    is_up_to_date(const Target& t) const;
    void                    save_fingerprint(const Target& t) const;
};

} // namespace flyp
