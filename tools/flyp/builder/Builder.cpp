#include "Builder.h"
#include "../fetcher/Cache.h"

#include <filesystem>
#include <iostream>
#include <sstream>

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
    return manifest_.root_dir / "target" /
           (mode_ == BuildMode::Release ? "release" : "debug");
}

std::vector<std::string> Builder::profile_flags() const {
    const BuildProfile& p = (mode_ == BuildMode::Release)
                            ? manifest_.profile_release
                            : manifest_.profile_debug;
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

bool Builder::run_tests(const std::string& suite_name) {
    std::filesystem::create_directories(out_dir());
    bool ok = true;
    for (const auto& t : manifest_.tests) {
        if (!suite_name.empty() && t.name != suite_name) continue;

        auto src = manifest_.root_dir / t.path;
        auto out = out_dir() / ("test-" + t.name);

        if (!invoke_fly(src.string(), out.string(), {"--test"})) {
            ok = false;
            continue;
        }

        std::cout << "  running " << t.name << "...\n";
        int rc = run_cmd(shell_quote(out.string()));
        if (rc != 0) {
            std::cerr << "  FAILED: " << t.name << " (exit " << rc << ")\n";
            ok = false;
        } else {
            std::cout << "  ok: " << t.name << "\n";
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
