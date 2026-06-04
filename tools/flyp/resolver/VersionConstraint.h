#pragma once

#include "../manifest/Manifest.h"
#include <string>
#include <tuple>
#include <vector>

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

// ── SemVerRange ───────────────────────────────────────────────────────────────
// Parses semver range expressions and tests versions against them.
//
// Supported syntax (comma = AND):
//   *           any version
//   1.2.3       exact
//   ^1.2.3      compatible (>=1.2.3,<2.0.0; or <0.3.0 if major==0)
//   ~1.2.3      patch-level (>=1.2.3,<1.3.0)
//   ~1.2        minor-level (>=1.2.0,<1.3.0)
//   ~1          major-level (>=1.0.0,<2.0.0)
//   >=1.0.0     at least
//   >1.0.0      strictly greater
//   <=2.0.0     at most
//   <2.0.0      strictly less
//   >=1.0,<2.0  combined (comma-separated atoms are AND-ed)

struct SemVerRange {
    struct Atom {
        enum class Op { Any, Eq, Gt, Gte, Lt, Lte };
        Op     op  = Op::Any;
        SemVer ver = {};

        bool matches(const SemVer& v) const;
    };

    std::vector<Atom> atoms; // all must match (AND)

    // Parse a range string. Returns an "any" range on parse failure.
    static SemVerRange parse(const std::string& s);

    // Returns true if v satisfies all atoms.
    bool matches(const SemVer& v) const;

    bool is_any() const { return atoms.empty(); }
};

// ── Constraint ────────────────────────────────────────────────────────────────

// A constraint as seen by the resolver: which package, from which requirer,
// with what git ref.
struct Constraint {
    std::string name;        // package name
    std::string requirer;    // who depends on it ("root" or package name)
    GitDep      dep;         // the original dependency entry
    SemVer      semver;      // parsed from tag (0.0.0 for branch/rev)
};

} // namespace flyp
