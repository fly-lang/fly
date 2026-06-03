#include "Builder.h"
#include "../fetcher/Cache.h"

#include <filesystem>
#include <future>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

int run_cmd(const std::string& cmd) {
    return std::system(cmd.c_str());
}

} // anonymous namespace

Builder::Builder(const Manifest& manifest,
                 const Lockfile& lockfile,
                 BuildMode       mode)
    : manifest_(manifest), lockfile_(lockfile), mode_(mode) {}

std::filesystem::path Builder::out_dir() const {
    if (mode_ == BuildMode::Release) return manifest_.root_dir / "target" / "release";
    if (mode_ == BuildMode::Test)    return manifest_.root_dir / "target" / "test";
    return manifest_.root_dir / "target" / "debug";
}

std::vector<std::string> Builder::profile_flags() const {
    const BuildProfile& p = (mode_ == BuildMode::Release)
                            ? manifest_.profile_release
                            : manifest_.profile_debug; // Test uses debug profile
    std::vector<std::string> flags;
    flags.push_back("-O" + std::to_string(p.opt_level));
    if (p.debug_info)  flags.push_back("-g");
    if (p.assertions)  flags.push_back("--assertions");
    if (p.lto)         flags.push_back("--lto");
    if (p.strip)       flags.push_back("--strip");
    return flags;
}

std::vector<std::string> Builder::include_paths() const {
    std::vector<std::string> paths;
    Cache cache;
    for (const auto& pkg : lockfile_.packages) {
        auto entry = cache.entry_path(pkg.source.substr(4), pkg.rev); // strip "git+"
        if (std::filesystem::exists(entry))
            paths.push_back("-I" + entry.string());
    }
    return paths;
}

bool Builder::invoke_fly(const std::string& source,
                         const std::string& output,
                         const std::vector<std::string>& extra_flags) const
{
    std::ostringstream cmd;
    cmd << "fly";
    for (const auto& f : profile_flags())  cmd << " " << shell_quote(f);
    for (const auto& i : include_paths())  cmd << " " << shell_quote(i);
    for (const auto& f : extra_flags)      cmd << " " << shell_quote(f);
    cmd << " -o " << shell_quote(output);
    cmd << " "    << shell_quote(source);

    std::cout << "  fly " << source << "\n";
    return run_cmd(cmd.str()) == 0;
}

bool Builder::build_all() {
    std::filesystem::create_directories(out_dir());
    bool ok = true;
    for (const auto& b : manifest_.bins)
        ok = build_bin(b.name) && ok;
    for (const auto& l : manifest_.libs)
        ok = build_lib(l.name) && ok;
    return ok;
}

bool Builder::build_bin(const std::string& name) {
    const BinTarget* target = nullptr;
    for (const auto& b : manifest_.bins)
        if (b.name == name) { target = &b; break; }
    if (!target) {
        std::cerr << "error: no [[bin]] target named '" << name << "'\n";
        return false;
    }

    auto src = manifest_.root_dir / target->path;
    auto out = out_dir() / name;
    return invoke_fly(src.string(), out.string());
}

bool Builder::build_lib(const std::string& name) {
    const LibTarget* target = nullptr;
    for (const auto& l : manifest_.libs)
        if (l.name == name) { target = &l; break; }
    if (!target) {
        std::cerr << "error: no [[lib]] target named '" << name << "'\n";
        return false;
    }

    auto src = manifest_.root_dir / target->path;
    std::string ext = (target->type == "shared") ? ".so" : ".a";
    auto out = out_dir() / ("lib" + name + ext);

    std::vector<std::string> extra;
    if (target->type == "shared") extra.push_back("--shared");

    return invoke_fly(src.string(), out.string(), extra);
}

