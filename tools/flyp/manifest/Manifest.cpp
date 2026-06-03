#include "Manifest.h"

#include <toml++/toml.hpp>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace flyp {

namespace {

GitDep parse_git_dep(const std::string& name, const toml::table& tbl) {
    GitDep dep;

    auto git = tbl["git"].value<std::string>();
    if (!git) throw std::runtime_error("[dependencies." + name + "] missing 'git' field");
    dep.git_url = *git;

    if (auto tag = tbl["tag"].value<std::string>()) {
        dep.ref = {GitRefKind::Tag, *tag};
    } else if (auto branch = tbl["branch"].value<std::string>()) {
        dep.ref = {GitRefKind::Branch, *branch};
    } else if (auto rev = tbl["rev"].value<std::string>()) {
        dep.ref = {GitRefKind::Rev, *rev};
    } else {
        throw std::runtime_error("[dependencies." + name + "] must specify 'tag', 'branch', or 'rev'");
    }
    return dep;
}

BuildProfile parse_profile(const toml::table* tbl, bool is_release) {
    BuildProfile p;
    if (is_release) {
        p.opt_level  = 3;
        p.debug_info = false;
        p.lto        = false;
        p.strip      = false;
        p.assertions = false;
    }
    if (!tbl) return p;

    if (auto v = (*tbl)["opt-level"].value<int64_t>())   p.opt_level  = (int)*v;
    if (auto v = (*tbl)["debug-info"].value<bool>())      p.debug_info = *v;
    if (auto v = (*tbl)["assertions"].value<bool>())      p.assertions = *v;
    if (auto v = (*tbl)["lto"].value<bool>())             p.lto        = *v;
    if (auto v = (*tbl)["strip"].value<bool>())           p.strip      = *v;
    return p;
}

void validate_name(const std::string& name) {
    static const std::regex re{"^[a-z0-9_-]+$"};
    if (!std::regex_match(name, re))
        throw std::runtime_error("package name '" + name + "' must match [a-z0-9_-]+");
}

void validate_version(const std::string& ver) {
    static const std::regex re{R"(\d+\.\d+\.\d+)"};
    if (!std::regex_match(ver, re))
        throw std::runtime_error("version '" + ver + "' must be MAJOR.MINOR.PATCH semver");
}

} // anonymous namespace

Manifest Manifest::parse(const std::filesystem::path& toml_path) {
    if (!std::filesystem::exists(toml_path))
        throw std::runtime_error("fly.toml not found: " + toml_path.string());

    toml::table tbl;
    try {
        tbl = toml::parse_file(toml_path.string());
    } catch (const toml::parse_error& e) {
        throw std::runtime_error(std::string("fly.toml parse error: ") + e.what());
    }

    Manifest m;
    m.root_dir = toml_path.parent_path();

    // [package] — required fields
    auto& pkg = *tbl["package"].as_table();
    m.name        = pkg["name"].value<std::string>().value_or("");
    m.version     = pkg["version"].value<std::string>().value_or("");
    m.description = pkg["description"].value<std::string>().value_or("");
    m.license     = pkg["license"].value<std::string>().value_or("");
    m.fly_version = pkg["fly-version"].value<std::string>().value_or("");
    m.homepage    = pkg["homepage"].value<std::string>();
    m.repository  = pkg["repository"].value<std::string>();

    if (m.name.empty())    throw std::runtime_error("[package] 'name' is required");
    if (m.version.empty()) throw std::runtime_error("[package] 'version' is required");

    validate_name(m.name);
    validate_version(m.version);

    if (auto* arr = pkg["authors"].as_array())
        for (auto& el : *arr)
            if (auto s = el.value<std::string>()) m.authors.push_back(*s);

    // [[bin]]
    if (auto* bins = tbl["bin"].as_array()) {
        for (auto& el : *bins) {
            auto* t = el.as_table();
            BinTarget b;
            b.name = (*t)["name"].value<std::string>().value_or("");
            b.path = (*t)["path"].value<std::string>().value_or("");
            if (!b.name.empty()) m.bins.push_back(std::move(b));
        }
    }

    // [[lib]]
    if (auto* libs = tbl["lib"].as_array()) {
        for (auto& el : *libs) {
            auto* t = el.as_table();
            LibTarget l;
            l.name = (*t)["name"].value<std::string>().value_or("");
            l.path = (*t)["path"].value<std::string>().value_or("");
            l.type = (*t)["type"].value<std::string>().value_or("static");
            if (!l.name.empty()) m.libs.push_back(std::move(l));
        }
    }

    // [[test]] — array of explicit test targets (legacy format)
    if (auto* tests = tbl["test"].as_array()) {
        for (auto& el : *tests) {
            auto* t = el.as_table();
            TestTarget tt;
            tt.name = (*t)["name"].value<std::string>().value_or("");
            tt.path = (*t)["path"].value<std::string>().value_or("");
            if (!tt.name.empty()) m.tests.push_back(std::move(tt));
        }
    }

    // [test] — plain table for suite-based test configuration
    if (auto* tc = tbl["test"].as_table()) {
        if (auto* suites_arr = (*tc)["suites"].as_array()) {
            for (auto& el : *suites_arr) {
                if (auto s = el.value<std::string>())
                    m.test_config.suites.push_back(*s);
            }
        }
        m.test_config.parallel   = (*tc)["parallel"].value<bool>().value_or(false);
        m.test_config.timeout_ms = (int)(*tc)["timeout_ms"].value<int64_t>().value_or(0);
        m.test_config.fail_fast  = (*tc)["fail_fast"].value<bool>().value_or(false);
    }

    // Auto-detect targets if none declared
    if (m.bins.empty() && std::filesystem::exists(m.root_dir / "src" / "main.fly"))
        m.bins.push_back({m.name, "src/main.fly"});

    if (m.libs.empty() && std::filesystem::exists(m.root_dir / "src" / "lib.fly"))
        m.libs.push_back({m.name, "src/lib.fly", "static"});

    // [dependencies]
    if (auto* deps = tbl["dependencies"].as_table()) {
        for (auto& [k, v] : *deps) {
            if (auto* t = v.as_table())
                m.dependencies[std::string(k)] = parse_git_dep(std::string(k), *t);
        }
    }

    // [dev-dependencies]
    if (auto* deps = tbl["dev-dependencies"].as_table()) {
        for (auto& [k, v] : *deps) {
            if (auto* t = v.as_table())
                m.dev_dependencies[std::string(k)] = parse_git_dep(std::string(k), *t);
        }
    }

    // [profile.debug] / [profile.release]
    const toml::table* prof_debug   = nullptr;
    const toml::table* prof_release = nullptr;
    if (auto* prof = tbl["profile"].as_table()) {
        prof_debug   = (*prof)["debug"].as_table();
        prof_release = (*prof)["release"].as_table();
    }
    m.profile_debug   = parse_profile(prof_debug,   false);
    m.profile_release = parse_profile(prof_release, true);

    return m;
}

