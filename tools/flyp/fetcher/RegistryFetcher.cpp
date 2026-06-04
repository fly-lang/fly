#include "RegistryFetcher.h"
#include "../resolver/VersionConstraint.h"
#include "../util/Checksum.h"

#include <array>
#include <cstdio>
#include <filesystem>
#include <regex>
#include <sstream>
#include <stdexcept>

#ifdef _WIN32
#  define popen  _popen
#  define pclose _pclose
#endif

namespace flyp {

namespace {

std::string shell_quote(const std::string& s) {
    std::string out = "'";
    for (char c : s) {
        if (c == '\'') out += "'\\''";
        else           out += c;
    }
    return out + "'";
}

// Parse a JSON array of version strings from the registry list endpoint.
// Registry returns: ["1.0.0","1.1.0","2.0.0"]
std::vector<std::string> parse_version_array(const std::string& json) {
    std::vector<std::string> result;
    size_t pos = 0;
    while ((pos = json.find('"', pos)) != std::string::npos) {
        size_t end = json.find('"', pos + 1);
        if (end == std::string::npos) break;
        std::string v = json.substr(pos + 1, end - pos - 1);
        if (!v.empty() && std::isdigit(static_cast<unsigned char>(v[0])))
            result.push_back(v);
        pos = end + 1;
    }
    return result;
}

} // anonymous namespace

// ── HTTP helpers (via system curl) ───────────────────────────────────────────

std::string RegistryFetcher::http_get(const std::string& url) const {
    std::string cmd = "curl -fsSL " + shell_quote(url) + " 2>/dev/null";
    std::array<char, 256> buf;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe{popen(cmd.c_str(), "r"), pclose};
    if (!pipe) throw std::runtime_error("curl: popen failed for " + url);
    while (fgets(buf.data(), buf.size(), pipe.get()))
        result += buf.data();
    return result;
}

void RegistryFetcher::http_download(const std::string& url,
                                    const std::filesystem::path& dest) const {
    std::string cmd = "curl -fsSL -o " + shell_quote(dest.string())
                    + " " + shell_quote(url) + " 2>/dev/null";
    if (std::system(cmd.c_str()) != 0)
        throw std::runtime_error("download failed: " + url);
}

void RegistryFetcher::http_post_file(const std::string& url,
                                     const std::filesystem::path& file,
                                     const std::string& token) const {
    std::string cmd = "curl -fsSL -X POST"
                    " -H " + shell_quote("Content-Type: application/octet-stream");
    if (!token.empty())
        cmd += " -H " + shell_quote("Authorization: Bearer " + token);
    cmd += " --data-binary @" + shell_quote(file.string())
         + " " + shell_quote(url) + " 2>/dev/null";
    if (std::system(cmd.c_str()) != 0)
        throw std::runtime_error("publish failed (check token if auth is required): " + url);
}

// ── Version resolution ────────────────────────────────────────────────────────

std::vector<std::string> RegistryFetcher::list_versions(
    const std::string& registry_url, const std::string& pkg_name) const
{
    std::string url = registry_url + "/v1/" + pkg_name;
    std::string body = http_get(url);
    return parse_version_array(body);
}

std::string RegistryFetcher::resolve_version(const std::string& registry_url,
                                              const std::string& pkg_name,
                                              const std::string& version_req) const {
    // Exact version — no need to query list.
    static const std::regex exact_re{R"(\d+\.\d+\.\d+)"};
    if (std::regex_match(version_req, exact_re))
        return version_req;

    auto versions = list_versions(registry_url, pkg_name);
    if (versions.empty())
        throw std::runtime_error("registry: no versions found for '" + pkg_name + "'");

    // Select the highest version that matches the requirement.
    // For now: treat any version_req that isn't exact as "latest".
    // Full semver range matching would go here.
    SemVer best{0, 0, 0};
    std::string best_str;
    for (const auto& v : versions) {
        auto sv = SemVer::parse(v);
        if (sv > best) { best = sv; best_str = v; }
    }
    if (best_str.empty())
        throw std::runtime_error("registry: no matching version for '"
                                 + pkg_name + "' req='" + version_req + "'");
    return best_str;
}

// ── fetch ─────────────────────────────────────────────────────────────────────

FetchResult RegistryFetcher::fetch(const std::string& alias, const GitDep& dep,
                                   const std::string& registry_url) {
    const std::string& pkg_name = dep.pkg_name.empty() ? alias : dep.pkg_name;

    std::string version = resolve_version(registry_url, pkg_name, dep.version_req);

    // Cache key: host/pkg_name/version
    if (cache_.has(registry_url, pkg_name + "/" + version)) {
        auto entry     = cache_.entry_path(registry_url, pkg_name + "/" + version);
        auto toml_p    = entry / "fly.toml";
        std::string cs = std::filesystem::exists(toml_p) ? sha256_file(toml_p) : "sha256:";
        return {version, version, entry, cs}; // rev=version, version, local_path, checksum
    }

    // Download tarball.
    std::string download_url = registry_url + "/v1/" + pkg_name + "/" + version + "/download";
    cache_.prepare(registry_url, pkg_name + "/" + version);
    auto entry = cache_.entry_path(registry_url, pkg_name + "/" + version);

    auto tarball = entry / (pkg_name + "-" + version + ".tar.gz");
    http_download(download_url, tarball);

    // Extract.
    std::string extract = "tar -xzf " + shell_quote(tarball.string())
                        + " -C " + shell_quote(entry.string())
                        + " 2>/dev/null";
    if (std::system(extract.c_str()) != 0)
        throw std::runtime_error("registry: failed to extract tarball for " + pkg_name);

    std::filesystem::remove(tarball); // keep cache clean

    auto toml_p = entry / "fly.toml";
    std::string cs = std::filesystem::exists(toml_p) ? sha256_file(toml_p) : "sha256:";
    return {version, version, entry, cs}; // rev=version, version, local_path, checksum
}

// ── publish ───────────────────────────────────────────────────────────────────

void RegistryFetcher::publish(const std::string& pkg_name, const std::string& version,
                              const std::filesystem::path& tarball_path,
                              const std::string& registry_url,
                              const std::string& token) const {
    std::string url = registry_url + "/v1/" + pkg_name + "/" + version;
    http_post_file(url, tarball_path, token);
}

// ── read_manifest ─────────────────────────────────────────────────────────────

Manifest RegistryFetcher::read_manifest(const std::string& alias,
                                         const std::filesystem::path& entry) const {
    auto toml_path = entry / "fly.toml";
    if (!std::filesystem::exists(toml_path))
        throw std::runtime_error("no fly.toml in registry package '" + alias + "'");
    return Manifest::parse(toml_path);
}

} // namespace flyp
