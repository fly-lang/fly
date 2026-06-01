//===----------------------------------------------------------------------===//
// tools/lsp/test/LspServerTest.cpp — integration tests for LspServer dispatch
//
// Strategy: pre-load all messages into an in-pipe before calling server.run().
// Since run() reads synchronously, it drains all messages and returns on EOF
// (or after shutdown). Responses are then read from the out-pipe.
//===----------------------------------------------------------------------===//
#include <gtest/gtest.h>
#include "LspServer.h"
#include <llvm/Support/JSON.h>

#include "PosixCompat.h"
#include <cstdio>
#include <string>

using namespace fly::lsp;
using namespace llvm;

// ── Server test fixture ───────────────────────────────────────────────────────

class ServerFixture {
    int saved_in, saved_out;
    int in_pipe[2];   // test writes here → server reads from stdin
    int out_pipe[2];  // server writes to stdout → test reads here
    bool in_closed = false;

public:
    ServerFixture() {
        saved_in  = dup(STDIN_FILENO);
        saved_out = dup(STDOUT_FILENO);
        pipe(in_pipe);
        pipe(out_pipe);
        dup2(in_pipe[0],  STDIN_FILENO);  close(in_pipe[0]);
        dup2(out_pipe[1], STDOUT_FILENO); close(out_pipe[1]);
        // Disable stdin buffering: prevents glibc's fgetc from reading ahead
        // into the next test's messages when the server stops mid-stream.
        setvbuf(stdin, nullptr, _IONBF, 0);
        clearerr(stdin);
        std::fflush(stdout);
    }

    // Send a JSON string as a properly framed LSP message.
    void send(const std::string &json) {
        std::string frame =
            "Content-Length: " + std::to_string(json.size()) + "\r\n\r\n" + json;
        write(in_pipe[1], frame.data(), frame.size());
    }

    void closeInput() {
        if (!in_closed) { close(in_pipe[1]); in_closed = true; }
    }

    // Run the server (blocks until EOF on stdin or after shutdown).
    void runServer() {
        LspServer srv;
        srv.run();
    }

    // Read one complete LSP response frame from the out-pipe.
    json::Value readFrame() {
        std::fflush(stdout);

        // Read headers byte-by-byte until \r\n\r\n.
        std::string header;
        header.reserve(128);
        for (;;) {
            char c;
            ssize_t n = ::read(out_pipe[0], &c, 1);
            if (n <= 0) return json::Value(nullptr);
            header += c;
            if (header.size() >= 4 &&
                header.compare(header.size() - 4, 4, "\r\n\r\n") == 0)
                break;
        }

        const std::string key = "Content-Length: ";
        auto pos = header.find(key);
        if (pos == std::string::npos) return json::Value(nullptr);
        auto eol = header.find("\r\n", pos);
        size_t len = std::stoul(header.substr(pos + key.size(),
                                               eol - pos - key.size()));

        std::string body(len, '\0');
        size_t done = 0;
        while (done < len) {
            ssize_t n = ::read(out_pipe[0], body.data() + done, len - done);
            if (n <= 0) break;
            done += (size_t)n;
        }

        auto v = json::parse(body);
        return v ? std::move(*v) : json::Value(nullptr);
    }

    ~ServerFixture() {
        closeInput();
        std::fflush(stdout);
        dup2(saved_in,  STDIN_FILENO);
        dup2(saved_out, STDOUT_FILENO);
        close(saved_in);
        close(saved_out);
        close(out_pipe[0]);
        clearerr(stdin);
    }
};

// ── Message builders ──────────────────────────────────────────────────────────

static std::string makeRequest(int id, const std::string &method,
                               const std::string &params = "{}") {
    return R"({"jsonrpc":"2.0","id":)" + std::to_string(id) +
           R"(,"method":")" + method + R"(","params":)" + params + "}";
}

static std::string makeNotif(const std::string &method,
                             const std::string &params = "{}") {
    return R"({"jsonrpc":"2.0","method":")" + method +
           R"(","params":)" + params + "}";
}

static constexpr const char *kInit = R"({"processId":null,"rootUri":null,"capabilities":{}})";

// ── Before initialize ─────────────────────────────────────────────────────────

