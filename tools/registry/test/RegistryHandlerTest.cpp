#include <gtest/gtest.h>
#include "../include/RegistryHandler.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace flyp::registry::test {

// ── Test fixture ──────────────────────────────────────────────────────────────

class RegistryHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        static int counter = 0;
        storage_ = std::filesystem::temp_directory_path()
                 / ("reg_test_" + std::to_string(++counter));
        std::filesystem::create_directories(storage_);
        handler_ = std::make_unique<RegistryHandler>(storage_);
    }

    void TearDown() override {
        std::filesystem::remove_all(storage_);
    }

    // Helpers ─────────────────────────────────────────────────────────────────

    Request make_get(const std::string& path, const std::string& query = "") const {
        return {"GET", path, query, {}, ""};
    }

    Request make_post(const std::string& path, const std::string& body) const {
        return {"POST", path, "", {}, body};
    }

    // Write a minimal fly.toml-containing tarball to storage manually.
    void seed_package(const std::string& name, const std::string& version,
                      const std::string& toml_content = "") {
        auto dir = storage_ / name;
        std::filesystem::create_directories(dir);
        // Write a fake tarball (just any bytes so the file exists).
        auto tb = dir / (version + ".tar.gz");
        std::ofstream f(tb, std::ios::binary);
        f << "FAKE_TARBALL";
        f.close();
        // Write fly.toml directly to the version dir (bypass tarball extraction).
        auto ver_dir = dir / version;
        std::filesystem::create_directories(ver_dir);
        std::ofstream tf(ver_dir / "fly.toml");
        tf << (toml_content.empty()
               ? "[package]\nname    = \"" + name + "\"\nversion = \"" + version + "\"\n"
               : toml_content);
    }

    std::filesystem::path storage_;
    std::unique_ptr<RegistryHandler> handler_;
};

// ── Storage helpers ───────────────────────────────────────────────────────────

TEST_F(RegistryHandlerTest, StoragePaths) {
    EXPECT_EQ(handler_->pkg_dir("mylib"),
              storage_ / "mylib");
    EXPECT_EQ(handler_->tarball_path("mylib", "1.0.0"),
              storage_ / "mylib" / "1.0.0.tar.gz");
    EXPECT_EQ(handler_->toml_path("mylib", "1.0.0"),
              storage_ / "mylib" / "1.0.0" / "fly.toml");
}

// ── URL decode ────────────────────────────────────────────────────────────────

TEST_F(RegistryHandlerTest, UrlDecodeBasic) {
    EXPECT_EQ(RegistryHandler::url_decode("hello%20world"), "hello world");
    EXPECT_EQ(RegistryHandler::url_decode("fly-std"),       "fly-std");
    EXPECT_EQ(RegistryHandler::url_decode("a+b"),           "a b");
    EXPECT_EQ(RegistryHandler::url_decode("a%2Bb"),         "a+b");
    EXPECT_EQ(RegistryHandler::url_decode(""),              "");
}

TEST_F(RegistryHandlerTest, UrlDecodeSpecialChars) {
    EXPECT_EQ(RegistryHandler::url_decode("fly%2Dstd"), "fly-std");
    EXPECT_EQ(RegistryHandler::url_decode("%61%62%63"),  "abc");
}

// ── GET /v1/{name} — list versions ───────────────────────────────────────────

TEST_F(RegistryHandlerTest, ListVersionsEmpty) {
    auto resp = handler_->handle(make_get("/v1/unknown-pkg"));
    EXPECT_EQ(resp.status, 404);
}

TEST_F(RegistryHandlerTest, ListVersionsSingle) {
    seed_package("mylib", "1.0.0");
    auto resp = handler_->handle(make_get("/v1/mylib"));
    EXPECT_EQ(resp.status,        200);
    EXPECT_EQ(resp.content_type,  "application/json");
    EXPECT_EQ(resp.body,          "[\"1.0.0\"]");
}

