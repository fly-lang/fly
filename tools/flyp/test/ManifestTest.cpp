#include <gtest/gtest.h>

#include "../manifest/Manifest.h"
#include "../manifest/Lockfile.h"

#include <filesystem>
#include <fstream>

namespace flyp::test {

namespace {

// Write a temporary fly.toml and return its path.
std::filesystem::path write_toml(const std::string& content) {
    static int counter = 0;
    auto tmp = std::filesystem::temp_directory_path()
             / ("flyp_test_" + std::to_string(++counter) + ".toml");
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

TEST(ManifestTest, ParseTargetBin) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "0.1.0"

[targets]
app = { path = "src/main.fly" }
)");
    auto m = Manifest::parse(p);
    ASSERT_EQ(m.targets.size(), 1u);
    EXPECT_EQ(m.targets[0].key,  "app");
    EXPECT_EQ(m.targets[0].name, "app");
    EXPECT_EQ(m.targets[0].path, "src/main.fly");
    EXPECT_TRUE(m.targets[0].is_bin());
    EXPECT_FALSE(m.targets[0].is_lib());
    std::filesystem::remove(p);
}

TEST(ManifestTest, ParseTargetBinWithNameOverride) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "0.1.0"

[targets]
app = { name = "MyApp", path = "src/main.fly" }
)");
    auto m = Manifest::parse(p);
    ASSERT_EQ(m.targets.size(), 1u);
    EXPECT_EQ(m.targets[0].key,  "app");
    EXPECT_EQ(m.targets[0].name, "MyApp");
    EXPECT_TRUE(m.targets[0].is_bin());
    std::filesystem::remove(p);
}

TEST(ManifestTest, ParseTargetLib) {
    auto p = write_toml(R"(
[package]
name    = "mylib"
version = "0.2.0"

[targets]
mylib = { path = "src/lib.fly", lib = "static" }
)");
    auto m = Manifest::parse(p);
    ASSERT_EQ(m.targets.size(), 1u);
    EXPECT_EQ(m.targets[0].key,  "mylib");
    EXPECT_EQ(m.targets[0].name, "mylib");
    EXPECT_EQ(m.targets[0].path, "src/lib.fly");
    EXPECT_EQ(m.targets[0].lib,  "static");
    EXPECT_TRUE(m.targets[0].is_lib());
    EXPECT_FALSE(m.targets[0].is_bin());
    std::filesystem::remove(p);
}

TEST(ManifestTest, ParseTargetLibDynamic) {
    auto p = write_toml(R"(
[package]
name    = "mylib"
version = "0.2.0"

[targets]
mylib = { name = "MyLib", path = "src/lib.fly", lib = "dynamic" }
)");
    auto m = Manifest::parse(p);
    ASSERT_EQ(m.targets.size(), 1u);
    EXPECT_EQ(m.targets[0].name, "MyLib");
    EXPECT_EQ(m.targets[0].lib,  "dynamic");
    EXPECT_TRUE(m.targets[0].is_lib());
    std::filesystem::remove(p);
}

TEST(ManifestTest, ParseTargetsMixed) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "0.1.0"

[targets]
app   = { path = "src/main.fly" }
cli   = { path = "src/cli.fly" }
mylib = { path = "src/lib.fly", lib = "static" }
)");
    auto m = Manifest::parse(p);
    ASSERT_EQ(m.targets.size(), 3u);
    EXPECT_TRUE(m.targets[0].is_bin());
    EXPECT_TRUE(m.targets[1].is_bin());
    EXPECT_TRUE(m.targets[2].is_lib());
    std::filesystem::remove(p);
}

TEST(ManifestTest, ParseHooks) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "0.1.0"

[hooks]
pre-build  = "python scripts/codegen.py"
post-build = "strip target/release/myapp"
)");
    auto m = Manifest::parse(p);
    EXPECT_EQ(m.hooks.pre_build,  "python scripts/codegen.py");
    EXPECT_EQ(m.hooks.post_build, "strip target/release/myapp");
    std::filesystem::remove(p);
}

TEST(ManifestTest, ParseHooksPartial) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "0.1.0"

[hooks]
pre-build = "make codegen"
)");
    auto m = Manifest::parse(p);
    EXPECT_EQ(m.hooks.pre_build,  "make codegen");
    EXPECT_TRUE(m.hooks.post_build.empty());
    std::filesystem::remove(p);
}

