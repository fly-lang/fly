#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace flyp::registry {

struct Request {
    std::string method;
    std::string path;
    std::string query;
    std::map<std::string, std::string> headers;
    std::string body;
};

struct Response {
    int         status       = 200;
    std::string content_type = "text/plain";
    std::string body;

    std::string to_http() const;
};

// Core HTTP handler — pure logic, no sockets.
// All operations are relative to a storage directory.
// If token is non-empty, write operations (POST) require
// the header "Authorization: Bearer <token>".
// Read operations (GET) are always public.
class RegistryHandler {
public:
    explicit RegistryHandler(const std::filesystem::path& storage,
                             const std::string& token = "");

    // Dispatch a parsed HTTP request and return a Response.
    Response handle(const Request& req) const;

    const std::filesystem::path& storage() const { return storage_; }

    // Storage layout helpers (public for testing).
    std::filesystem::path pkg_dir(const std::string& name) const;
    std::filesystem::path tarball_path(const std::string& name,
                                       const std::string& version) const;
    std::filesystem::path toml_path(const std::string& name,
                                    const std::string& version) const;

    // URL-decode a percent-encoded string.
    static std::string url_decode(const std::string& s);

    // True if the registry requires authentication for writes.
    bool requires_auth() const { return !token_.empty(); }

private:
    std::filesystem::path storage_;
    std::string           token_; // empty = no auth

    // Returns true if the request carries a valid Bearer token.
    bool check_auth(const Request& req) const;

    // Individual endpoint handlers.
    Response handle_list_versions (const std::string& name) const;
    Response handle_get_metadata  (const std::string& name,
                                   const std::string& version) const;
    Response handle_download      (const std::string& name,
                                   const std::string& version) const;
    Response handle_publish       (const std::string& name,
                                   const std::string& version,
                                   const std::string& body) const;
    Response handle_search        (const std::string& query) const;

    // Extract fly.toml from a stored tarball into <storage>/<name>/<version>/fly.toml
    void extract_toml(const std::string& name, const std::string& version) const;

    // JSON helpers.
    static std::string json_string_array(const std::vector<std::string>& vec);
};

} // namespace flyp::registry
