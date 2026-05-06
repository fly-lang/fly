#pragma once

#include "DependencyGraph.h"
#include "VersionConstraint.h"
#include "../manifest/Manifest.h"
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace flyp {

// Resolved package: final selected ref + metadata filled after fetch.
struct ResolvedPackage {
    std::string name;
    std::string version;   // semver string, e.g. "1.2.0"
    std::string git_url;
    std::string rev;       // full 40-char commit hash
    GitRef      ref;       // original ref from fly.toml
    std::string checksum;  // sha256:... of the fetched tree
    std::filesystem::path local_path; // path in cache
    std::vector<std::string> dep_names; // transitive dependency names
};

// Callback used by the resolver to fetch a dependency manifest.
// Given a dep name + GitDep, returns the fly.toml of that package.
using FetchManifestFn = std::function<
    std::optional<Manifest>(const std::string& name, const GitDep& dep)>;

// Error encountered during resolution (E001 or E002).
struct ResolveError {
    bool        is_conflict; // true=E001, false=E002
    std::string package;
    // E001
    std::vector<std::pair<std::string,std::string>> conflict_chain; // {requirer, constraint}
    // E002
    std::string ref_kind, ref_value, git_url;
    std::vector<std::string> available;
};

class MVSResolver {
public:
    explicit MVSResolver(bool include_dev = false) : include_dev_(include_dev) {}

    // Run MVS resolution starting from `root`.
    // `fetch_manifest` is called for every dependency discovered.
    // Returns the resolved map on success.
    // On failure, resolve_errors() is non-empty.
    std::map<std::string, ResolvedPackage>
    resolve(const Manifest& root, FetchManifestFn fetch_manifest);

    const std::vector<ResolveError>& resolve_errors() const { return errors_; }

    // Access the dependency graph (for `flyp why`).
    const DependencyGraph& graph() const { return graph_; }

private:
    bool                       include_dev_;
    DependencyGraph            graph_;
    std::vector<ResolveError>  errors_;

    // Select the maximum (highest semver) among all constraints for a package.
    // Returns nullopt if there's an irresolvable conflict.
    std::optional<Constraint> select(
        const std::string& name,
        const std::vector<Constraint>& constraints);
};

} // namespace flyp
