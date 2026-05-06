#pragma once

#include "../manifest/Manifest.h"
#include <string>
#include <tuple>

namespace flyp {

// Comparable version triplet parsed from a semver string or tag like "v1.2.3".
struct SemVer {
    int major = 0, minor = 0, patch = 0;

    static SemVer parse(const std::string& s); // strips leading 'v', returns 0.0.0 on failure
    std::string   str() const;

    bool operator==(const SemVer& o) const {
        return std::tie(major, minor, patch) == std::tie(o.major, o.minor, o.patch);
    }
    bool operator<(const SemVer& o) const {
        return std::tie(major, minor, patch) < std::tie(o.major, o.minor, o.patch);
    }
    bool operator>(const SemVer& o)  const { return o < *this; }
    bool operator<=(const SemVer& o) const { return !(o < *this); }
    bool operator>=(const SemVer& o) const { return !(*this < o); }
};

// A constraint as seen by the resolver: which package, from which requirer,
// with what git ref.
struct Constraint {
    std::string name;        // package name
    std::string requirer;    // who depends on it ("root" or package name)
    GitDep      dep;         // the original dependency entry
    SemVer      semver;      // parsed from tag (0.0.0 for branch/rev)
};

} // namespace flyp