void Manifest::add_dependency(const std::string& name, const GitDep& dep) {
    dependencies[name] = dep;
    save();
}

void Manifest::remove_dependency(const std::string& name) {
    dependencies.erase(name);
    dev_dependencies.erase(name);
    save();
}

void Manifest::save() const {
    // Serialize the manifest back to fly.toml.
    // We rewrite the full file to avoid partial-update issues.
    auto path = root_dir / "fly.toml";
    std::ofstream f(path);
    if (!f) throw std::runtime_error("cannot write fly.toml: " + path.string());

    auto ref_str = [](const GitRef& r) -> std::string {
        switch (r.kind) {
            case GitRefKind::Tag:    return "tag = \"" + r.value + "\"";
            case GitRefKind::Branch: return "branch = \"" + r.value + "\"";
            case GitRefKind::Rev:    return "rev = \"" + r.value + "\"";
        }
        return "";
    };

    f << "[package]\n";
    f << "name        = \"" << name        << "\"\n";
    f << "version     = \"" << version     << "\"\n";
    f << "description = \"" << description << "\"\n";
    f << "license     = \"" << license     << "\"\n";
    f << "fly-version = \"" << fly_version << "\"\n";
    if (homepage)   f << "homepage    = \"" << *homepage   << "\"\n";
    if (repository) f << "repository  = \"" << *repository << "\"\n";
    if (!authors.empty()) {
        f << "authors     = [";
        for (size_t i = 0; i < authors.size(); ++i) {
            if (i) f << ", ";
            f << "\"" << authors[i] << "\"";
        }
        f << "]\n";
    }
    f << "\n";

    for (auto& b : bins)
        f << "[[bin]]\nname = \"" << b.name << "\"\npath = \"" << b.path << "\"\n\n";
    for (auto& l : libs)
        f << "[[lib]]\nname = \"" << l.name << "\"\npath = \"" << l.path
          << "\"\ntype = \"" << l.type << "\"\n\n";
    for (auto& t : tests)
        f << "[[test]]\nname = \"" << t.name << "\"\npath = \"" << t.path << "\"\n\n";

    if (!dependencies.empty()) {
        f << "[dependencies]\n";
        for (auto& [n, d] : dependencies)
            f << n << " = { git = \"" << d.git_url << "\", " << ref_str(d.ref) << " }\n";
        f << "\n";
    }

    if (!dev_dependencies.empty()) {
        f << "[dev-dependencies]\n";
        for (auto& [n, d] : dev_dependencies)
            f << n << " = { git = \"" << d.git_url << "\", " << ref_str(d.ref) << " }\n";
        f << "\n";
    }

    // profiles
    f << "[profile.debug]\n";
    f << "opt-level  = " << profile_debug.opt_level  << "\n";
    f << "debug-info = " << (profile_debug.debug_info ? "true" : "false") << "\n";
    f << "assertions = " << (profile_debug.assertions ? "true" : "false") << "\n";
    f << "\n";
    f << "[profile.release]\n";
    f << "opt-level  = " << profile_release.opt_level  << "\n";
    f << "debug-info = " << (profile_release.debug_info ? "true" : "false") << "\n";
    f << "lto        = " << (profile_release.lto        ? "true" : "false") << "\n";
    f << "strip      = " << (profile_release.strip      ? "true" : "false") << "\n";
    f << "assertions = " << (profile_release.assertions ? "true" : "false") << "\n";
}

} // namespace flyp
