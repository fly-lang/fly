#include "include/RegistryHandler.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <vector>

namespace flyp::registry {

// ── Response ──────────────────────────────────────────────────────────────────

std::string Response::to_http() const {
    const char* msg = status == 200 ? "OK"
                    : status == 201 ? "Created"
                    : status == 404 ? "Not Found"
                    : status == 400 ? "Bad Request"
                    : "Internal Server Error";
    std::ostringstream s;
    s << "HTTP/1.1 " << status << " " << msg << "\r\n"
      << "Content-Type: " << content_type << "\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "Connection: close\r\n\r\n"
      << body;
    return s.str();
}

// ── RegistryHandler ───────────────────────────────────────────────────────────

RegistryHandler::RegistryHandler(const std::filesystem::path& storage,
                                 const std::string& token)
    : storage_(storage), token_(token) {
    std::filesystem::create_directories(storage_);
}

bool RegistryHandler::check_auth(const Request& req) const {
    if (token_.empty()) return true; // auth disabled
    auto it = req.headers.find("Authorization");
    if (it == req.headers.end()) return false;
    const std::string& hdr = it->second;
    // Accept "Bearer <token>" (case-sensitive).
    const std::string prefix = "Bearer ";
    if (hdr.size() <= prefix.size()) return false;
    if (hdr.substr(0, prefix.size()) != prefix) return false;
    return hdr.substr(prefix.size()) == token_;
}

// ── Storage helpers ───────────────────────────────────────────────────────────

std::filesystem::path RegistryHandler::pkg_dir(const std::string& name) const {
    return storage_ / name;
}

std::filesystem::path RegistryHandler::tarball_path(const std::string& name,
                                                     const std::string& version) const {
    return storage_ / name / (version + ".tar.gz");
}

std::filesystem::path RegistryHandler::toml_path(const std::string& name,
                                                  const std::string& version) const {
    return storage_ / name / version / "fly.toml";
}

// ── URL decode ────────────────────────────────────────────────────────────────

std::string RegistryHandler::url_decode(const std::string& s) {
    std::string result;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            int v = 0;
            std::istringstream ss(s.substr(i + 1, 2));
            if (ss >> std::hex >> v) { result += static_cast<char>(v); i += 2; }
            else result += s[i];
        } else if (s[i] == '+') {
            result += ' ';
        } else {
            result += s[i];
        }
    }
    return result;
}

// ── JSON helper ───────────────────────────────────────────────────────────────

std::string RegistryHandler::json_string_array(const std::vector<std::string>& vec) {
    std::ostringstream s;
    s << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i) s << ",";
        s << "\"" << vec[i] << "\"";
    }
    s << "]";
    return s.str();
}

// ── extract_toml ──────────────────────────────────────────────────────────────

void RegistryHandler::extract_toml(const std::string& name,
                                   const std::string& version) const {
    auto tball = tarball_path(name, version);
    auto tdir  = storage_ / name / version;
    std::filesystem::create_directories(tdir);
    // Try two common tarball structures: with and without leading ./
    std::string cmd =
        "tar -xzf '" + tball.string() + "'"
        " -C '" + tdir.string() + "'"
        " --wildcards '*/fly.toml' --strip-components=1 2>/dev/null"
        " || tar -xzf '" + tball.string() + "'"
        " -C '" + tdir.string() + "' ./fly.toml 2>/dev/null"
        " || tar -xzf '" + tball.string() + "'"
        " -C '" + tdir.string() + "' fly.toml 2>/dev/null";
    std::system(cmd.c_str());
}

// ── Endpoint handlers ─────────────────────────────────────────────────────────

Response RegistryHandler::handle_list_versions(const std::string& name) const {
    auto dir = pkg_dir(name);
    if (!std::filesystem::exists(dir))
        return {404, "text/plain", "package not found"};

    std::vector<std::string> versions;
    for (const auto& e : std::filesystem::directory_iterator(dir)) {
        if (!e.is_regular_file()) continue;
        std::string fn = e.path().filename().string();
        if (fn.size() > 7 && fn.substr(fn.size() - 7) == ".tar.gz")
            versions.push_back(fn.substr(0, fn.size() - 7));
    }
    std::sort(versions.begin(), versions.end());
    return {200, "application/json", json_string_array(versions)};
}

