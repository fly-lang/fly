#pragma once

#include "../manifest/Lockfile.h"
#include "../manifest/Manifest.h"
#include <string>
#include <vector>

namespace flyp {

enum class BuildMode { Debug, Release };

class Builder {
public:
    Builder(const Manifest& manifest,
            const Lockfile& lockfile,
            BuildMode       mode = BuildMode::Debug);

    // Build all [[bin]] and [[lib]] targets.
    // Returns false if any target fails.
    bool build_all();

    // Build a single bin target by name.
    bool build_bin(const std::string& name);

    // Build a single lib target by name.
    bool build_lib(const std::string& name);

    // Build all [[test]] targets and run them.
    // If suite_name is non-empty, runs only that suite.
    bool run_tests(const std::string& suite_name = "");

    // Run a built bin (must have been built first).
    bool run_bin(const std::string& name,
                 const std::vector<std::string>& args = {});

private:
    const Manifest& manifest_;
    const Lockfile& lockfile_;
    BuildMode       mode_;

    // Build output directory: <root>/target/debug or target/release
    std::filesystem::path out_dir() const;

    // Compiler flags derived from the active profile.
    std::vector<std::string> profile_flags() const;

    // Collect -I include paths from locked packages.
    std::vector<std::string> include_paths() const;

    // Invoke the fly compiler. Returns true on success.
    bool invoke_fly(const std::string& source,
                    const std::string& output,
                    const std::vector<std::string>& extra_flags = {}) const;
};

} // namespace flyp
