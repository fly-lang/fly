#pragma once

#include "Cache.h"
#include "GitFetcher.h"
#include "../manifest/Manifest.h"
#include <filesystem>
#include <string>
#include <vector>

namespace flyp {

class RegistryFetcher {
public:
    explicit RegistryFetcher(Cache& cache) : cache_(cache) {}

    // Resolve version_req against the registry, download tarball, store in cache.
    // registry_url is the resolved URL (alias already looked up from manifest.repos).
    // Throws std::runtime_error on network or registry failure.
    FetchResult fetch(const std::string& alias, const GitDep& dep,
                      const std::string& registry_url);

    // Upload a tarball to the registry. Used by flyp deploy and flyp vendor.
    // token: API key sent as "Authorization: Bearer <token>" (empty = no auth header).
    void publish(const std::string& pkg_name, const std::string& version,
                 const std::filesystem::path& tarball_path,
                 const std::string& registry_url,
                 const std::string& token = "") const;

    // Read fly.toml from a cached entry.
    Manifest read_manifest(const std::string& alias,
                           const std::filesystem::path& entry) const;

    // List available versions for a package on the registry.
    std::vector<std::string> list_versions(const std::string& registry_url,
                                           const std::string& pkg_name) const;

    // HTTP GET — public so Commands can use it for search.
    std::string http_get(const std::string& url) const;

private:
    Cache& cache_;
    void        http_download(const std::string& url,
                              const std::filesystem::path& dest) const;
    void        http_post_file(const std::string& url,
                               const std::filesystem::path& file,
                               const std::string& token = "") const;

    // Resolve version requirement to exact version using registry data.
    std::string resolve_version(const std::string& registry_url,
                                const std::string& pkg_name,
                                const std::string& version_req) const;
};

} // namespace flyp
