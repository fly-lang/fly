#pragma once

#include <filesystem>
#include <string>

namespace flyp {

// Manages ~/.flyp/cache/<host>/<owner>/<repo>/<rev40>/
class Cache {
public:
    Cache();

    // Root cache directory (respects $FLYP_HOME).
    const std::filesystem::path& root() const { return root_; }

    // Returns the cache path for a given rev (may not exist yet).
    std::filesystem::path entry_path(const std::string& git_url,
                                     const std::string& rev40) const;

    // True if the cache entry exists and is a non-empty directory.
    bool has(const std::string& git_url, const std::string& rev40) const;

    // Ensure the parent directories for an entry exist.
    void prepare(const std::string& git_url, const std::string& rev40) const;

    // Parse git URL into host/owner/repo segments for directory layout.
    static std::tuple<std::string,std::string,std::string>
    parse_url(const std::string& git_url);

    // Remove all cache entries (flyp cache clean).
    void clean() const;

    // Print cache statistics to stdout.
    void stats() const;

private:
    std::filesystem::path root_;
};

} // namespace flyp