Response RegistryHandler::handle_get_metadata(const std::string& name,
                                               const std::string& version) const {
    auto tp = toml_path(name, version);
    if (!std::filesystem::exists(tp)) {
        if (std::filesystem::exists(tarball_path(name, version)))
            extract_toml(name, version);
    }
    if (!std::filesystem::exists(tp))
        return {404, "text/plain", "version not found"};

    std::ifstream f(tp);
    std::string body((std::istreambuf_iterator<char>(f)),
                      std::istreambuf_iterator<char>());
    return {200, "text/plain", body};
}

Response RegistryHandler::handle_download(const std::string& name,
                                          const std::string& version) const {
    auto tb = tarball_path(name, version);
    if (!std::filesystem::exists(tb))
        return {404, "text/plain", "tarball not found"};

    std::ifstream f(tb, std::ios::binary);
    std::string body((std::istreambuf_iterator<char>(f)),
                      std::istreambuf_iterator<char>());
    return {200, "application/octet-stream", body};
}

Response RegistryHandler::handle_publish(const std::string& name,
                                         const std::string& version,
                                         const std::string& body) const {
    if (version.empty())
        return {400, "text/plain", "version required: POST /v1/{name}/{version}"};
    if (body.empty())
        return {400, "text/plain", "empty body"};

    std::filesystem::create_directories(pkg_dir(name));
    auto tb = tarball_path(name, version);
    std::ofstream f(tb, std::ios::binary);
    if (!f) return {500, "text/plain", "cannot write tarball"};
    f.write(body.data(), static_cast<std::streamsize>(body.size()));
    f.close();
    extract_toml(name, version);
    return {201, "text/plain", "published"};
}

Response RegistryHandler::handle_search(const std::string& query) const {
    std::vector<std::string> matches;
    if (std::filesystem::exists(storage_)) {
        for (const auto& e : std::filesystem::directory_iterator(storage_)) {
            if (!e.is_directory()) continue;
            const std::string name = e.path().filename().string();
            if (query.empty() || name.find(query) != std::string::npos)
                matches.push_back(name);
        }
        std::sort(matches.begin(), matches.end());
    }
    return {200, "application/json", json_string_array(matches)};
}

// ── Main dispatch ─────────────────────────────────────────────────────────────

Response RegistryHandler::handle(const Request& req) const {
    // GET /v1/search?q=...
    if (req.method == "GET" && req.path == "/v1/search") {
        std::string query;
        auto pos = req.query.find("q=");
        if (pos != std::string::npos)
            query = url_decode(req.query.substr(pos + 2));
        return handle_search(query);
    }

    // Parse /v1/{name}[/{version}[/download]]
    static const std::regex route_re{
        R"(/v1/([^/?#]+)(?:/([^/?#]+)(?:/(download))?)?)"};
    std::smatch m;
    if (!std::regex_match(req.path, m, route_re))
        return {404, "text/plain", "not found"};

    std::string name    = m[1].str();
    std::string version = m[2].str();
    std::string action  = m[3].str();

    if (req.method == "GET" && version.empty())
        return handle_list_versions(name);

    if (req.method == "GET" && action.empty())
        return handle_get_metadata(name, version);

    if (req.method == "GET" && action == "download")
        return handle_download(name, version);

    if (req.method == "POST" && action.empty()) {
        if (!check_auth(req))
            return {401, "text/plain",
                    requires_auth()
                        ? "Unauthorized — provide 'Authorization: Bearer <token>'"
                        : "Unauthorized"};
        return handle_publish(name, version, req.body);
    }

    return {400, "text/plain", "bad request"};
}

} // namespace flyp::registry
