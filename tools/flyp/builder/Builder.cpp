#include "Builder.h"
#include "../fetcher/Cache.h"
#include "../util/Checksum.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#if defined(__linux__)
#  include <unistd.h>
#  include <climits>
#elif defined(__APPLE__)
#  include <mach-o/dyld.h>
#  include <climits>
#elif defined(_WIN32)
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

namespace flyp {

namespace {

// Resolve the fly compiler binary. Priority:
//   1. FLY_COMPILER env var
//   2. Sibling of the running flyp binary (same directory, platform-specific)
//   3. "fly" (PATH lookup, fallback)
std::string resolve_fly_binary() {
    if (const char* env = std::getenv("FLY_COMPILER"))
        if (*env) return env;

    std::filesystem::path exe;

#if defined(__linux__)
    char buf[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n > 0) { buf[n] = '\0'; exe = buf; }

#elif defined(__APPLE__)
    char buf[PATH_MAX];
    uint32_t sz = sizeof(buf);
    if (_NSGetExecutablePath(buf, &sz) == 0) {
        char resolved[PATH_MAX];
        if (realpath(buf, resolved)) exe = resolved;
    }

#elif defined(_WIN32)
    char buf[MAX_PATH];
    DWORD n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    if (n > 0 && n < MAX_PATH) exe = buf;
#endif

    if (!exe.empty()) {
        auto dir = exe.parent_path();
#ifdef _WIN32
        for (auto name : {"fly.exe", "fly"}) {
#else
        for (auto name : {"fly"}) {
#endif
            auto sibling = dir / name;
            if (std::filesystem::exists(sibling))
                return sibling.string();
        }
    }
    return "fly";
}

} // anonymous namespace

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

Builder::Builder(const Manifest& manifest,
                 const Lockfile& lockfile,
                 const std::string& profile,
                 bool is_test,
                 int jobs,
                 const std::string& target_triple,
                 std::vector<std::filesystem::path> extra_includes)
    : manifest_(manifest), lockfile_(lockfile),
      profile_name_(profile), is_test_(is_test),
      jobs_(jobs), target_triple_(target_triple),
      extra_includes_(std::move(extra_includes)),
      fly_binary_(resolve_fly_binary()) {}

std::filesystem::path Builder::out_dir() const {
    if (is_test_) return manifest_.root_dir / "target" / "test";
    if (!target_triple_.empty())
        return manifest_.root_dir / "target" / profile_name_ / target_triple_;
    return manifest_.root_dir / "target" / profile_name_;
}

std::vector<std::string> Builder::profile_flags() const {
    static const BuildProfile kDefaultDebug{};
    auto it = manifest_.profiles.find(profile_name_);
    const BuildProfile& p = (it != manifest_.profiles.end()) ? it->second : kDefaultDebug;
    std::vector<std::string> flags;
    flags.push_back("-O");
    flags.push_back(std::to_string(p.opt_level));
    if (p.debug_info) flags.push_back("--debug-symbols"); // emit DWARF debug info only
    // --assertions, --lto, --strip not yet exposed as fly CLI flags
    return flags;
}

std::vector<std::string> Builder::include_paths() const {
    std::vector<std::string> paths;
    Cache cache;
    for (const auto& pkg : lockfile_.packages) {
        // source is "git+<url>" or "registry+<name>/<version>"
        std::string cache_url = pkg.source.rfind("git+", 0) == 0
            ? pkg.source.substr(4)   // strip "git+"
            : pkg.source.substr(9);  // strip "registry+"
        auto entry = cache.entry_path(cache_url, pkg.rev);
        if (std::filesystem::exists(entry))
            paths.push_back("-L" + entry.string()); // fly uses -L for library dirs
    }
    // Path dependency output directories (workspace members).
    for (const auto& dir : extra_includes_)
        if (std::filesystem::exists(dir))
            paths.push_back("-L" + dir.string());
    // When building tests, also include the project's own source directories
    // so test files can import the project's own library namespace via .fly.h.
    if (is_test_) {
        for (const auto& t : manifest_.targets) {
            if (t.is_lib()) {
                auto src_dir = (manifest_.root_dir / t.path).parent_path();
                if (std::filesystem::exists(src_dir))
                    paths.push_back("-L" + src_dir.string());
            }
        }
        // Also add root src/ as a common convention.
        auto src = manifest_.root_dir / "src";
        if (std::filesystem::exists(src))
            paths.push_back("-L" + src.string());
    }
    return paths;
}

std::vector<std::filesystem::path> Builder::project_lib_inputs() const {
    std::vector<std::filesystem::path> libs;
    if (!is_test_) return libs;
    // Add the project's own compiled library archives as inputs for test linking.
    for (const auto& t : manifest_.targets) {
        if (!t.is_lib()) continue;
        std::string ext = (t.lib == "dynamic") ? ".so" : ".a";
        // Use the profile's out_dir (not test out_dir) for lib artifacts.
        auto lib_out = manifest_.root_dir / "target" / profile_name_;
        auto lib_path = lib_out / ("lib" + t.name + ext);
        if (std::filesystem::exists(lib_path))
            libs.push_back(lib_path);
    }
    return libs;
}

// ── Build hooks ───────────────────────────────────────────────────────────────

bool Builder::run_hook(const std::string& cmd, const std::string& phase) const {
    std::cout << "  hook [" << phase << "]: " << cmd << "\n";
    std::cout.flush();

    // Set FLYP_* environment variables for the hook process.
    auto set_env = [](const std::string& key, const std::string& val) {
#ifdef _WIN32
        _putenv_s(key.c_str(), val.c_str());
#else
        ::setenv(key.c_str(), val.c_str(), 1);
#endif
    };
    set_env("FLYP_PROFILE",       profile_name_);
    set_env("FLYP_OUT_DIR",       out_dir().string());
    set_env("FLYP_ROOT_DIR",      manifest_.root_dir.string());
    set_env("FLYP_TARGET_TRIPLE", target_triple_);
    set_env("FLYP_PACKAGE_NAME",  manifest_.name);

    // Run from the package root directory.
    std::string full_cmd = "cd " + shell_quote(manifest_.root_dir.string())
                         + " && " + cmd;
    int rc = std::system(full_cmd.c_str());
    if (rc != 0) {
        std::cerr << "error: hook [" << phase << "] failed (exit " << rc << "): "
                  << cmd << "\n";
        return false;
    }
    return true;
}

// ── Incremental build ─────────────────────────────────────────────────────────

std::string Builder::compute_fingerprint(const Target& t) const {
    auto src_path = manifest_.root_dir / t.path;

    std::string src_hash = std::filesystem::exists(src_path)
        ? sha256_file(src_path) : "sha256:missing";

    // Profile flags that affect the generated binary (not --jobs).
    std::string flags;
    for (const auto& f : profile_flags()) flags += f + " ";

    // Hash of fly.lock captures resolved dependency revisions.
    auto lock_path = manifest_.root_dir / "fly.lock";
    std::string lock_hash = std::filesystem::exists(lock_path)
        ? sha256_file(lock_path) : "sha256:none";

    std::ostringstream oss;
    oss << "source:" << src_hash  << "\n"
        << "flags:"  << flags     << "\n"
        << "triple:" << target_triple_ << "\n"
        << "lock:"   << lock_hash << "\n"
        << "lib:"    << t.lib     << "\n"
        << "name:"   << t.name    << "\n";
    return oss.str();
}

std::filesystem::path Builder::fp_path(const Target& t) const {
    return out_dir() / ".flyp" / (t.key + ".fp");
}

bool Builder::is_up_to_date(const Target& t) const {
    auto fp = fp_path(t);
    if (!std::filesystem::exists(fp)) return false;

    std::ifstream f(fp);
    if (!f) return false;
    std::string stored((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
    return stored == compute_fingerprint(t);
}

void Builder::save_fingerprint(const Target& t) const {
    auto fp = fp_path(t);
    std::filesystem::create_directories(fp.parent_path());
    std::ofstream f(fp);
    if (f) f << compute_fingerprint(t);
}

bool Builder::invoke_fly(const std::string& source,
                         const std::string& output,
                         const std::vector<std::string>& extra_flags) const
{
    std::vector<std::string> all_flags = extra_flags;
    if (!target_triple_.empty())
        all_flags.push_back("--target=" + target_triple_);
    if (jobs_ > 0)
        all_flags.push_back("--jobs=" + std::to_string(jobs_));

    // Run from out_dir so intermediate .fly.o files land there, not in cwd.
    std::filesystem::create_directories(out_dir());
    std::ostringstream cmd;
    cmd << "cd " << shell_quote(out_dir().string()) << " && ";
    cmd << shell_quote(fly_binary_);
    for (const auto& f : profile_flags())  cmd << " " << shell_quote(f);
    for (const auto& i : include_paths())  cmd << " " << shell_quote(i);
    for (const auto& f : all_flags)        cmd << " " << shell_quote(f);
    // Pass native library dependencies from [link] section
    for (const auto& lib : manifest_.link_libs)
        cmd << " --link-lib=" << shell_quote(lib);
    cmd << " -o " << shell_quote(output);
    cmd << " "    << shell_quote(source);
    // For test builds, also pass the project's own .a libraries as inputs
    // so the fly compiler can find and link them (it loads the companion .fly.h).
    for (const auto& lib : project_lib_inputs())
        cmd << " " << shell_quote(lib.string());

    {
        std::lock_guard<std::mutex> lk(log_mutex_);
        std::cout << "  fly " << source << "\n";
        std::cout.flush();
    }
    return run_cmd(cmd.str()) == 0;
}

bool Builder::build_subset(const std::vector<std::string>& keys, int jobs) {
    if (keys.empty()) return build_all(jobs);

    // Validate all keys exist before starting.
    for (const auto& key : keys) {
        bool found = false;
        for (const auto& t : manifest_.targets)
            if (t.key == key) { found = true; break; }
        if (!found) {
            std::cerr << "error: no [targets] entry named '" << key << "'\n";
            return false;
        }
    }

    std::filesystem::create_directories(out_dir());
    const auto n = keys.size();

    if (!manifest_.hooks.pre_build.empty())
        if (!run_hook(manifest_.hooks.pre_build, "pre-build")) return false;

    const int resolved = (jobs == 0)
        ? static_cast<int>(std::max(1u, std::thread::hardware_concurrency()))
        : jobs;

    bool ok;
    if (n == 1) {
        jobs_ = resolved;
        ok = build_target(keys[0]);
    } else if (resolved == 1) {
        jobs_ = 1;
        ok = true;
        for (const auto& key : keys)
            ok = build_target(key) && ok;
    } else {
        jobs_ = 1;
        std::vector<std::future<bool>> futures;
        futures.reserve(n);
        for (const auto& key : keys)
            futures.push_back(std::async(std::launch::async,
                                         [this, k = key]{ return build_target(k); }));
        ok = true;
        for (auto& f : futures) ok = f.get() && ok;
    }

    if (ok && !manifest_.hooks.post_build.empty())
        ok = run_hook(manifest_.hooks.post_build, "post-build");
    return ok;
}

bool Builder::build_all(int jobs) {
    std::filesystem::create_directories(out_dir());

    const auto n = manifest_.targets.size();
    if (n == 0) return true;

    if (!manifest_.hooks.pre_build.empty())
        if (!run_hook(manifest_.hooks.pre_build, "pre-build")) return false;

    const int resolved = (jobs == 0)
        ? static_cast<int>(std::max(1u, std::thread::hardware_concurrency()))
        : jobs;

    bool ok;

    if (n == 1) {
        jobs_ = resolved;
        ok = build_target(manifest_.targets[0].key);
    } else if (resolved == 1) {
        jobs_ = 1;
        ok = true;
        for (const auto& t : manifest_.targets)
            ok = build_target(t.key) && ok;
    } else {
        jobs_ = 1;
        std::vector<std::future<bool>> futures;
        futures.reserve(n);
        for (const auto& t : manifest_.targets)
            futures.push_back(std::async(std::launch::async,
                                         [this, key = t.key]{ return build_target(key); }));
        ok = true;
        for (auto& f : futures) ok = f.get() && ok;
    }

    if (ok && !manifest_.hooks.post_build.empty())
        ok = run_hook(manifest_.hooks.post_build, "post-build");
    return ok;
}

bool Builder::build_target(const std::string& key) {
    const Target* t = nullptr;
    for (const auto& tgt : manifest_.targets)
        if (tgt.key == key) { t = &tgt; break; }
    if (!t) {
        std::cerr << "error: no [targets] entry named '" << key << "'\n";
        return false;
    }

    // Incremental check: skip if nothing has changed.
    if (is_up_to_date(*t)) {
        std::lock_guard<std::mutex> lk(log_mutex_);
        std::cout << "  up to date: " << key << "\n";
        std::cout.flush();
        return true;
    }

    auto src = manifest_.root_dir / t->path;
    bool ok;

    if (t->is_lib()) {
        if (t->lib == "dynamic" || t->lib == "both") {
            auto out = out_dir() / ("lib" + t->name + ".so");
            ok = invoke_fly(src.string(), out.string(), {"--shared"});
            if (!ok) return false;
        }
        if (t->lib != "dynamic") { // "static" or "both"
            auto out = out_dir() / ("lib" + t->name + ".a");
            ok = invoke_fly(src.string(), out.string(), {"--lib"});
        }
    } else {
        ok = invoke_fly(src.string(), (out_dir() / t->name).string());
    }

    if (ok) save_fingerprint(*t);
    return ok;
}

bool Builder::run_tests(const std::string& suite_filter) {
    is_test_ = true;
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

    // Glob expansion: find all .fly files matching each pattern prefix.
    for (const auto& pattern : manifest_.test_config.suites) {
        std::filesystem::path base = manifest_.root_dir;
        auto p = base / pattern;
        if (std::filesystem::exists(p) && std::filesystem::is_regular_file(p)) {
            suite_files.push_back(p);
        } else {
            // Determine the concrete directory by stripping wildcard components.
            std::filesystem::path dir = base;
            for (const auto& comp : std::filesystem::path(pattern)) {
                std::string s = comp.string();
                if (s.find('*') != std::string::npos || s.find('?') != std::string::npos)
                    break;
                dir /= comp;
            }
            if (!std::filesystem::exists(dir)) dir = base;
            for (auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".fly")
                    suite_files.push_back(entry.path());
            }
        }
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

bool Builder::run_target(const std::string& key,
                         const std::vector<std::string>& args)
{
    if (!target_triple_.empty()) {
        std::cerr << "error: cannot run a cross-compiled binary (target triple '"
                  << target_triple_ << "' differs from host).\n"
                  << "  Use an emulator (e.g. qemu-" << target_triple_.substr(0, target_triple_.find('-'))
                  << ") to execute the binary manually.\n";
        return false;
    }
    const Target* t = nullptr;
    for (const auto& tgt : manifest_.targets)
        if (tgt.key == key) { t = &tgt; break; }
    if (!t) {
        std::cerr << "error: no [targets] entry named '" << key << "'\n";
        return false;
    }

    auto bin = out_dir() / t->name;
    if (!std::filesystem::exists(bin)) {
        std::cerr << "error: '" << key << "' not built — run `flyp build` first\n";
        return false;
    }

    std::ostringstream cmd;
    cmd << shell_quote(bin.string());
    for (const auto& a : args) cmd << " " << shell_quote(a);
    return run_cmd(cmd.str()) == 0;
}

} // namespace flyp
