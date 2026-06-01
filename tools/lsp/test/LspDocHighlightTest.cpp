//===----------------------------------------------------------------------===//
// tools/lsp/test/LspDocHighlightTest.cpp — unit tests for document highlights
//
// Scope of unit tests
// ───────────────────
// Walking the compiled AST post-Execute() is unsafe in the test harness
// because the Fly Frontend frees internal AST nodes when Execute() returns.
// In the production fly-lsp binary this is not an issue because the server
// stays alive in an event loop and the LspAnalyzer owns the Driver/Frontend
// for the entire server lifetime.
//
// Unit tests therefore cover:
//   1. LspProtocol: toJson(LspDocumentHighlight) structure and values.
//   2. LspAnalyzer: getDocumentHighlights() returns empty before any compile.
//   3. LspServer:   textDocument/documentHighlight is dispatched and returns
//                   an empty JSON array (no compile → no AST → no symbols).
//
// End-to-end highlight correctness (symbol found, call sites returned, …) is
// validated manually via VS Code + the fly-lsp binary.
//===----------------------------------------------------------------------===//
#include <gtest/gtest.h>
#include "LspAnalyzer.h"
#include "LspProtocol.h"
#include "LspServer.h"

#include <fcntl.h>
#include <unistd.h>

using namespace fly::lsp;
using namespace llvm;

// ── 1. Protocol: toJson(LspDocumentHighlight) ────────────────────────────────

TEST(LspDocumentHighlight, ToJson_ContainsRangeAndKind) {
    LspDocumentHighlight h;
    h.range = {{1, 4}, {1, 10}};
    h.kind  = 2;
    auto obj = toJson(h);
    EXPECT_NE(obj.get("range"), nullptr) << "toJson must produce a 'range' key";
    EXPECT_NE(obj.get("kind"),  nullptr) << "toJson must produce a 'kind' key";
}

TEST(LspDocumentHighlight, ToJson_KindValueMatchesInput) {
    LspDocumentHighlight h;
    h.range = {{0, 0}, {0, 6}};
    h.kind  = 3;
    auto obj = toJson(h);
    EXPECT_EQ(obj.getInteger("kind").value_or(-1), 3);
}

TEST(LspDocumentHighlight, ToJson_DefaultKindIsOne) {
    LspDocumentHighlight h;
    h.range = {{0, 0}, {0, 1}};
    // kind not set → default 1
    auto obj = toJson(h);
    EXPECT_EQ(obj.getInteger("kind").value_or(-1), 1);
}

TEST(LspDocumentHighlight, ToJson_RangeIsNestedObject) {
    LspDocumentHighlight h;
    h.range = {{2, 4}, {2, 10}};
    auto obj = toJson(h);
    auto *range = obj.getObject("range");
    ASSERT_NE(range, nullptr);
    EXPECT_NE(range->getObject("start"), nullptr);
    EXPECT_NE(range->getObject("end"),   nullptr);
}

TEST(LspDocumentHighlight, ToJson_StartPositionValues) {
    LspDocumentHighlight h;
    h.range = {{7, 3}, {7, 9}};
    auto obj    = toJson(h);
    auto *range = obj.getObject("range");
    ASSERT_NE(range, nullptr);
    auto *start = range->getObject("start");
    ASSERT_NE(start, nullptr);
    EXPECT_EQ(start->getInteger("line").value_or(-1),      7);
    EXPECT_EQ(start->getInteger("character").value_or(-1), 3);
}

TEST(LspDocumentHighlight, ToJson_EndPositionValues) {
    LspDocumentHighlight h;
    h.range = {{7, 3}, {7, 9}};
    auto obj  = toJson(h);
    auto *end = obj.getObject("range")->getObject("end");
    ASSERT_NE(end, nullptr);
    EXPECT_EQ(end->getInteger("line").value_or(-1),      7);
    EXPECT_EQ(end->getInteger("character").value_or(-1), 9);
}

// ── 2. Analyzer: safe pre-compile behaviour ───────────────────────────────────