TEST_F(RegistryHandlerTest, ListVersionsMultiple) {
    seed_package("mylib", "1.0.0");
    seed_package("mylib", "1.1.0");
    seed_package("mylib", "2.0.0");
    auto resp = handler_->handle(make_get("/v1/mylib"));
    EXPECT_EQ(resp.status, 200);
    // Versions are sorted alphabetically.
    EXPECT_NE(resp.body.find("1.0.0"), std::string::npos);
    EXPECT_NE(resp.body.find("1.1.0"), std::string::npos);
    EXPECT_NE(resp.body.find("2.0.0"), std::string::npos);
}

TEST_F(RegistryHandlerTest, ListVersionsIgnoresNonTarballs) {
    seed_package("mylib", "1.0.0");
    // Write a non-tarball file in the package dir.
    std::ofstream f(storage_ / "mylib" / "README.md");
    f << "readme";
    auto resp = handler_->handle(make_get("/v1/mylib"));
    EXPECT_EQ(resp.body, "[\"1.0.0\"]");
}

// ── GET /v1/{name}/{version} — metadata ──────────────────────────────────────

TEST_F(RegistryHandlerTest, GetMetadataOk) {
    const std::string toml = "[package]\nname = \"mylib\"\nversion = \"1.0.0\"\n";
    seed_package("mylib", "1.0.0", toml);
    auto resp = handler_->handle(make_get("/v1/mylib/1.0.0"));
    EXPECT_EQ(resp.status, 200);
    EXPECT_EQ(resp.body,   toml);
}

TEST_F(RegistryHandlerTest, GetMetadataVersionNotFound) {
    seed_package("mylib", "1.0.0");
    auto resp = handler_->handle(make_get("/v1/mylib/9.9.9"));
    EXPECT_EQ(resp.status, 404);
}

TEST_F(RegistryHandlerTest, GetMetadataPackageNotFound) {
    auto resp = handler_->handle(make_get("/v1/no-such-pkg/1.0.0"));
    EXPECT_EQ(resp.status, 404);
}

// ── GET /v1/{name}/{version}/download — tarball ───────────────────────────────

TEST_F(RegistryHandlerTest, DownloadOk) {
    seed_package("mylib", "1.0.0");
    auto resp = handler_->handle(make_get("/v1/mylib/1.0.0/download"));
    EXPECT_EQ(resp.status,       200);
    EXPECT_EQ(resp.content_type, "application/octet-stream");
    EXPECT_EQ(resp.body,         "FAKE_TARBALL");
}

TEST_F(RegistryHandlerTest, DownloadNotFound) {
    auto resp = handler_->handle(make_get("/v1/mylib/1.0.0/download"));
    EXPECT_EQ(resp.status, 404);
}

TEST_F(RegistryHandlerTest, DownloadWrongVersion) {
    seed_package("mylib", "1.0.0");
    auto resp = handler_->handle(make_get("/v1/mylib/2.0.0/download"));
    EXPECT_EQ(resp.status, 404);
}

// ── POST /v1/{name}/{version} — publish ───────────────────────────────────────

TEST_F(RegistryHandlerTest, PublishCreatesPackage) {
    auto resp = handler_->handle(make_post("/v1/newpkg/1.0.0", "TARBALL_BYTES"));
    EXPECT_EQ(resp.status, 201);
    EXPECT_TRUE(std::filesystem::exists(
        handler_->tarball_path("newpkg", "1.0.0")));
}