TEST(ManifestTest, NoHooksIsEmpty) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "0.1.0"
)");
    auto m = Manifest::parse(p);
    EXPECT_TRUE(m.hooks.pre_build.empty());
    EXPECT_TRUE(m.hooks.post_build.empty());
    std::filesystem::remove(p);
}

TEST(ManifestTest, ParseWorkspaceRoot) {
    auto p = write_toml(R"(
[workspace]
members = ["app", "libs/core"]
)");
    auto m = Manifest::parse(p);
    EXPECT_TRUE(m.is_workspace_root());
    ASSERT_TRUE(m.workspace.has_value());
    ASSERT_EQ(m.workspace->members.size(), 2u);
    EXPECT_EQ(m.workspace->members[0], "app");
    EXPECT_EQ(m.workspace->members[1], "libs/core");
    std::filesystem::remove(p);
}

TEST(ManifestTest, ParsePathDependency) {
    auto p = write_toml(R"(
[package]
name    = "app"
version = "0.1.0"

[dependencies]
core    = { path = "../libs/core" }
fly-std = { git = "https://github.com/flylang/fly-std.git", tag = "v1.0.0" }
)");
    auto m = Manifest::parse(p);
    ASSERT_EQ(m.dependencies.size(), 2u);
    EXPECT_TRUE(m.dependencies.at("core").is_path_dep());
    EXPECT_EQ(m.dependencies.at("core").path, "../libs/core");
    EXPECT_FALSE(m.dependencies.at("fly-std").is_path_dep());
    std::filesystem::remove(p);
}

TEST(ManifestTest, NonWorkspaceHasNoWorkspaceConfig) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "0.1.0"
)");
    auto m = Manifest::parse(p);
    EXPECT_FALSE(m.is_workspace_root());
    EXPECT_FALSE(m.workspace.has_value());
    std::filesystem::remove(p);
}

TEST(ManifestTest, ParseProfiles) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "0.1.0"

[profiles]
debug   = { opt-level = 0, debug-info = true,  assertions = true,  lto = false, strip = false }
release = { opt-level = 3, debug-info = false, assertions = false, lto = true,  strip = true  }
)");
    auto m = Manifest::parse(p);
    ASSERT_TRUE(m.profiles.count("debug"));
    ASSERT_TRUE(m.profiles.count("release"));
    auto& dbg = m.profiles.at("debug");
    auto& rel = m.profiles.at("release");
    EXPECT_EQ(dbg.opt_level,  0);
    EXPECT_TRUE(dbg.debug_info);
    EXPECT_TRUE(dbg.assertions);
    EXPECT_EQ(rel.opt_level,  3);
    EXPECT_FALSE(rel.debug_info);
    EXPECT_TRUE(rel.lto);
    EXPECT_TRUE(rel.strip);
    std::filesystem::remove(p);
}

TEST(ManifestTest, ParseCustomProfile) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "0.1.0"

[profiles]
ci = { opt-level = 2, debug-info = true, assertions = false, lto = false, strip = false }
)");
    auto m = Manifest::parse(p);
    ASSERT_TRUE(m.profiles.count("ci"));
    EXPECT_EQ(m.profiles.at("ci").opt_level, 2);
    EXPECT_FALSE(m.profiles.at("ci").assertions);
    // debug and release are always auto-added
    EXPECT_TRUE(m.profiles.count("debug"));
    EXPECT_TRUE(m.profiles.count("release"));
    std::filesystem::remove(p);
}

TEST(ManifestTest, ProfileDefaultsAppliedWhenAbsent) {
    auto p = write_toml(R"(
[package]
name    = "myapp"
version = "0.1.0"
)");
    auto m = Manifest::parse(p);
    ASSERT_TRUE(m.profiles.count("debug"));
    ASSERT_TRUE(m.profiles.count("release"));
    EXPECT_EQ(m.profiles.at("debug").opt_level,    0);
    EXPECT_TRUE(m.profiles.at("debug").debug_info);
    EXPECT_EQ(m.profiles.at("release").opt_level,  3);
    EXPECT_FALSE(m.profiles.at("release").debug_info);
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