TEST(LspDocumentHighlight, Analyzer_BeforeCompile_ReturnsEmpty) {
    LspAnalyzer a;
    EXPECT_TRUE(a.getDocumentHighlights("/nonexistent.fly", 0, 0).empty());
}

// ── 3. Server: dispatch + empty-array response ───────────────────────────────

// Minimal stdio redirect helpers (same pattern as LspServerTest.cpp)
struct PipeIO {
    int saved_in, saved_out, in_w, out_r;
    bool in_closed = false;

    PipeIO() {
        saved_in  = dup(STDIN_FILENO);
        saved_out = dup(STDOUT_FILENO);
        int ip[2], op[2];
        pipe(ip); pipe(op);
        dup2(ip[0], STDIN_FILENO); close(ip[0]);
        dup2(op[1], STDOUT_FILENO); close(op[1]);
        setvbuf(stdin, nullptr, _IONBF, 0);
        clearerr(stdin); fflush(stdout);
        in_w = ip[1]; out_r = op[0];
    }

    void send(const std::string &json) {
        std::string f = "Content-Length: " + std::to_string(json.size()) + "\r\n\r\n" + json;
        write(in_w, f.data(), f.size());
    }

    void close_in() { if (!in_closed) { close(in_w); in_closed = true; } }

    json::Value readFrame() {
        fflush(stdout);
        std::string hdr; char c;
        while (true) {
            if (::read(out_r, &c, 1) <= 0) return json::Value(nullptr);
            hdr += c;
            if (hdr.size() >= 4 && hdr.compare(hdr.size()-4, 4, "\r\n\r\n") == 0) break;
        }
        auto pos = hdr.find("Content-Length: ");
        if (pos == std::string::npos) return json::Value(nullptr);
        auto eol = hdr.find("\r\n", pos);
        size_t len = std::stoul(hdr.substr(pos + 16, eol - pos - 16));
        std::string body(len, '\0');
        size_t done = 0;
        while (done < len) { auto n = ::read(out_r, body.data()+done, len-done); if (n<=0) break; done+=n; }
        auto v = json::parse(body);
        return v ? std::move(*v) : json::Value(nullptr);
    }

    ~PipeIO() {
        close_in(); fflush(stdout);
        dup2(saved_in, STDIN_FILENO); dup2(saved_out, STDOUT_FILENO);
        close(saved_in); close(saved_out); close(out_r); clearerr(stdin);
    }
};

TEST(LspDocumentHighlight, Server_DocumentHighlight_ReturnsEmptyArray) {
    PipeIO io;
    io.send(R"({"jsonrpc":"2.0","id":1,"method":"initialize","params":{}})");
    io.send(R"({"jsonrpc":"2.0","id":2,"method":"textDocument/documentHighlight","params":{"textDocument":{"uri":"file:///a.fly"},"position":{"line":0,"character":0}}})");
    io.close_in();

    LspServer srv; srv.run();

    io.readFrame();   // initialize response
    auto v = io.readFrame();
    auto *obj = v.getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getInteger("id").value_or(-1), 2);
    const json::Value *res = obj->get("result");
    ASSERT_NE(res, nullptr);
    auto *arr = res->getAsArray();
    ASSERT_NE(arr, nullptr) << "result must be a JSON array";
    EXPECT_TRUE(arr->empty()) << "No compile → no symbols → empty highlight array";
}

TEST(LspDocumentHighlight, Server_Initialize_AdvertisesDocumentHighlightProvider) {
    PipeIO io;
    io.send(R"({"jsonrpc":"2.0","id":1,"method":"initialize","params":{}})");
    io.close_in();

    LspServer srv; srv.run();

    auto v    = io.readFrame();
    auto *res = v.getAsObject()->getObject("result");
    ASSERT_NE(res, nullptr);
    auto *caps = res->getObject("capabilities");
    ASSERT_NE(caps, nullptr);
    auto dh = caps->getBoolean("documentHighlightProvider");
    EXPECT_TRUE(dh.has_value()) << "capabilities must include documentHighlightProvider";
    EXPECT_TRUE(dh.value_or(false));
}
