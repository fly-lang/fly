#pragma once

#include "VersionConstraint.h"
#include <map>
#include <string>
#include <vector>

namespace flyp {

// One node in a "why" explanation chain.
struct WhyEntry {
    std::string package;    // the requirer
    std::string version;    // requirer's resolved version ("root" for the project)
    std::string constraint; // human-readable ref, e.g. "tag v1.2.0"
};

// Directed dependency graph used for:
//   1. Tracking who-requires-what during BFS.
//   2. Answering `flyp why <package>`.
class DependencyGraph {
public:
    // Record that `requirer` depends on `dep_name` with the given constraint.
    void add_edge(const std::string& requirer,
                  const std::string& requirer_version,
                  const std::string& dep_name,
                  const Constraint&  constraint);

    // Returns all "why" chains leading to `package`.
    // Each inner vector is one path from root to the package.
    std::vector<std::vector<WhyEntry>> why(const std::string& package) const;

    // Print `flyp why <package>` output to stdout.
    void print_why(const std::string& package,
                   const std::string& resolved_version) const;

    // All packages recorded in the graph.
    std::vector<std::string> all_packages() const;

private:
    // dep_name → list of (requirer, requirer_version, constraint_desc)
    std::map<std::string, std::vector<WhyEntry>> edges_;
};

} // namespace flyp