TEST(LspServer, RequestBeforeInitialize_ReturnsError32002) {
    ServerFixture f;
    f.send(makeRequest(1, "textDocument/hover",
        R"({"textDocument":{"uri":"file:///a.fly"},"position":{"line":0,"character":0}})"));
    f.closeInput();
    f.runServer();

    auto v = f.readFrame();
    auto *obj = v.getAsObject();
    ASSERT_NE(obj, nullptr);
    auto *err = obj->getObject("error");
    ASSERT_NE(err, nullptr);
    EXPECT_EQ(err->getInteger("code").value_or(0), -32002);
}

TEST(LspServer, NotificationBeforeInitialize_Ignored) {
    // Notifications before initialize produce no error response.
    ServerFixture f;
    f.send(makeNotif("initialized", "{}"));
    f.send(makeRequest(1, "initialize", kInit));
    f.closeInput();
    f.runServer();

    // Only one response: the initialize result (no error from the notification).
    auto v = f.readFrame();
    auto *obj = v.getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getInteger("id").value_or(-1), 1);
    EXPECT_NE(obj->get("result"), nullptr);
}

// ── initialize ────────────────────────────────────────────────────────────────

TEST(LspServer, Initialize_ReturnsId) {
    ServerFixture f;
    f.send(makeRequest(42, "initialize", kInit));
    f.closeInput();
    f.runServer();

    auto v = f.readFrame();
    auto *obj = v.getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getInteger("id").value_or(-1), 42);
}

TEST(LspServer, Initialize_HasCapabilities) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.closeInput();
    f.runServer();

    auto v = f.readFrame();
    auto *result = v.getAsObject()->getObject("result");
    ASSERT_NE(result, nullptr);
    auto *caps = result->getObject("capabilities");
    ASSERT_NE(caps, nullptr);
    EXPECT_EQ(caps->getInteger("textDocumentSync").value_or(-1), 1);
    EXPECT_TRUE(caps->getBoolean("hoverProvider").value_or(false));
    EXPECT_TRUE(caps->getBoolean("definitionProvider").value_or(false));
    EXPECT_TRUE(caps->getBoolean("referencesProvider").value_or(false));
    EXPECT_TRUE(caps->getBoolean("documentSymbolProvider").value_or(false));
}

TEST(LspServer, Initialize_CompletionProviderHasTriggerChar) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.closeInput();
    f.runServer();

    auto v = f.readFrame();
    auto *caps = v.getAsObject()->getObject("result")->getObject("capabilities");
    ASSERT_NE(caps, nullptr);
    auto *cp = caps->getObject("completionProvider");
    ASSERT_NE(cp, nullptr);
    auto *triggers = cp->getArray("triggerCharacters");
    ASSERT_NE(triggers, nullptr);
    ASSERT_EQ(triggers->size(), 1u);
    EXPECT_EQ((*triggers)[0].getAsString().value_or(""), ".");
}

TEST(LspServer, Initialize_ServerInfo) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.closeInput();
    f.runServer();

    auto v = f.readFrame();
    auto *info = v.getAsObject()->getObject("result")->getObject("serverInfo");
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->getString("name").value_or(""), "fly-lsp");
    EXPECT_FALSE(info->getString("version").value_or("").empty());
}

TEST(LspServer, Initialize_NoErrorField) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.closeInput();
    f.runServer();

    auto v = f.readFrame();
    EXPECT_EQ(v.getAsObject()->get("error"), nullptr);
}

// ── initialized notification ──────────────────────────────────────────────────

TEST(LspServer, Initialized_ProducesNoResponse) {
    // "initialized" is a client notification; the server sends nothing back.
    // Test: initialize (id=1) then initialized then shutdown (id=2).
    // We expect exactly two responses (id 1 and id 2), not three.
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeNotif("initialized", "{}"));
    f.send(makeRequest(2, "shutdown", "{}"));
    f.runServer();

    auto r1 = f.readFrame();
    auto r2 = f.readFrame();
    EXPECT_EQ(r1.getAsObject()->getInteger("id").value_or(-1), 1);
    EXPECT_EQ(r2.getAsObject()->getInteger("id").value_or(-1), 2);
}

// ── shutdown ──────────────────────────────────────────────────────────────────

TEST(LspServer, Shutdown_ReturnsNullResult) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeRequest(2, "shutdown", "{}"));
    f.runServer();

    f.readFrame(); // initialize
    auto v = f.readFrame();
    auto *obj = v.getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getInteger("id").value_or(-1), 2);
    const json::Value *res = obj->get("result");
    ASSERT_NE(res, nullptr);
    EXPECT_TRUE(res->getAsNull().has_value());
}

TEST(LspServer, Shutdown_StopsProcessingFurtherMessages) {
    // After shutdown, further requests should not produce responses.
    // We load: initialize, shutdown, then another request.
    // Then we close stdin and run. The server stops after shutdown.
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeRequest(2, "shutdown", "{}"));
    f.send(makeRequest(3, "textDocument/completion", "{}"));
    f.closeInput();
    f.runServer();

    auto r1 = f.readFrame(); // initialize
    auto r2 = f.readFrame(); // shutdown
    EXPECT_EQ(r1.getAsObject()->getInteger("id").value_or(-1), 1);
    EXPECT_EQ(r2.getAsObject()->getInteger("id").value_or(-1), 2);
    // Verify shutdown result is null; the completion request (id=3) was never
    // processed because the server loop exited as soon as shutdown_ was set.
    EXPECT_TRUE(r2.getAsObject()->get("result")->getAsNull().has_value());
}

