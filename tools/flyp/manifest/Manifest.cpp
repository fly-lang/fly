#include "Manifest.h"

#include <toml++/toml.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace flyp {

namespace {

// map_key is the dependency alias (used as default pkg_name for registry deps).
GitDep parse_git_dep(const std::string& name, const toml::table& tbl,
                     const std::string& map_key = "") {
    GitDep dep;

    // Path dependency: { path = "../member" }
    if (auto p = tbl["path"].value<std::string>()) {
        dep.path = *p;
        return dep;
    }

    // Registry dependency: { registry = "local", version = "1.0.0" }
    if (auto reg = tbl["registry"].value<std::string>()) {
        dep.registry_name = *reg;
        dep.pkg_name      = tbl["name"].value<std::string>().value_or(
                                map_key.empty() ? name : map_key);
        dep.version_req   = tbl["version"].value<std::string>().value_or("*");
        return dep;
    }

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

BuildProfile default_profile(const std::string& name) {
    BuildProfile p; // defaults: opt_level=0, debug_info=true, assertions=true, lto=false, strip=false
    if (name == "release") {
        p.opt_level  = 3;
        p.debug_info = false;
        p.assertions = false;
    }
    return p;
}

BuildProfile parse_inline_profile(const toml::table& tbl, const std::string& name) {
    BuildProfile p = default_profile(name);
    if (auto v = tbl["opt-level"].value<int64_t>())  p.opt_level  = (int)*v;
    if (auto v = tbl["debug-info"].value<bool>())    p.debug_info = *v;
    if (auto v = tbl["assertions"].value<bool>())    p.assertions = *v;
    if (auto v = tbl["lto"].value<bool>())           p.lto        = *v;
    if (auto v = tbl["strip"].value<bool>())         p.strip      = *v;
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

    // [package] — required for regular packages; workspace roots may omit it.
    if (auto* pkg_tbl = tbl["package"].as_table()) {
        auto& pkg = *pkg_tbl;
        m.name        = pkg["name"].value<std::string>().value_or("");
        m.version     = pkg["version"].value<std::string>().value_or("");
        m.description = pkg["description"].value<std::string>().value_or("");
        m.license     = pkg["license"].value<std::string>().value_or("");
        m.fly_version = pkg["fly-version"].value<std::string>().value_or("");
        m.homepage    = pkg["homepage"].value<std::string>();
        m.repository  = pkg["repository"].value<std::string>();

        if (auto* arr = pkg["authors"].as_array())
            for (auto& el : *arr)
                if (auto s = el.value<std::string>()) m.authors.push_back(*s);

        m.default_registry = pkg["registry"].value<std::string>().value_or("");
    }

    // [repo] — named registry aliases
    if (auto* repo_tbl = tbl["repo"].as_table())
        for (auto& [k, v] : *repo_tbl)
            if (auto url = v.value<std::string>()) m.repos[std::string(k)] = *url;

    // Parse [workspace] early so we can skip [package] validation for workspace roots.
    if (auto* ws_tbl = tbl["workspace"].as_table()) {
        WorkspaceConfig ws;
        if (auto* members_arr = (*ws_tbl)["members"].as_array())
            for (auto& el : *members_arr)
                if (auto s = el.value<std::string>()) ws.members.push_back(*s);
        m.workspace = std::move(ws);
    }

    // [package] name+version are required only for non-workspace-root manifests.
    if (!m.is_workspace_root()) {
        if (m.name.empty())    throw std::runtime_error("[package] 'name' is required");
        if (m.version.empty()) throw std::runtime_error("[package] 'version' is required");
        if (!m.name.empty())    validate_name(m.name);
        if (!m.version.empty()) validate_version(m.version);
    }

    // [targets]
    if (auto* tbl_tgt = tbl["targets"].as_table()) {
        for (auto& [k, v] : *tbl_tgt) {
            auto* t = v.as_table(); if (!t) continue;
            Target tgt;
            tgt.key  = std::string(k);
            tgt.name = (*t)["name"].value<std::string>().value_or(tgt.key);
            tgt.path = (*t)["path"].value<std::string>().value_or("");
            tgt.lib  = (*t)["lib"].value<std::string>().value_or("");
            if (!tgt.path.empty()) m.targets.push_back(std::move(tgt));
        }
    }

    // [hooks]
    if (auto* h = tbl["hooks"].as_table()) {
        m.hooks.pre_build  = (*h)["pre-build"].value<std::string>().value_or("");
        m.hooks.post_build = (*h)["post-build"].value<std::string>().value_or("");
    }

    // [link] — native C library dependencies
    if (auto* lnk = tbl["link"].as_table()) {
        if (auto* libs_arr = (*lnk)["libs"].as_array()) {
            for (auto& el : *libs_arr) {
                if (auto s = el.value<std::string>())
                    m.link_libs.push_back(*s);
            }
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

    // Auto-detect: independent checks for bin and lib
    bool has_bin = std::any_of(m.targets.begin(), m.targets.end(),
                               [](const Target& t){ return t.is_bin(); });
    if (!has_bin && std::filesystem::exists(m.root_dir / "src" / "main.fly"))
        m.targets.push_back({m.name, m.name, "src/main.fly", ""});

    bool has_lib = std::any_of(m.targets.begin(), m.targets.end(),
                               [](const Target& t){ return t.is_lib(); });
    if (!has_lib && std::filesystem::exists(m.root_dir / "src" / "lib.fly"))
        m.targets.push_back({m.name, m.name, "src/lib.fly", "static"});

    // [dependencies]
    if (auto* deps = tbl["dependencies"].as_table()) {
        for (auto& [k, v] : *deps) {
            const std::string key = std::string(k);
            if (auto* t = v.as_table())
                m.dependencies[key] = parse_git_dep(key, *t, key);
        }
    }

    // [dev-dependencies]
    if (auto* deps = tbl["dev-dependencies"].as_table()) {
        for (auto& [k, v] : *deps) {
            const std::string key = std::string(k);
            if (auto* t = v.as_table())
                m.dev_dependencies[key] = parse_git_dep(key, *t, key);
        }
    }

    // [profiles]
    if (auto* prof_tbl = tbl["profiles"].as_table()) {
        for (auto& [k, v] : *prof_tbl) {
            auto* t = v.as_table(); if (!t) continue;
            std::string pname = std::string(k);
            m.profiles[pname] = parse_inline_profile(*t, pname);
        }
    }
    if (!m.profiles.count("debug"))   m.profiles["debug"]   = default_profile("debug");
    if (!m.profiles.count("release")) m.profiles["release"] = default_profile("release");

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
    if (homepage)              f << "homepage    = \"" << *homepage         << "\"\n";
    if (repository)            f << "repository  = \"" << *repository       << "\"\n";
    if (!default_registry.empty()) f << "registry    = \"" << default_registry << "\"\n";
    if (!authors.empty()) {
        f << "authors     = [";
        for (size_t i = 0; i < authors.size(); ++i) {
            if (i) f << ", ";
            f << "\"" << authors[i] << "\"";
        }
        f << "]\n";
    }
    f << "\n";

    if (!repos.empty()) {
        f << "[repo]\n";
        for (auto& [alias, url] : repos)
            f << alias << " = \"" << url << "\"\n";
        f << "\n";
    }

    if (!targets.empty()) {
        f << "[targets]\n";
        for (auto& t : targets) {
            f << t.key << " = { ";
            if (t.name != t.key) f << "name = \"" << t.name << "\", ";
            f << "path = \"" << t.path << "\"";
            if (t.is_lib()) f << ", lib = \"" << t.lib << "\"";
            f << " }\n";
        }
        f << "\n";
    }

    if (!hooks.pre_build.empty() || !hooks.post_build.empty()) {
        f << "[hooks]\n";
        if (!hooks.pre_build.empty())
            f << "pre-build  = \"" << hooks.pre_build  << "\"\n";
        if (!hooks.post_build.empty())
            f << "post-build = \"" << hooks.post_build << "\"\n";
        f << "\n";
    }

    auto dep_str = [&](const std::string& n, const GitDep& d) {
        if (d.is_path_dep())
            return n + " = { path = \"" + d.path + "\" }";
        if (d.is_registry_dep()) {
            std::string s = n + " = { registry = \"" + d.registry_name + "\", version = \""
                          + d.version_req + "\"";
            if (d.pkg_name != n) s += ", name = \"" + d.pkg_name + "\"";
            return s + " }";
        }
        return n + " = { git = \"" + d.git_url + "\", " + ref_str(d.ref) + " }";
    };

    if (!dependencies.empty()) {
        f << "[dependencies]\n";
        for (auto& [n, d] : dependencies) f << dep_str(n, d) << "\n";
        f << "\n";
    }

    if (!dev_dependencies.empty()) {
        f << "[dev-dependencies]\n";
        for (auto& [n, d] : dev_dependencies) f << dep_str(n, d) << "\n";
        f << "\n";
    }

    if (workspace) {
        f << "[workspace]\n";
        f << "members = [";
        for (size_t i = 0; i < workspace->members.size(); ++i) {
            if (i) f << ", ";
            f << "\"" << workspace->members[i] << "\"";
        }
        f << "]\n\n";
    }

    // profiles
    if (!profiles.empty()) {
        auto b = [](bool v) { return v ? "true" : "false"; };
        f << "[profiles]\n";
        for (auto& [name, p] : profiles) {
            f << name << " = { "
              << "opt-level = "  << p.opt_level        << ", "
              << "debug-info = " << b(p.debug_info)    << ", "
              << "assertions = " << b(p.assertions)    << ", "
              << "lto = "        << b(p.lto)           << ", "
              << "strip = "      << b(p.strip)
              << " }\n";
        }
        f << "\n";
    }
}

} // namespace flyp
