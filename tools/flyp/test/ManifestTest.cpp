#include <gtest/gtest.h>

#include "../manifest/Manifest.h"
#include "../manifest/Lockfile.h"

#include <filesystem>
#include <fstream>
#include <unistd.h>

namespace flyp::test {

namespace {

// Write a temporary fly.toml and return its path.
std::filesystem::path write_toml(const std::string& content) {
    auto tmp = std::filesystem::temp_directory_path()
             / ("flyp_test_" + std::to_string(::getpid()) + ".toml");
    std::ofstream f(tmp);
    f << content;
    return tmp;
}

} // namespace

// ── Manifest parsing ─────────────────────────────────────────────────────────

TEST(ManifestTest, ParseMinimal) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "1.0.0"
)");
    auto m = Manifest::parse(p);
    EXPECT_EQ(m.name,    "myapp");
    EXPECT_EQ(m.version, "1.0.0");
    std::filesystem::remove(p);
}

TEST(ManifestTest, ParseDependencies) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "0.1.0"

[dependencies]
mylib = { git = "https://github.com/example/mylib.git", tag = "v1.2.0" }
)");
    auto m = Manifest::parse(p);
    ASSERT_EQ(m.dependencies.size(), 1u);
    auto& dep = m.dependencies.at("mylib");
    EXPECT_EQ(dep.git_url,        "https://github.com/example/mylib.git");
    EXPECT_EQ(dep.ref.kind,       GitRefKind::Tag);
    EXPECT_EQ(dep.ref.value,      "v1.2.0");
    std::filesystem::remove(p);
}

TEST(ManifestTest, ParseBranchDep) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "0.1.0"

[dependencies]
alpha = { git = "https://github.com/example/alpha.git", branch = "main" }
)");
    auto m = Manifest::parse(p);
    auto& dep = m.dependencies.at("alpha");
    EXPECT_EQ(dep.ref.kind,  GitRefKind::Branch);
    EXPECT_EQ(dep.ref.value, "main");
    std::filesystem::remove(p);
}

TEST(ManifestTest, ParseRevDep) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "0.1.0"

[dependencies]
beta = { git = "https://github.com/example/beta.git", rev = "abc123def456abc123def456abc123def456abc1" }
)");
    auto m = Manifest::parse(p);
    auto& dep = m.dependencies.at("beta");
    EXPECT_EQ(dep.ref.kind,  GitRefKind::Rev);
    EXPECT_EQ(dep.ref.value, "abc123def456abc123def456abc123def456abc1");
    std::filesystem::remove(p);
}

TEST(ManifestTest, InvalidNameRejected) {
    auto p = write_toml(R"(
[package]
name    = "My Package!"
version = "0.1.0"
)");
    EXPECT_THROW(Manifest::parse(p), std::runtime_error);
    std::filesystem::remove(p);
}

TEST(ManifestTest, InvalidVersionRejected) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "1.0"
)");
    EXPECT_THROW(Manifest::parse(p), std::runtime_error);
    std::filesystem::remove(p);
}

TEST(ManifestTest, MissingNameRejected) {
    auto p = write_toml(R"(
[package]
version = "1.0.0"
)");
    EXPECT_THROW(Manifest::parse(p), std::runtime_error);
    std::filesystem::remove(p);
}

TEST(ManifestTest, ParseProfiles) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "0.1.0"

[profile.debug]
opt-level  = 0
debug-info = true
assertions = true

[profile.release]
opt-level  = 3
debug-info = false
lto        = true
strip      = true
assertions = false
)");
    auto m = Manifest::parse(p);
    EXPECT_EQ(m.profile_debug.opt_level,    0);
    EXPECT_TRUE(m.profile_debug.debug_info);
    EXPECT_TRUE(m.profile_debug.assertions);
    EXPECT_EQ(m.profile_release.opt_level,  3);
    EXPECT_FALSE(m.profile_release.debug_info);
    EXPECT_TRUE(m.profile_release.lto);
    EXPECT_TRUE(m.profile_release.strip);
    std::filesystem::remove(p);
}

// ── Lockfile ─────────────────────────────────────────────────────────────────

TEST(LockfileTest, RoundTrip) {
    Lockfile lf;
    lf.format_version = 1;
    lf.flyp_version   = "0.1.0";
    lf.toml_checksum  = "sha256:abcdef";

    LockedPackage p;
    p.name     = "mylib";
    p.version  = "1.2.0";
    p.source   = "git+https://github.com/example/mylib.git";
    p.rev      = std::string(40, 'a');
    p.tag      = "v1.2.0";
    p.checksum = "sha256:deadbeef";
    p.deps     = {"depA", "depB"};
    lf.packages.push_back(p);

    auto tmp = std::filesystem::temp_directory_path()
             / ("flyp_lock_test_" + std::to_string(::getpid()) + ".lock");
    lf.write(tmp);

    auto lf2 = Lockfile::read(tmp);
    EXPECT_EQ(lf2.format_version, 1);
    EXPECT_EQ(lf2.flyp_version,   "0.1.0");
    ASSERT_EQ(lf2.packages.size(), 1u);

    auto& p2 = lf2.packages[0];
    EXPECT_EQ(p2.name,     "mylib");
    EXPECT_EQ(p2.version,  "1.2.0");
    EXPECT_EQ(p2.rev,      std::string(40, 'a'));
    EXPECT_EQ(p2.tag,      "v1.2.0");
    EXPECT_EQ(p2.checksum, "sha256:deadbeef");
    ASSERT_EQ(p2.deps.size(), 2u);
    EXPECT_EQ(p2.deps[0], "depA");
    EXPECT_EQ(p2.deps[1], "depB");

    std::filesystem::remove(tmp);
}

TEST(LockfileTest, MissingFileIsEmpty) {
    auto lf = Lockfile::read("/nonexistent/path/fly.lock");
    EXPECT_TRUE(lf.packages.empty());
}

TEST(LockfileTest, FindReturnsNullptrForMissing) {
    Lockfile lf;
    EXPECT_EQ(lf.find("missing"), nullptr);
}

} // namespace flyp::test