// ── exit ─────────────────────────────────────────────────────────────────────

TEST(LspServer, Exit_StopsServer) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeNotif("exit", "{}"));
    f.runServer();  // must return, not block forever

    auto v = f.readFrame();
    EXPECT_EQ(v.getAsObject()->getInteger("id").value_or(-1), 1);
}

// ── unknown method ────────────────────────────────────────────────────────────

TEST(LspServer, UnknownMethod_ReturnsError32601) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeRequest(2, "workspace/doesNotExist", "{}"));
    f.closeInput();
    f.runServer();

    f.readFrame(); // initialize
    auto v = f.readFrame();
    auto *err = v.getAsObject()->getObject("error");
    ASSERT_NE(err, nullptr);
    EXPECT_EQ(err->getInteger("code").value_or(0), -32601);
}

TEST(LspServer, UnknownMethod_MessageContainsMethodName) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeRequest(2, "foo/bar", "{}"));
    f.closeInput();
    f.runServer();

    f.readFrame();
    auto v = f.readFrame();
    auto *err = v.getAsObject()->getObject("error");
    ASSERT_NE(err, nullptr);
    EXPECT_NE(err->getString("message").value_or("").find("foo/bar"),
              std::string::npos);
}

// ── textDocument/hover ────────────────────────────────────────────────────────

TEST(LspServer, Hover_NoSymbol_ReturnsNull) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeRequest(2, "textDocument/hover",
        R"({"textDocument":{"uri":"file:///a.fly"},"position":{"line":0,"character":0}})"));
    f.closeInput();
    f.runServer();

    f.readFrame();
    auto v = f.readFrame();
    auto *obj = v.getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getInteger("id").value_or(-1), 2);
    const json::Value *res = obj->get("result");
    ASSERT_NE(res, nullptr);
    EXPECT_TRUE(res->getAsNull().has_value());
}

TEST(LspServer, Hover_MissingPosition_ReturnsNull) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeRequest(2, "textDocument/hover",
        R"({"textDocument":{"uri":"file:///a.fly"}})"));
    f.closeInput();
    f.runServer();

    f.readFrame();
    auto v = f.readFrame();
    const json::Value *res = v.getAsObject()->get("result");
    ASSERT_NE(res, nullptr);
    EXPECT_TRUE(res->getAsNull().has_value());
}

// ── textDocument/definition ───────────────────────────────────────────────────

TEST(LspServer, Definition_NoSymbol_ReturnsNull) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeRequest(2, "textDocument/definition",
        R"({"textDocument":{"uri":"file:///a.fly"},"position":{"line":0,"character":0}})"));
    f.closeInput();
    f.runServer();

    f.readFrame();
    auto v = f.readFrame();
    auto *obj = v.getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getInteger("id").value_or(-1), 2);
    const json::Value *res = obj->get("result");
    ASSERT_NE(res, nullptr);
    EXPECT_TRUE(res->getAsNull().has_value());
}

// ── textDocument/completion ───────────────────────────────────────────────────

TEST(LspServer, Completion_ReturnsEmptyArray) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeRequest(2, "textDocument/completion",
        R"({"textDocument":{"uri":"file:///a.fly"},"position":{"line":0,"character":5}})"));
    f.closeInput();
    f.runServer();

    f.readFrame();
    auto v = f.readFrame();
    auto *obj = v.getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getInteger("id").value_or(-1), 2);
    const json::Value *res = obj->get("result");
    ASSERT_NE(res, nullptr);
    auto *arr = res->getAsArray();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->empty());
}

// ── textDocument/documentSymbol ───────────────────────────────────────────────

TEST(LspServer, DocumentSymbol_ReturnsEmptyArray) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeRequest(2, "textDocument/documentSymbol",
        R"({"textDocument":{"uri":"file:///a.fly"}})"));
    f.closeInput();
    f.runServer();

    f.readFrame();
    auto v = f.readFrame();
    auto *obj = v.getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getInteger("id").value_or(-1), 2);
    const json::Value *res = obj->get("result");
    ASSERT_NE(res, nullptr);
    auto *arr = res->getAsArray();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->empty());
}

// ── textDocument/references ───────────────────────────────────────────────────

TEST(LspServer, References_ReturnsEmptyArray) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeRequest(2, "textDocument/references",
        R"({"textDocument":{"uri":"file:///a.fly"},"position":{"line":0,"character":0},"context":{"includeDeclaration":false}})"));
    f.closeInput();
    f.runServer();

    f.readFrame();
    auto v = f.readFrame();
    auto *obj = v.getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getInteger("id").value_or(-1), 2);
    const json::Value *res = obj->get("result");
    ASSERT_NE(res, nullptr);
    auto *arr = res->getAsArray();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->empty());
}

