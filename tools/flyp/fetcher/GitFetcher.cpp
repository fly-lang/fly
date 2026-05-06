#include "GitFetcher.h"
#include "../resolver/VersionConstraint.h"
#include "../util/Checksum.h"

#include <array>
#include <cstdio>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>

#ifdef _WIN32
#  define popen  _popen
#  define pclose _pclose
#endif

namespace flyp {

namespace {

// Quote a string for shell. Simple: wrap in single quotes, escape embedded ones.
std::string shell_quote(const std::string& s) {
    std::string out = "'";
    for (char c : s) {
        if (c == '\'') out += "'\\''";
        else           out += c;
    }
    return out + "'";
}

} // anonymous namespace

std::string GitFetcher::run(const std::string& cmd) {
    std::array<char, 256> buf;
    std::string result;
    // Redirect stderr to /dev/null to keep output clean.
    std::string full = cmd + " 2>/dev/null";
    std::unique_ptr<FILE, decltype(&pclose)> pipe{popen(full.c_str(), "r"), pclose};
    if (!pipe) throw std::runtime_error("popen failed: " + cmd);
    while (fgets(buf.data(), buf.size(), pipe.get()))
        result += buf.data();
    // Trim trailing newline
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
        result.pop_back();
    return result;
}

std::string GitFetcher::run_quiet(const std::string& cmd) {
    try { return run(cmd); } catch (...) { return ""; }
}

FetchResult GitFetcher::fetch(const std::string& name, const GitDep& dep) {
    const std::string& url = dep.git_url;
    const GitRef&      ref = dep.ref;

    // Step 1: Resolve the ref to a full commit hash.
    std::string rev40;

    // Use a temporary bare clone in a temp dir to resolve the ref.
    // If the rev is already a 40-char hash we still verify it exists.
    std::string tmp_bare = (std::filesystem::temp_directory_path()
                            / ("flyp-bare-" + name)).string();
    std::filesystem::remove_all(tmp_bare);

    // Clone or init bare repo.
    std::string clone_cmd;
    if (ref.kind == GitRefKind::Tag || ref.kind == GitRefKind::Branch) {
        clone_cmd = "git clone --quiet --bare --depth=1 --branch "
                  + shell_quote(ref.value) + " "
                  + shell_quote(url) + " "
                  + shell_quote(tmp_bare);
    } else {
        // Rev: full clone, then checkout
        clone_cmd = "git clone --quiet --bare "
                  + shell_quote(url) + " "
                  + shell_quote(tmp_bare);
    }

    int rc = std::system(clone_cmd.c_str());
    if (rc != 0) {
        std::filesystem::remove_all(tmp_bare);
        auto avail = list_tags(url);
        throw std::runtime_error(
            "failed to clone " + url + " ref=" + ref.value +
            (avail.empty() ? "" : " (available tags: " + avail[0] + "...)"));
    }

    if (ref.kind == GitRefKind::Rev) {
        // Checkout specific rev
        std::string checkout = "git -C " + shell_quote(tmp_bare)
                             + " rev-parse --verify "
                             + shell_quote(ref.value);
        rev40 = run_quiet(checkout);
        if (rev40.empty() || rev40.size() < 40) {
            std::filesystem::remove_all(tmp_bare);
            throw std::runtime_error("rev " + ref.value + " not found in " + url);
        }
    } else {
        std::string rp = "git -C " + shell_quote(tmp_bare) + " rev-parse HEAD";
        rev40 = run(rp);
    }

    if (rev40.size() != 40) {
        std::filesystem::remove_all(tmp_bare);
        throw std::runtime_error("could not resolve commit hash for " + name);
    }

    // Step 2: Check cache.
    if (cache_.has(url, rev40)) {
        std::filesystem::remove_all(tmp_bare);
        auto entry = cache_.entry_path(url, rev40);
        auto toml_path = entry / "fly.toml";
        std::string checksum = std::filesystem::exists(toml_path)
            ? sha256_file(toml_path) : "sha256:";

        std::string version = "0.0.0";
        if (ref.kind == GitRefKind::Tag)
            version = SemVer::parse(ref.value).str();

        return {rev40, version, entry, checksum};
    }

    // Step 3: Checkout working tree into cache entry.
    cache_.prepare(url, rev40);
    auto entry = cache_.entry_path(url, rev40);

    // Use git archive to extract the tree cleanly.
    std::string archive_cmd =
        "git -C " + shell_quote(tmp_bare) +
        " archive " + shell_quote(rev40) +
        " | tar -x -C " + shell_quote(entry.string());

    rc = std::system(archive_cmd.c_str());
    std::filesystem::remove_all(tmp_bare);

    if (rc != 0 || !std::filesystem::exists(entry / "fly.toml")) {
        // Fallback: full clone into entry
        std::filesystem::remove_all(entry);
        std::string full_clone = "git clone --quiet "
            + shell_quote(url) + " " + shell_quote(entry.string());
        std::system(full_clone.c_str());
        if (ref.kind != GitRefKind::Tag && ref.kind != GitRefKind::Branch) {
            std::string co = "git -C " + shell_quote(entry.string())
                           + " checkout --quiet " + shell_quote(ref.value);
            std::system(co.c_str());
        }
    }

    auto toml_path = entry / "fly.toml";
    std::string checksum = std::filesystem::exists(toml_path)
        ? sha256_file(toml_path) : "sha256:";

    std::string version = "0.0.0";
    if (ref.kind == GitRefKind::Tag)
        version = SemVer::parse(ref.value).str();

    return {rev40, version, entry, checksum};
}

Manifest GitFetcher::read_manifest(const std::string& name,
                                   const std::filesystem::path& entry) const
{
    auto toml_path = entry / "fly.toml";
    if (!std::filesystem::exists(toml_path))
        throw std::runtime_error("no fly.toml in fetched package '" + name + "'");
    return Manifest::parse(toml_path);
}

std::vector<std::string> GitFetcher::list_tags(const std::string& git_url) const {
    std::string cmd = "git ls-remote --tags " + shell_quote(git_url)
                    + " | awk '{print $2}' | sed 's|refs/tags/||' | grep -v '\\^{}' 2>/dev/null";
    std::string out = run_quiet(cmd);
    std::vector<std::string> tags;
    std::istringstream ss(out);
    std::string line;
    while (std::getline(ss, line))
        if (!line.empty()) tags.push_back(line);
    return tags;
}

std::vector<std::string> GitFetcher::list_branches(const std::string& git_url) const {
    std::string cmd = "git ls-remote --heads " + shell_quote(git_url)
                    + " | awk '{print $2}' | sed 's|refs/heads/||' 2>/dev/null";
    std::string out = run_quiet(cmd);
    std::vector<std::string> branches;
    std::istringstream ss(out);
    std::string line;
    while (std::getline(ss, line))
        if (!line.empty()) branches.push_back(line);
    return branches;
}

} // namespace flyp
