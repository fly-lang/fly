#include "Cache.h"

#include <cstdlib>
#include <iostream>
#include <regex>
#include <stdexcept>

namespace flyp {

Cache::Cache() {
    const char* home = std::getenv("FLYP_HOME");
    if (home && *home) {
        root_ = std::filesystem::path(home) / "cache";
    } else {
        const char* h = std::getenv("HOME");
        if (!h || !*h) throw std::runtime_error("$HOME not set; set $FLYP_HOME");
        root_ = std::filesystem::path(h) / ".flyp" / "cache";
    }
    std::filesystem::create_directories(root_);
}

std::tuple<std::string,std::string,std::string>
Cache::parse_url(const std::string& git_url) {
    // Strip protocol prefix: https://, http://, git://, git@host:owner/repo
    std::string url = git_url;

    // Normalize git@host:owner/repo → host/owner/repo
    static const std::regex ssh_re{R"(git@([^:]+):(.+))"};
    std::smatch m;
    if (std::regex_match(url, m, ssh_re)) {
        url = m[1].str() + "/" + m[2].str();
    } else {
        // Strip https?:// or git://
        static const std::regex proto_re{R"((?:https?|git)://(.+))"};
        if (std::regex_match(url, m, proto_re)) url = m[1].str();
    }

    // Strip trailing .git
    if (url.size() > 4 && url.substr(url.size()-4) == ".git")
        url = url.substr(0, url.size()-4);

    // Split into parts
    auto p1 = url.find('/');
    std::string host  = (p1 != std::string::npos) ? url.substr(0, p1) : url;
    std::string rest  = (p1 != std::string::npos) ? url.substr(p1+1) : "";
    auto p2 = rest.find('/');
    std::string owner = (p2 != std::string::npos) ? rest.substr(0, p2) : rest;
    std::string repo  = (p2 != std::string::npos) ? rest.substr(p2+1) : rest;

    return {host, owner, repo};
}

std::filesystem::path
Cache::entry_path(const std::string& git_url, const std::string& rev40) const {
    auto [host, owner, repo] = parse_url(git_url);
    return root_ / host / owner / repo / rev40;
}

bool Cache::has(const std::string& git_url, const std::string& rev40) const {
    auto p = entry_path(git_url, rev40);
    return std::filesystem::exists(p) && !std::filesystem::is_empty(p);
}

void Cache::prepare(const std::string& git_url, const std::string& rev40) const {
    std::filesystem::create_directories(entry_path(git_url, rev40));
}

void Cache::clean() const {
    if (std::filesystem::exists(root_)) {
        std::filesystem::remove_all(root_);
        std::filesystem::create_directories(root_);
        std::cout << "Cache cleared: " << root_.string() << "\n";
    }
}

void Cache::stats() const {
    if (!std::filesystem::exists(root_)) {
        std::cout << "Cache is empty.\n";
        return;
    }
    size_t entries = 0;
    uintmax_t total_size = 0;
    for (const auto& e : std::filesystem::recursive_directory_iterator(root_)) {
        if (e.is_regular_file()) {
            ++entries;
            total_size += e.file_size();
        }
    }
    std::cout << "Cache: " << root_.string() << "\n";
    std::cout << "  " << entries << " files, "
              << (total_size / 1024) << " KB\n";
}

} // namespace flyp
