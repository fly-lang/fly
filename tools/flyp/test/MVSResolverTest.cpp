#include <gtest/gtest.h>

#include "../resolver/MVSResolver.h"
#include "../resolver/VersionConstraint.h"
#include "../manifest/Manifest.h"
#include "../fetcher/Cache.h"

namespace flyp::test {

// ── SemVer ───────────────────────────────────────────────────────────────────

TEST(SemVerTest, ParseTag) {
    auto v = SemVer::parse("v1.2.3");
    EXPECT_EQ(v.major, 1);
    EXPECT_EQ(v.minor, 2);
    EXPECT_EQ(v.patch, 3);
}

TEST(SemVerTest, ParseNoPrefix) {
    auto v = SemVer::parse("0.10.5");
    EXPECT_EQ(v.major, 0);
    EXPECT_EQ(v.minor, 10);
    EXPECT_EQ(v.patch, 5);
}

TEST(SemVerTest, ParseInvalidReturnsZero) {
    auto v = SemVer::parse("not-a-version");
    EXPECT_EQ(v.major, 0);
    EXPECT_EQ(v.minor, 0);
    EXPECT_EQ(v.patch, 0);
}

TEST(SemVerTest, CompareOrdering) {
    EXPECT_LT(SemVer::parse("v1.0.0"), SemVer::parse("v2.0.0"));
    EXPECT_LT(SemVer::parse("v1.2.0"), SemVer::parse("v1.3.0"));
    EXPECT_LT(SemVer::parse("v1.2.3"), SemVer::parse("v1.2.4"));
    EXPECT_EQ(SemVer::parse("v1.0.0"), SemVer::parse("1.0.0"));
}

TEST(SemVerTest, StrRoundTrip) {
    EXPECT_EQ(SemVer::parse("v3.14.15").str(), "3.14.15");
}

// ── Cache::parse_url ─────────────────────────────────────────────────────────

TEST(CacheTest, ParseHttpsUrl) {
    auto [host, owner, repo] = Cache::parse_url(
        "https://github.com/example/mylib.git");
    EXPECT_EQ(host,  "github.com");
    EXPECT_EQ(owner, "example");
    EXPECT_EQ(repo,  "mylib");
}

TEST(CacheTest, ParseSshUrl) {
    auto [host, owner, repo] = Cache::parse_url(
        "git@github.com:example/mylib.git");
    EXPECT_EQ(host,  "github.com");
    EXPECT_EQ(owner, "example");
    EXPECT_EQ(repo,  "mylib");
}

TEST(CacheTest, ParseNoGitSuffix) {
    auto [host, owner, repo] = Cache::parse_url(
        "https://github.com/example/mylib");
    EXPECT_EQ(repo, "mylib");
}

// ── MVSResolver ──────────────────────────────────────────────────────────────

// Build a minimal Manifest with the given deps.
Manifest make_root(const std::map<std::string, GitDep>& deps) {
    Manifest m;
    m.name         = "root";
    m.version      = "0.1.0";
    m.dependencies = deps;
    return m;
}

GitDep tag_dep(const std::string& url, const std::string& tag) {
    GitDep d;
    d.git_url = url;
    d.ref     = {GitRefKind::Tag, tag};
    return d;
}

GitDep branch_dep(const std::string& url, const std::string& branch) {
    GitDep d;
    d.git_url = url;
    d.ref     = {GitRefKind::Branch, branch};
    return d;
}

// Returns a flat manifest with no transitive deps.
Manifest leaf(const std::string& name, const std::string& version) {
    Manifest m;
    m.name    = name;
    m.version = version;
    return m;
}

TEST(MVSResolverTest, SingleDep) {
    auto root = make_root({{"libA", tag_dep("https://host/A.git", "v1.0.0")}});

    MVSResolver resolver;
    auto resolved = resolver.resolve(root, [](const std::string& name, const GitDep&) {
        return leaf(name, "1.0.0");
    });

    EXPECT_TRUE(resolver.resolve_errors().empty());
    ASSERT_EQ(resolved.size(), 1u);
    EXPECT_EQ(resolved["libA"].version, "1.0.0");
}

TEST(MVSResolverTest, MaxVersionSelected) {
    // root depends on libA v1.0.0 and libB v1.0.0
    // libB depends on libA v2.0.0
    // MVS should select libA v2.0.0
    std::map<std::string, GitDep> root_deps = {
        {"libA", tag_dep("https://host/A.git", "v1.0.0")},
        {"libB", tag_dep("https://host/B.git", "v1.0.0")},
    };
    auto root = make_root(root_deps);

    auto fetch_manifest = [](const std::string& name, const GitDep& dep) -> std::optional<Manifest> {
        if (name == "libB") {
            Manifest m;
            m.name    = "libB";
            m.version = "1.0.0";
            m.dependencies["libA"] = tag_dep("https://host/A.git", "v2.0.0");
            return m;
        }
        return leaf(name, SemVer::parse(dep.ref.value).str());
    };

    MVSResolver resolver;
    auto resolved = resolver.resolve(root, fetch_manifest);

    EXPECT_TRUE(resolver.resolve_errors().empty());
    EXPECT_EQ(resolved["libA"].version, "2.0.0");
}

TEST(MVSResolverTest, ConflictingUrlsIsE001) {
    std::map<std::string, GitDep> root_deps = {
        {"libA", tag_dep("https://host1/A.git", "v1.0.0")},
        {"libB", tag_dep("https://host2/B.git", "v1.0.0")},
    };
    auto root = make_root(root_deps);

    // libB requires libA from a DIFFERENT URL
    auto fetch_manifest = [](const std::string& name, const GitDep& dep) -> std::optional<Manifest> {
        if (name == "libB") {
            Manifest m;
            m.name    = "libB";
            m.version = "1.0.0";
            m.dependencies["libA"] = tag_dep("https://DIFFERENT-HOST/A.git", "v1.0.0");
            return m;
        }
        return leaf(name, "1.0.0");
    };

    MVSResolver resolver;
    auto resolved = resolver.resolve(root, fetch_manifest);

    EXPECT_FALSE(resolver.resolve_errors().empty());
    EXPECT_TRUE(resolved.empty());
    EXPECT_TRUE(resolver.resolve_errors()[0].is_conflict);
}

TEST(MVSResolverTest, NoDepsResolvesEmpty) {
    auto root = make_root({});
    MVSResolver resolver;
    auto resolved = resolver.resolve(root, [](const std::string&, const GitDep&) {
        return std::optional<Manifest>{};
    });
    EXPECT_TRUE(resolver.resolve_errors().empty());
    EXPECT_TRUE(resolved.empty());
}

TEST(MVSResolverTest, FetchFailureIsCallerResponsibility) {
    auto root = make_root({{"libX", tag_dep("https://host/X.git", "v9.9.9")}});
    MVSResolver resolver;
    // fetch_manifest returns nullopt — the resolver does NOT record an error itself.
    // E002 is the caller's responsibility (Commands.cpp records it in its own list).
    // The constraint was already added to all_constraints before the fetch attempt,
    // so the package is still present in the resolved map.
    auto resolved = resolver.resolve(root, [](const std::string&, const GitDep&) {
        return std::nullopt;
    });
    EXPECT_TRUE(resolver.resolve_errors().empty()); // no resolver-level error
    EXPECT_FALSE(resolved.empty());                 // constraint still resolved
    EXPECT_EQ(resolved.count("libX"), 1u);
}

} // namespace flyp::test