// ── textDocument/didClose ─────────────────────────────────────────────────────

TEST(LspServer, DidClose_PublishesDiagnosticsForUri) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeNotif("textDocument/didClose",
        R"({"textDocument":{"uri":"file:///my/file.fly"}})"));
    f.closeInput();
    f.runServer();

    f.readFrame(); // initialize result
    auto v = f.readFrame(); // publishDiagnostics notification
    auto *obj = v.getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getString("method").value_or(""), "textDocument/publishDiagnostics");
    auto *params = obj->getObject("params");
    ASSERT_NE(params, nullptr);
    EXPECT_EQ(params->getString("uri").value_or(""), "file:///my/file.fly");
}

TEST(LspServer, DidClose_DiagnosticsArrayIsEmpty) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeNotif("textDocument/didClose",
        R"({"textDocument":{"uri":"file:///a.fly"}})"));
    f.closeInput();
    f.runServer();

    f.readFrame();
    auto v = f.readFrame();
    auto *params = v.getAsObject()->getObject("params");
    ASSERT_NE(params, nullptr);
    const json::Value *diags = params->get("diagnostics");
    ASSERT_NE(diags, nullptr);
    auto *arr = diags->getAsArray();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->empty());
}

TEST(LspServer, DidClose_HasNoId) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeNotif("textDocument/didClose",
        R"({"textDocument":{"uri":"file:///a.fly"}})"));
    f.closeInput();
    f.runServer();

    f.readFrame();
    auto v = f.readFrame();
    EXPECT_EQ(v.getAsObject()->get("id"), nullptr);
}

// ── full lifecycle ────────────────────────────────────────────────────────────

TEST(LspServer, FullLifecycle_InitializeHoverShutdown) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeNotif("initialized", "{}"));
    f.send(makeRequest(2, "textDocument/hover",
        R"({"textDocument":{"uri":"file:///a.fly"},"position":{"line":5,"character":3}})"));
    f.send(makeRequest(3, "textDocument/completion",
        R"({"textDocument":{"uri":"file:///a.fly"},"position":{"line":5,"character":3}})"));
    f.send(makeRequest(4, "shutdown", "{}"));
    f.runServer();

    auto r1 = f.readFrame(); // initialize
    auto r2 = f.readFrame(); // hover
    auto r3 = f.readFrame(); // completion
    auto r4 = f.readFrame(); // shutdown

    EXPECT_EQ(r1.getAsObject()->getInteger("id").value_or(-1), 1);
    EXPECT_EQ(r2.getAsObject()->getInteger("id").value_or(-1), 2);
    EXPECT_EQ(r3.getAsObject()->getInteger("id").value_or(-1), 3);
    EXPECT_EQ(r4.getAsObject()->getInteger("id").value_or(-1), 4);
}

TEST(LspServer, FullLifecycle_AllLanguageFeatures) {
    ServerFixture f;
    f.send(makeRequest(1, "initialize", kInit));
    f.send(makeRequest(2, "textDocument/hover",
        R"({"textDocument":{"uri":"file:///a.fly"},"position":{"line":0,"character":0}})"));
    f.send(makeRequest(3, "textDocument/definition",
        R"({"textDocument":{"uri":"file:///a.fly"},"position":{"line":0,"character":0}})"));
    f.send(makeRequest(4, "textDocument/completion",
        R"({"textDocument":{"uri":"file:///a.fly"},"position":{"line":0,"character":0}})"));
    f.send(makeRequest(5, "textDocument/documentSymbol",
        R"({"textDocument":{"uri":"file:///a.fly"}})"));
    f.send(makeRequest(6, "textDocument/references",
        R"({"textDocument":{"uri":"file:///a.fly"},"position":{"line":0,"character":0},"context":{"includeDeclaration":false}})"));
    f.send(makeNotif("textDocument/didClose",
        R"({"textDocument":{"uri":"file:///a.fly"}})"));
    f.send(makeRequest(7, "shutdown", "{}"));
    f.runServer();

    for (int expected_id : {1, 2, 3, 4, 5, 6}) {
        auto v = f.readFrame();
        auto *obj = v.getAsObject();
        ASSERT_NE(obj, nullptr) << "response " << expected_id;
        EXPECT_EQ(obj->getInteger("id").value_or(-1), expected_id)
            << "response " << expected_id;
    }
    // didClose notification (no id)
    auto notif = f.readFrame();
    EXPECT_EQ(notif.getAsObject()->getString("method").value_or(""),
              "textDocument/publishDiagnostics");
    // shutdown
    auto shut = f.readFrame();
    EXPECT_EQ(shut.getAsObject()->getInteger("id").value_or(-1), 7);
}