bool Builder::run_tests(const std::string& suite_filter) {
    // Force test build mode
    mode_ = BuildMode::Test;
    std::filesystem::create_directories(out_dir());

    // Parse filter: "SuiteName::method::\"label\""
    std::string filter_suite, filter_method, filter_label;
    {
        std::string rem = suite_filter;
        auto cut = [&](std::string& dst) {
            auto pos = rem.find("::");
            if (pos == std::string::npos) { dst = rem; rem.clear(); }
            else { dst = rem.substr(0, pos); rem = rem.substr(pos + 2); }
        };
        if (!rem.empty()) { cut(filter_suite); cut(filter_method); cut(filter_label); }
        // Strip surrounding quotes from label if present
        if (filter_label.size() >= 2 && filter_label.front() == '"' && filter_label.back() == '"')
            filter_label = filter_label.substr(1, filter_label.size() - 2);
    }

    // Collect suite files from [test] config globs
    std::vector<std::filesystem::path> suite_files;

    // Glob expansion: match files ending with "Suite.fly" under each pattern prefix
    for (const auto& pattern : manifest_.test_config.suites) {
        std::filesystem::path base = manifest_.root_dir;
        // Simplified glob: treat pattern as a directory prefix + filename suffix
        // Full glob support would use a library; for now accept explicit paths or dir/**
        auto p = base / pattern;
        if (std::filesystem::exists(p) && std::filesystem::is_regular_file(p)) {
            suite_files.push_back(p);
        } else {
            // Walk directories looking for *Suite.fly
            auto dir = p.parent_path();
            if (!std::filesystem::exists(dir)) dir = base;
            for (auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
                if (entry.is_regular_file() &&
                    entry.path().extension() == ".fly" &&
                    entry.path().filename().string().find("Suite") != std::string::npos) {
                    suite_files.push_back(entry.path());
                }
            }
        }
    }

    // Also include explicit [[test]] targets
    for (const auto& t : manifest_.tests) {
        suite_files.push_back(manifest_.root_dir / t.path);
    }

    if (suite_files.empty()) {
        std::cerr << "flyp test: no suite files found. Add [test] suites = [...] to fly.toml\n";
        return false;
    }

    bool ok = true;
    auto run_suite = [&](const std::filesystem::path& src) -> bool {
        std::string suite_name = src.stem().string();
        if (!filter_suite.empty() && suite_name != filter_suite) return true; // skip

        auto out = out_dir() / suite_name;

        std::vector<std::string> extra_flags = {"--test"};
        if (!invoke_fly(src.string(), out.string(), extra_flags))
            return false;

        // Build environment with filter
        std::ostringstream cmd;
        if (!filter_method.empty()) {
            std::string filter_val = filter_suite;
            if (!filter_method.empty()) filter_val += "::" + filter_method;
            if (!filter_label.empty())  filter_val += "::\"" + filter_label + "\"";
            cmd << "FLY_TEST_FILTER=" << shell_quote(filter_val) << " ";
        }
        cmd << shell_quote(out.string());

        std::cout << "  running " << suite_name << "...\n";
        int rc = run_cmd(cmd.str());
        if (rc != 0) {
            std::cerr << "  FAILED: " << suite_name << " (exit " << rc << ")\n";
            if (manifest_.test_config.fail_fast) return false;
            return false;
        }
        std::cout << "  ok: " << suite_name << "\n";
        return true;
    };

    if (manifest_.test_config.parallel && suite_files.size() > 1) {
        std::vector<std::future<bool>> futures;
        for (const auto& sf : suite_files)
            futures.push_back(std::async(std::launch::async, run_suite, sf));
        for (auto& f : futures)
            ok = f.get() && ok;
    } else {
        for (const auto& sf : suite_files) {
            bool r = run_suite(sf);
            ok = r && ok;
            if (!r && manifest_.test_config.fail_fast) break;
        }
    }

    return ok;
}

bool Builder::run_bin(const std::string& name,
                      const std::vector<std::string>& args)
{
    auto bin = out_dir() / name;
    if (!std::filesystem::exists(bin)) {
        std::cerr << "error: '" << name << "' not built — run `flyp build` first\n";
        return false;
    }

    std::ostringstream cmd;
    cmd << shell_quote(bin.string());
    for (const auto& a : args) cmd << " " << shell_quote(a);
    return run_cmd(cmd.str()) == 0;
}

} // namespace flyp