TEST_F(RegistryHandlerTest, PublishTarballContents) {
    const std::string content = "REAL_TARBALL_DATA_12345";
    handler_->handle(make_post("/v1/mylib/2.0.0", content));
    auto tb = handler_->tarball_path("mylib", "2.0.0");
    std::ifstream f(tb, std::ios::binary);
    std::string stored((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
    EXPECT_EQ(stored, content);
}

TEST_F(RegistryHandlerTest, PublishEmptyBodyRejected) {
    auto resp = handler_->handle(make_post("/v1/newpkg/1.0.0", ""));
    EXPECT_EQ(resp.status, 400);
    EXPECT_FALSE(std::filesystem::exists(
        handler_->tarball_path("newpkg", "1.0.0")));
}

TEST_F(RegistryHandlerTest, PublishThenList) {
    handler_->handle(make_post("/v1/mylib/1.0.0", "TARBALL1"));
    handler_->handle(make_post("/v1/mylib/1.1.0", "TARBALL2"));
    auto resp = handler_->handle(make_get("/v1/mylib"));
    EXPECT_EQ(resp.status, 200);
    EXPECT_NE(resp.body.find("1.0.0"), std::string::npos);
    EXPECT_NE(resp.body.find("1.1.0"), std::string::npos);
}

TEST_F(RegistryHandlerTest, PublishOverwrite) {
    handler_->handle(make_post("/v1/mylib/1.0.0", "FIRST"));
    handler_->handle(make_post("/v1/mylib/1.0.0", "SECOND"));
    auto resp = handler_->handle(make_get("/v1/mylib/1.0.0/download"));
    EXPECT_EQ(resp.body, "SECOND");
}

// ── GET /v1/search ────────────────────────────────────────────────────────────

TEST_F(RegistryHandlerTest, SearchEmpty) {
    auto resp = handler_->handle(make_get("/v1/search", "q="));
    EXPECT_EQ(resp.status, 200);
    EXPECT_EQ(resp.body,   "[]");
}

TEST_F(RegistryHandlerTest, SearchAllPackages) {
    seed_package("fly-std", "1.0.0");
    seed_package("fly-io",  "1.0.0");
    auto resp = handler_->handle(make_get("/v1/search", "q="));
    EXPECT_EQ(resp.status, 200);
    EXPECT_NE(resp.body.find("fly-std"), std::string::npos);
    EXPECT_NE(resp.body.find("fly-io"),  std::string::npos);
}

TEST_F(RegistryHandlerTest, SearchByPrefix) {
    seed_package("fly-std",  "1.0.0");
    seed_package("fly-io",   "1.0.0");
    seed_package("other-pkg","1.0.0");
    auto resp = handler_->handle(make_get("/v1/search", "q=fly-"));
    EXPECT_EQ(resp.status, 200);
    EXPECT_NE(resp.body.find("fly-std"), std::string::npos);
    EXPECT_NE(resp.body.find("fly-io"),  std::string::npos);
    EXPECT_EQ(resp.body.find("other-pkg"), std::string::npos);
}

TEST_F(RegistryHandlerTest, SearchNoMatch) {
    seed_package("fly-std", "1.0.0");
    auto resp = handler_->handle(make_get("/v1/search", "q=zzz"));
    EXPECT_EQ(resp.status, 200);
    EXPECT_EQ(resp.body,   "[]");
}

TEST_F(RegistryHandlerTest, SearchUrlDecoded) {
    seed_package("my lib", "1.0.0"); // space in name (unusual but valid dir)
    auto resp = handler_->handle(make_get("/v1/search", "q=my%20lib"));
    EXPECT_EQ(resp.status, 200);
    EXPECT_NE(resp.body.find("my lib"), std::string::npos);
}

TEST_F(RegistryHandlerTest, SearchSortedAlphabetically) {
    seed_package("zzz-pkg", "1.0.0");
    seed_package("aaa-pkg", "1.0.0");
    seed_package("mmm-pkg", "1.0.0");
    auto resp = handler_->handle(make_get("/v1/search", "q="));
    auto aaa  = resp.body.find("aaa-pkg");
    auto mmm  = resp.body.find("mmm-pkg");
    auto zzz  = resp.body.find("zzz-pkg");
    EXPECT_LT(aaa, mmm);
    EXPECT_LT(mmm, zzz);
}

// ── Response format ───────────────────────────────────────────────────────────

TEST_F(RegistryHandlerTest, Response200Format) {
    seed_package("mylib", "1.0.0");
    auto resp = handler_->handle(make_get("/v1/mylib"));
    auto http  = resp.to_http();
    EXPECT_NE(http.find("HTTP/1.1 200 OK"),  std::string::npos);
    EXPECT_NE(http.find("Content-Type: application/json"), std::string::npos);
    EXPECT_NE(http.find("Content-Length:"),  std::string::npos);
    EXPECT_NE(http.find("Connection: close"), std::string::npos);
}

TEST_F(RegistryHandlerTest, Response404Format) {
    auto resp = handler_->handle(make_get("/v1/no-such-pkg"));
    auto http  = resp.to_http();
    EXPECT_NE(http.find("HTTP/1.1 404 Not Found"), std::string::npos);
}

TEST_F(RegistryHandlerTest, Response201Format) {
    auto resp = handler_->handle(make_post("/v1/pkg/1.0.0", "data"));
    auto http  = resp.to_http();
    EXPECT_NE(http.find("HTTP/1.1 201 Created"), std::string::npos);
}

// ── Routing edge cases ────────────────────────────────────────────────────────

TEST_F(RegistryHandlerTest, UnknownPath) {
    auto resp = handler_->handle(make_get("/unknown"));
    EXPECT_EQ(resp.status, 404);
}

TEST_F(RegistryHandlerTest, UnknownAction) {
    auto resp = handler_->handle(make_get("/v1/mylib/1.0.0/badaction"));
    EXPECT_EQ(resp.status, 404);
}

TEST_F(RegistryHandlerTest, PostWithoutVersion) {
    // POST /v1/{name} (no version) — not a valid endpoint
    auto resp = handler_->handle(make_post("/v1/mylib", "data"));
    EXPECT_EQ(resp.status, 400);
}

TEST_F(RegistryHandlerTest, GetRootPath) {
    auto resp = handler_->handle(make_get("/"));
    EXPECT_EQ(resp.status, 404);
}

// ── Package isolation ─────────────────────────────────────────────────────────

TEST_F(RegistryHandlerTest, DifferentPackagesIsolated) {
    seed_package("pkg-a", "1.0.0", "[package]\nname=\"pkg-a\"\nversion=\"1.0.0\"\n");
    seed_package("pkg-b", "1.0.0", "[package]\nname=\"pkg-b\"\nversion=\"1.0.0\"\n");
    auto ra = handler_->handle(make_get("/v1/pkg-a/1.0.0"));
    auto rb = handler_->handle(make_get("/v1/pkg-b/1.0.0"));
    EXPECT_EQ(ra.status, 200);
    EXPECT_EQ(rb.status, 200);
    EXPECT_NE(ra.body, rb.body);
    EXPECT_NE(ra.body.find("pkg-a"), std::string::npos);
    EXPECT_NE(rb.body.find("pkg-b"), std::string::npos);
}

TEST_F(RegistryHandlerTest, VersionsOfDifferentPackagesDoNotMix) {
    seed_package("pkg-a", "1.0.0");
    // pkg-b version 1.0.0 does not exist
    auto resp = handler_->handle(make_get("/v1/pkg-b/1.0.0"));
    EXPECT_EQ(resp.status, 404);
}

// ── Concurrent publish then read ──────────────────────────────────────────────

TEST_F(RegistryHandlerTest, PublishMultipleVersionsThenListAll) {
    for (int i = 1; i <= 5; ++i) {
        std::string ver = std::to_string(i) + ".0.0";
        handler_->handle(make_post("/v1/mylib/" + ver, "TARBALL_" + ver));
    }
    auto resp = handler_->handle(make_get("/v1/mylib"));
    EXPECT_EQ(resp.status, 200);
    for (int i = 1; i <= 5; ++i)
        EXPECT_NE(resp.body.find(std::to_string(i) + ".0.0"), std::string::npos);
}

// ── Authentication ────────────────────────────────────────────────────────────

class RegistryHandlerAuthTest : public ::testing::Test {
protected:
    void SetUp() override {
        static int counter = 0;
        storage_ = std::filesystem::temp_directory_path()
                 / ("reg_auth_test_" + std::to_string(++counter));
        std::filesystem::create_directories(storage_);
        handler_ = std::make_unique<RegistryHandler>(storage_, "secret-token");
    }

    void TearDown() override { std::filesystem::remove_all(storage_); }

    Request make_post_auth(const std::string& path, const std::string& body,
                           const std::string& token) const {
        Request r{"POST", path, "", {}, body};
        if (!token.empty()) r.headers["Authorization"] = "Bearer " + token;
        return r;
    }

    Request make_get(const std::string& path) const {
        return {"GET", path, "", {}, ""};
    }

    void seed(const std::string& name, const std::string& version) {
        auto dir = storage_ / name;
        std::filesystem::create_directories(dir / version);
        std::ofstream f1(dir / (version + ".tar.gz"));  f1 << "TB";
        std::ofstream f2(dir / version / "fly.toml");    f2 << "[package]\nname=\"" << name << "\"\n";
    }

    std::filesystem::path storage_;
    std::unique_ptr<RegistryHandler> handler_;
};

TEST_F(RegistryHandlerAuthTest, RequiresAuthIsTrue) {
    EXPECT_TRUE(handler_->requires_auth());
}

TEST_F(RegistryHandlerAuthTest, PublishWithCorrectToken) {
    auto resp = handler_->handle(make_post_auth("/v1/pkg/1.0.0", "DATA", "secret-token"));
    EXPECT_EQ(resp.status, 201);
}

TEST_F(RegistryHandlerAuthTest, PublishWithoutTokenReturns401) {
    Request r{"POST", "/v1/pkg/1.0.0", "", {}, "DATA"};
    auto resp = handler_->handle(r);
    EXPECT_EQ(resp.status, 401);
}

TEST_F(RegistryHandlerAuthTest, PublishWithWrongTokenReturns401) {
    auto resp = handler_->handle(make_post_auth("/v1/pkg/1.0.0", "DATA", "wrong-token"));
    EXPECT_EQ(resp.status, 401);
}

TEST_F(RegistryHandlerAuthTest, PublishWithEmptyTokenReturns401) {
    auto resp = handler_->handle(make_post_auth("/v1/pkg/1.0.0", "DATA", ""));
    EXPECT_EQ(resp.status, 401);
}

TEST_F(RegistryHandlerAuthTest, PublishWithMalformedHeaderReturns401) {
    Request r{"POST", "/v1/pkg/1.0.0", "", {{"Authorization", "secret-token"}}, "DATA"};
    auto resp = handler_->handle(r);
    EXPECT_EQ(resp.status, 401); // missing "Bearer " prefix
}

TEST_F(RegistryHandlerAuthTest, GetListVersionsPublicNoToken) {
    seed("mylib", "1.0.0");
    auto resp = handler_->handle(make_get("/v1/mylib"));
    EXPECT_EQ(resp.status, 200); // GET is always public
}

TEST_F(RegistryHandlerAuthTest, GetMetadataPublicNoToken) {
    seed("mylib", "1.0.0");
    auto resp = handler_->handle(make_get("/v1/mylib/1.0.0"));
    EXPECT_EQ(resp.status, 200);
}

TEST_F(RegistryHandlerAuthTest, GetDownloadPublicNoToken) {
    seed("mylib", "1.0.0");
    auto resp = handler_->handle(make_get("/v1/mylib/1.0.0/download"));
    EXPECT_EQ(resp.status, 200);
}

TEST_F(RegistryHandlerAuthTest, SearchPublicNoToken) {
    seed("mylib", "1.0.0");
    auto resp = handler_->handle({"GET", "/v1/search", "q=", {}, ""});
    EXPECT_EQ(resp.status, 200);
}

TEST_F(RegistryHandlerAuthTest, NoAuthRequired_WhenTokenEmpty) {
    // Handler with no token: all POSTs pass without Authorization header.
    RegistryHandler open(storage_, "");
    EXPECT_FALSE(open.requires_auth());
    Request r{"POST", "/v1/pkg/1.0.0", "", {}, "DATA"};
    auto resp = open.handle(r);
    EXPECT_EQ(resp.status, 201);
}

TEST_F(RegistryHandlerAuthTest, Response401ContainsHint) {
    Request r{"POST", "/v1/pkg/1.0.0", "", {}, "DATA"};
    auto resp = handler_->handle(r);
    EXPECT_NE(resp.body.find("Bearer"), std::string::npos);
}

} // namespace flyp::registry::test
