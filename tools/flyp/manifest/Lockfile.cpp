#include "Lockfile.h"
#include "../util/Checksum.h"

#include <toml++/toml.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace flyp {

Lockfile Lockfile::read(const std::filesystem::path& path) {
    Lockfile lf;
    if (!std::filesystem::exists(path)) return lf;

    toml::table tbl;
    try {
        tbl = toml::parse_file(path.string());
    } catch (const toml::parse_error& e) {
        throw std::runtime_error(std::string("fly.lock parse error: ") + e.what());
    }

    lf.format_version = (int)tbl["version"].value<int64_t>().value_or(1);
    lf.flyp_version   = tbl["flyp"].value<std::string>().value_or("");
    lf.toml_checksum  = tbl["checksum"].value<std::string>().value_or("");

    if (auto* pkgs = tbl["package"].as_array()) {
        for (auto& el : *pkgs) {
            auto* t = el.as_table();
            if (!t) continue;
            LockedPackage p;
            p.name     = (*t)["name"].value<std::string>().value_or("");
            p.version  = (*t)["version"].value<std::string>().value_or("");
            p.source   = (*t)["source"].value<std::string>().value_or("");
            p.rev      = (*t)["rev"].value<std::string>().value_or("");
            p.tag      = (*t)["tag"].value<std::string>().value_or("");
            p.checksum = (*t)["checksum"].value<std::string>().value_or("");

            if (auto* deps = (*t)["dependencies"].as_array())
                for (auto& d : *deps)
                    if (auto s = d.value<std::string>()) p.deps.push_back(*s);

            lf.packages.push_back(std::move(p));
        }
    }
    return lf;
}

void Lockfile::write(const std::filesystem::path& path) const {
    std::ofstream f(path);
    if (!f) throw std::runtime_error("cannot write fly.lock: " + path.string());

    f << "# fly.lock — generated automatically by flyp.\n";
    f << "# Do not edit manually.\n";
    f << "# Commit alongside fly.toml for reproducible builds.\n\n";

    f << "version  = " << format_version << "\n";
    f << "flyp     = \"" << flyp_version  << "\"\n";
    f << "checksum = \"" << toml_checksum << "\"\n";

    for (const auto& p : packages) {
        f << "\n[[package]]\n";
        f << "name         = \"" << p.name     << "\"\n";
        f << "version      = \"" << p.version  << "\"\n";
        f << "source       = \"" << p.source   << "\"\n";
        f << "rev          = \"" << p.rev      << "\"\n";
        if (!p.tag.empty())
            f << "tag          = \"" << p.tag  << "\"\n";
        f << "checksum     = \"" << p.checksum << "\"\n";

        f << "dependencies = [";
        for (size_t i = 0; i < p.deps.size(); ++i) {
            if (i) f << ", ";
            f << "\"" << p.deps[i] << "\"";
        }
        f << "]\n";
    }
}

bool Lockfile::is_stale(const std::filesystem::path& toml_path) const {
    if (toml_checksum.empty()) return true;
    try {
        return sha256_file(toml_path) != toml_checksum;
    } catch (...) {
        return true;
    }
}

const LockedPackage* Lockfile::find(const std::string& name) const {
    for (const auto& p : packages)
        if (p.name == name) return &p;
    return nullptr;
}

} // namespace flyp
