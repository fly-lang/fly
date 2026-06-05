#include "MVSResolver.h"

#include <algorithm>
#include <iostream>
#include <queue>
#include <sstream>
#include <stdexcept>

namespace flyp {

// ── SemVer ────────────────────────────────────────────────────────────────────

SemVer SemVer::parse(const std::string& s) {
    SemVer v;
    const char* p = s.c_str();
    if (*p == 'v' || *p == 'V') ++p;
    try {
        char* end;
        v.major = (int)std::strtol(p, &end, 10);
        if (*end == '.') { p = end + 1; v.minor = (int)std::strtol(p, &end, 10); }
        if (*end == '.') { p = end + 1; v.patch = (int)std::strtol(p, &end, 10); }
    } catch (...) {}
    return v;
}

std::string SemVer::str() const {
    return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}

// ── SemVerRange ───────────────────────────────────────────────────────────────

bool SemVerRange::Atom::matches(const SemVer& v) const {
    switch (op) {
        case Op::Any: return true;
        case Op::Eq:  return v == ver;
        case Op::Gt:  return v >  ver;
        case Op::Gte: return v >= ver;
        case Op::Lt:  return v <  ver;
        case Op::Lte: return v <= ver;
    }
    return true;
}

bool SemVerRange::matches(const SemVer& v) const {
    for (const auto& a : atoms)
        if (!a.matches(v)) return false;
    return true;
}

SemVerRange SemVerRange::parse(const std::string& s) {
    SemVerRange range;
    if (s.empty() || s == "*") return range; // any

    // Split by comma (AND).
    std::istringstream ss(s);
    std::string token;
    while (std::getline(ss, token, ',')) {
        // Trim whitespace.
        size_t first = token.find_first_not_of(" \t");
        if (first == std::string::npos) continue;
        token = token.substr(first);
        token.erase(token.find_last_not_of(" \t") + 1);
        if (token.empty()) continue;

        // Caret: ^1.2.3 → >=1.2.3,<2.0.0 (or <0.3.0 / <0.0.4 for 0.x)
        if (token[0] == '^') {
            SemVer base = SemVer::parse(token.substr(1));
            range.atoms.push_back({Atom::Op::Gte, base});
            SemVer upper;
            if (base.major > 0)       upper = {base.major + 1, 0, 0};
            else if (base.minor > 0)  upper = {0, base.minor + 1, 0};
            else                      upper = {0, 0, base.patch + 1};
            range.atoms.push_back({Atom::Op::Lt, upper});
            continue;
        }

        // Tilde: ~1.2.3 → >=1.2.3,<1.3.0 | ~1.2 → >=1.2.0,<1.3.0 | ~1 → >=1.0.0,<2.0.0
        if (token[0] == '~') {
            std::string rest = token.substr(1);
            long dots = std::count(rest.begin(), rest.end(), '.');
            SemVer base = SemVer::parse(rest);
            range.atoms.push_back({Atom::Op::Gte, base});
            SemVer upper;
            if (dots >= 1) upper = {base.major, base.minor + 1, 0};
            else           upper = {base.major + 1, 0, 0};
            range.atoms.push_back({Atom::Op::Lt, upper});
            continue;
        }

        // Comparison operators: >=, >, <=, <
        if (token.size() >= 2 && token[0] == '>' && token[1] == '=') {
            range.atoms.push_back({Atom::Op::Gte, SemVer::parse(token.substr(2))});
            continue;
        }
        if (token.size() >= 2 && token[0] == '<' && token[1] == '=') {
            range.atoms.push_back({Atom::Op::Lte, SemVer::parse(token.substr(2))});
            continue;
        }
        if (token[0] == '>') {
            range.atoms.push_back({Atom::Op::Gt, SemVer::parse(token.substr(1))});
            continue;
        }
        if (token[0] == '<') {
            range.atoms.push_back({Atom::Op::Lt, SemVer::parse(token.substr(1))});
            continue;
        }

        // Wildcard shorthand: 1.x or 1.*
        if (token.find(".x") != std::string::npos || token.find(".*") != std::string::npos) {
            std::string base_str = token.substr(0, token.find_last_of('.'));
            SemVer base = SemVer::parse(base_str);
            range.atoms.push_back({Atom::Op::Gte, base});
            range.atoms.push_back({Atom::Op::Lt,  {base.major + 1, 0, 0}});
            continue;
        }

        // Exact version.
        range.atoms.push_back({Atom::Op::Eq, SemVer::parse(token)});
    }
    return range;
}

// ── DependencyGraph ───────────────────────────────────────────────────────────

void DependencyGraph::add_edge(const std::string& requirer,
                               const std::string& requirer_version,
                               const std::string& dep_name,
                               const Constraint&  c)
{
    std::string constraint_desc;
    switch (c.dep.ref.kind) {
        case GitRefKind::Tag:    constraint_desc = "tag "    + c.dep.ref.value; break;
        case GitRefKind::Branch: constraint_desc = "branch " + c.dep.ref.value; break;
        case GitRefKind::Rev:    constraint_desc = "rev "    + c.dep.ref.value.substr(0, 8); break;
    }
    edges_[dep_name].push_back({requirer, requirer_version, constraint_desc});
}

std::vector<std::vector<WhyEntry>>
DependencyGraph::why(const std::string& package) const {
    auto it = edges_.find(package);
    if (it == edges_.end()) return {};
    std::vector<std::vector<WhyEntry>> result;
    for (const auto& e : it->second)
        result.push_back({e});
    return result;
}

void DependencyGraph::print_why(const std::string& package,
                                const std::string& resolved_version) const
{
    auto chains = why(package);
    if (chains.empty()) {
        std::cout << package << " is not a dependency of this project.\n";
        return;
    }
    std::cout << package << " v" << resolved_version << " is included because:\n\n";
    for (const auto& chain : chains) {
        for (const auto& e : chain) {
            std::cout << "  " << e.package << " v" << e.version
                      << "  (requires " << package << " " << e.constraint << ")\n";
        }
    }
    std::cout << "\nresolved: " << package << " " << resolved_version
              << " satisfies all " << chains.size() << " constraint(s)\n";
}

std::vector<std::string> DependencyGraph::all_packages() const {
    std::vector<std::string> pkgs;
    for (const auto& [k,_] : edges_) pkgs.push_back(k);
    return pkgs;
}

// ── MVSResolver ───────────────────────────────────────────────────────────────

std::optional<Constraint> MVSResolver::select(
    const std::string& name,
    const std::vector<Constraint>& constraints)
{
    if (constraints.empty()) return std::nullopt;

    // All constraints must share the same git URL.
    const std::string& url = constraints[0].dep.git_url;
    for (const auto& c : constraints) {
        if (c.dep.git_url != url) {
            // Different URLs for the same package name → E001 conflict.
            ResolveError err;
            err.is_conflict = true;
            err.package     = name;
            for (const auto& cc : constraints)
                err.conflict_chain.push_back({
                    cc.requirer,
                    "git=" + cc.dep.git_url + " " +
                    (cc.dep.ref.kind == GitRefKind::Tag    ? "tag="    :
                     cc.dep.ref.kind == GitRefKind::Branch ? "branch=" : "rev=")
                    + cc.dep.ref.value
                });
            errors_.push_back(std::move(err));
            return std::nullopt;
        }
    }

    // Among tag constraints, pick the highest semver.
    // Branch/rev constraints require exact match or conflict.
    std::optional<Constraint> best;
    for (const auto& c : constraints) {
        if (c.dep.ref.kind == GitRefKind::Tag) {
            if (!best || c.semver > best->semver)
                best = c;
        } else {
            // branch or rev: all must agree
            if (best && best->dep.ref.value != c.dep.ref.value &&
                best->dep.ref.kind != GitRefKind::Tag)
            {
                ResolveError err;
                err.is_conflict = true;
                err.package     = name;
                for (const auto& cc : constraints)
                    err.conflict_chain.push_back({cc.requirer, cc.dep.ref.value});
                errors_.push_back(std::move(err));
                return std::nullopt;
            }
            if (!best) best = c;
        }
    }
    return best;
}

std::map<std::string, ResolvedPackage>
MVSResolver::resolve(const Manifest& root, FetchManifestFn fetch_manifest)
{
    // BFS collecting all constraints.
    struct QueueEntry {
        std::string name;
        GitDep      dep;
        std::string requirer;
        std::string requirer_version;
    };

    std::map<std::string, std::vector<Constraint>> all_constraints;
    std::map<std::string, bool>                    visited;
    std::queue<QueueEntry>                         queue;

    auto enqueue = [&](const std::string& pkg_name, const GitDep& dep,
                       const std::string& requirer, const std::string& req_ver) {
        Constraint c;
        c.name             = pkg_name;
        c.requirer         = requirer;
        c.dep              = dep;
        c.semver           = (dep.ref.kind == GitRefKind::Tag)
                                ? SemVer::parse(dep.ref.value)
                                : SemVer{};
        all_constraints[pkg_name].push_back(c);
        graph_.add_edge(requirer, req_ver, pkg_name, c);

        if (!visited[pkg_name]) {
            visited[pkg_name] = true;
            queue.push({pkg_name, dep, requirer, req_ver});
        }
    };

    // Seed from root manifest.
    for (const auto& [name, dep] : root.dependencies)
        enqueue(name, dep, "root", root.version);

    if (include_dev_)
        for (const auto& [name, dep] : root.dev_dependencies)
            enqueue(name, dep, "root", root.version);

    // BFS.
    while (!queue.empty()) {
        auto entry = queue.front();
        queue.pop();

        auto child_manifest = fetch_manifest(entry.name, entry.dep);
        if (!child_manifest) {
            // fetch_manifest already pushed an E002 error
            continue;
        }

        // Enqueue transitive deps.
        for (const auto& [name, dep] : child_manifest->dependencies)
            enqueue(name, dep, entry.name, child_manifest->version);
    }

    if (!errors_.empty()) return {};

    // MVS selection: max semver per package.
    std::map<std::string, ResolvedPackage> resolved;
    for (const auto& [name, constraints] : all_constraints) {
        auto selected = select(name, constraints);
        if (!selected) continue; // error already recorded

        ResolvedPackage rp;
        rp.name    = name;
        rp.version = selected->semver.str();
        rp.git_url = selected->dep.git_url;
        rp.ref     = selected->dep.ref;
        resolved[name] = std::move(rp);
    }

    return errors_.empty() ? resolved : std::map<std::string, ResolvedPackage>{};
}

} // namespace flyp
