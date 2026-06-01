//===----------------------------------------------------------------------===//
// tools/lsp/test/LspTransportTest.cpp — unit tests for LspTransport
//
// Each test redirects fd 0 (stdin) / fd 1 (stdout) to pipes so the transport
// can be exercised without touching the real terminal.
//===----------------------------------------------------------------------===//
#include <gtest/gtest.h>
#include "LspTransport.h"
#include <llvm/Support/JSON.h>

#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>

using namespace fly::lsp;
using namespace llvm;

// ── IO redirect helper ────────────────────────────────────────────────────────

struct StdioCapture {
    int saved_in, saved_out;
    int in_pipe[2];   // [0]=read end (becomes stdin), [1]=write end (test writes here)
    int out_pipe[2];  // [0]=read end (test reads here), [1]=write end (becomes stdout)
    bool in_closed = false;

    StdioCapture() {
        saved_in  = dup(STDIN_FILENO);
        saved_out = dup(STDOUT_FILENO);
        pipe(in_pipe);
        pipe(out_pipe);
        dup2(in_pipe[0],  STDIN_FILENO);  close(in_pipe[0]);
        dup2(out_pipe[1], STDOUT_FILENO); close(out_pipe[1]);
        // Disable stdin buffering so fgetc reads one byte at a time from the
        // pipe; this prevents stale buffered bytes from leaking into the next
        // test when the server stops reading mid-stream.
        setvbuf(stdin, nullptr, _IONBF, 0);
        clearerr(stdin);
        std::fflush(stdout);
    }

    // Feed raw bytes into the fake stdin.
    void feedStdin(const std::string &data) {
        write(in_pipe[1], data.data(), data.size());
    }

    // Feed a properly framed LSP message into stdin.
    void feedMessage(const std::string &json) {
        std::string frame =
            "Content-Length: " + std::to_string(json.size()) + "\r\n\r\n" + json;
        feedStdin(frame);
    }

    void closeStdin() {
        if (!in_closed) { close(in_pipe[1]); in_closed = true; }
    }

    // Drain all bytes currently in the stdout pipe (non-blocking after fflush).
    std::string captureStdout() {
        std::fflush(stdout);
        int fl = fcntl(out_pipe[0], F_GETFL, 0);
        fcntl(out_pipe[0], F_SETFL, fl | O_NONBLOCK);
        std::string result;
        char buf[4096];
        ssize_t n;
        while ((n = ::read(out_pipe[0], buf, sizeof(buf))) > 0)
            result.append(buf, (size_t)n);
        fcntl(out_pipe[0], F_SETFL, fl);
        return result;
    }

    ~StdioCapture() {
        closeStdin();
        std::fflush(stdout);
        dup2(saved_in,  STDIN_FILENO);
        dup2(saved_out, STDOUT_FILENO);
        close(saved_in);
        close(saved_out);
        close(out_pipe[0]);
        clearerr(stdin);
    }
};

// Parse the JSON body from the first LSP frame in a raw byte string.
static std::string extractFirstBody(const std::string &raw) {
    const std::string key = "Content-Length: ";
    auto pos = raw.find(key);
    if (pos == std::string::npos) return {};
    auto sep = raw.find("\r\n\r\n", pos);
    if (sep == std::string::npos) return {};
    size_t len = std::stoul(raw.substr(pos + key.size(), sep - pos - key.size()));
    size_t start = sep + 4;
    if (start + len > raw.size()) return {};
    return raw.substr(start, len);
}

// ── readMessage ──────────────────────────────────────────────────────────────

TEST(LspTransport, ReadMessage_ValidFrame) {
    StdioCapture cap;
    cap.feedMessage(R"({"jsonrpc":"2.0","id":1,"method":"ping"})");
    cap.closeStdin();

    LspTransport transport;
    auto msg = transport.readMessage();
    ASSERT_TRUE(msg.has_value());
    auto *obj = msg->getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getString("method").value_or(""), "ping");
    EXPECT_EQ(obj->getInteger("id").value_or(-1), 1);
}

TEST(LspTransport, ReadMessage_MultipleFramesInSequence) {
    StdioCapture cap;
    cap.feedMessage(R"({"id":1,"method":"first"})");
    cap.feedMessage(R"({"id":2,"method":"second"})");
    cap.feedMessage(R"({"id":3,"method":"third"})");
    cap.closeStdin();

    LspTransport transport;
    auto m1 = transport.readMessage();
    auto m2 = transport.readMessage();
    auto m3 = transport.readMessage();
    auto m4 = transport.readMessage(); // EOF

    ASSERT_TRUE(m1.has_value());
    ASSERT_TRUE(m2.has_value());
    ASSERT_TRUE(m3.has_value());
    EXPECT_FALSE(m4.has_value());

    EXPECT_EQ(m1->getAsObject()->getString("method").value_or(""), "first");
    EXPECT_EQ(m2->getAsObject()->getString("method").value_or(""), "second");
    EXPECT_EQ(m3->getAsObject()->getString("method").value_or(""), "third");
}

TEST(LspTransport, ReadMessage_EofReturnsNullopt) {
    StdioCapture cap;
    cap.closeStdin();

    LspTransport transport;
    auto msg = transport.readMessage();
    EXPECT_FALSE(msg.has_value());
}

TEST(LspTransport, ReadMessage_ZeroContentLength_ReturnsNullopt) {
    StdioCapture cap;
    cap.feedStdin("Content-Length: 0\r\n\r\n");
    cap.closeStdin();

    LspTransport transport;
    auto msg = transport.readMessage();
    EXPECT_FALSE(msg.has_value());
}

TEST(LspTransport, ReadMessage_InvalidJson_ReturnsNullopt) {
    StdioCapture cap;
    std::string bad = "not { valid } json <<<";
    cap.feedStdin("Content-Length: " + std::to_string(bad.size()) + "\r\n\r\n" + bad);
    cap.closeStdin();

    LspTransport transport;
    auto msg = transport.readMessage();
    EXPECT_FALSE(msg.has_value());
}

TEST(LspTransport, ReadMessage_ExtraHeadersIgnored) {
    // Real LSP clients send Content-Type alongside Content-Length.
    StdioCapture cap;
    std::string json = R"({"id":42,"method":"test"})";
    cap.feedStdin("Content-Type: application/vscode-jsonrpc; charset=utf-8\r\n"
                  "Content-Length: " + std::to_string(json.size()) + "\r\n\r\n" + json);
    cap.closeStdin();

    LspTransport transport;
    auto msg = transport.readMessage();
    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->getAsObject()->getInteger("id").value_or(-1), 42);
}

TEST(LspTransport, ReadMessage_LargeBody) {
    StdioCapture cap;
    // Build a JSON object with a 2 KB string value.
    std::string big(2048, 'a');
    std::string json = R"({"method":"big","data":")" + big + R"("})";
    cap.feedMessage(json);
    cap.closeStdin();

    LspTransport transport;
    auto msg = transport.readMessage();
    ASSERT_TRUE(msg.has_value());
    auto *obj = msg->getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getString("data").value_or("").size(), 2048u);
}

// ── send ────────────────────────────────────────────────────────────────────

TEST(LspTransport, Send_OutputStartsWithContentLength) {
    StdioCapture cap;
    json::Object msg{{"jsonrpc", "2.0"}, {"id", 1}};

    LspTransport transport;
    transport.send(msg);

    std::string raw = cap.captureStdout();
    ASSERT_FALSE(raw.empty());
    EXPECT_EQ(raw.substr(0, 16), "Content-Length: ");
}

TEST(LspTransport, Send_ContentLengthMatchesBodySize) {
    StdioCapture cap;
    json::Object msg{{"jsonrpc", "2.0"}, {"id", 99}};

    LspTransport transport;
    transport.send(msg);

    std::string raw = cap.captureStdout();
    const std::string key = "Content-Length: ";
    auto pos = raw.find(key);
    ASSERT_NE(pos, std::string::npos);
    auto sep = raw.find("\r\n\r\n", pos);
    ASSERT_NE(sep, std::string::npos);
    size_t declared = std::stoul(raw.substr(pos + key.size(), sep - pos - key.size()));
    size_t actual   = raw.size() - sep - 4;
    EXPECT_EQ(declared, actual);
}

TEST(LspTransport, Send_BodyIsValidJson) {
    StdioCapture cap;
    json::Object msg{{"jsonrpc", "2.0"}, {"id", 7}, {"result", nullptr}};

    LspTransport transport;
    transport.send(msg);

    std::string body = extractFirstBody(cap.captureStdout());
    ASSERT_FALSE(body.empty());
    auto parsed = json::parse(body);
    ASSERT_TRUE((bool)parsed);
    EXPECT_EQ(parsed->getAsObject()->getInteger("id").value_or(-1), 7);
}

TEST(LspTransport, Send_FrameHasCrlfSeparator) {
    StdioCapture cap;
    json::Object msg{{"x", 1}};

    LspTransport transport;
    transport.send(msg);

    std::string raw = cap.captureStdout();
    EXPECT_NE(raw.find("\r\n\r\n"), std::string::npos);
}

// ── sendResult ───────────────────────────────────────────────────────────────

TEST(LspTransport, SendResult_JsonRpcStructure) {
    StdioCapture cap;
    LspTransport transport;
    transport.sendResult(json::Value(5), json::Value(nullptr));

    std::string body = extractFirstBody(cap.captureStdout());
    auto parsed = json::parse(body);
    ASSERT_TRUE((bool)parsed);
    auto *obj = parsed->getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getString("jsonrpc").value_or(""), "2.0");
    EXPECT_EQ(obj->getInteger("id").value_or(-1), 5);
    EXPECT_NE(obj->get("result"), nullptr);
    EXPECT_EQ(obj->get("error"), nullptr);
}

TEST(LspTransport, SendResult_NullResult) {
    StdioCapture cap;
    LspTransport transport;
    transport.sendResult(json::Value(1), json::Value(nullptr));

    std::string body = extractFirstBody(cap.captureStdout());
    auto parsed = json::parse(body);
    ASSERT_TRUE((bool)parsed);
    const json::Value *res = parsed->getAsObject()->get("result");
    ASSERT_NE(res, nullptr);
    EXPECT_TRUE(res->getAsNull().has_value());
}

TEST(LspTransport, SendResult_ObjectResult) {
    StdioCapture cap;
    LspTransport transport;
    transport.sendResult(json::Value(2),
                         json::Object{{"answer", 42}, {"ok", true}});

    std::string body = extractFirstBody(cap.captureStdout());
    auto parsed = json::parse(body);
    ASSERT_TRUE((bool)parsed);
    auto *res = parsed->getAsObject()->getObject("result");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->getInteger("answer").value_or(-1), 42);
}

TEST(LspTransport, SendResult_ArrayResult) {
    StdioCapture cap;
    LspTransport transport;
    transport.sendResult(json::Value(3), json::Array{});

    std::string body = extractFirstBody(cap.captureStdout());
    auto parsed = json::parse(body);
    ASSERT_TRUE((bool)parsed);
    const json::Value *res = parsed->getAsObject()->get("result");
    ASSERT_NE(res, nullptr);
    EXPECT_NE(res->getAsArray(), nullptr);
}

// ── sendError ────────────────────────────────────────────────────────────────

TEST(LspTransport, SendError_JsonRpcStructure) {
    StdioCapture cap;
    LspTransport transport;
    transport.sendError(json::Value(3), -32601, "Method not found");

    std::string body = extractFirstBody(cap.captureStdout());
    auto parsed = json::parse(body);
    ASSERT_TRUE((bool)parsed);
    auto *obj = parsed->getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getString("jsonrpc").value_or(""), "2.0");
    EXPECT_EQ(obj->getInteger("id").value_or(0), 3);
    EXPECT_EQ(obj->get("result"), nullptr);
}

TEST(LspTransport, SendError_ErrorObject) {
    StdioCapture cap;
    LspTransport transport;
    transport.sendError(json::Value(1), -32601, "Method not found");

    std::string body = extractFirstBody(cap.captureStdout());
    auto parsed = json::parse(body);
    ASSERT_TRUE((bool)parsed);
    auto *err = parsed->getAsObject()->getObject("error");
    ASSERT_NE(err, nullptr);
    EXPECT_EQ(err->getInteger("code").value_or(0), -32601);
    EXPECT_EQ(err->getString("message").value_or(""), "Method not found");
}

TEST(LspTransport, SendError_NotInitialized) {
    StdioCapture cap;
    LspTransport transport;
    transport.sendError(json::Value(1), -32002, "Server not initialized");

    std::string body = extractFirstBody(cap.captureStdout());
    auto parsed = json::parse(body);
    ASSERT_TRUE((bool)parsed);
    auto *err = parsed->getAsObject()->getObject("error");
    ASSERT_NE(err, nullptr);
    EXPECT_EQ(err->getInteger("code").value_or(0), -32002);
    EXPECT_EQ(err->getString("message").value_or(""), "Server not initialized");
}

TEST(LspTransport, SendError_ParseError) {
    StdioCapture cap;
    LspTransport transport;
    transport.sendError(json::Value(nullptr), -32700, "Parse error");

    std::string body = extractFirstBody(cap.captureStdout());
    auto parsed = json::parse(body);
    ASSERT_TRUE((bool)parsed);
    auto *err = parsed->getAsObject()->getObject("error");
    ASSERT_NE(err, nullptr);
    EXPECT_EQ(err->getInteger("code").value_or(0), -32700);
}

// ── sendNotification ─────────────────────────────────────────────────────────

TEST(LspTransport, SendNotification_HasNoId) {
    StdioCapture cap;
    LspTransport transport;
    transport.sendNotification("textDocument/publishDiagnostics",
                               json::Object{{"uri", "file:///a.fly"},
                                            {"diagnostics", json::Array{}}});

    std::string body = extractFirstBody(cap.captureStdout());
    auto parsed = json::parse(body);
    ASSERT_TRUE((bool)parsed);
    auto *obj = parsed->getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->get("id"), nullptr);
}

TEST(LspTransport, SendNotification_JsonRpc) {
    StdioCapture cap;
    LspTransport transport;
    transport.sendNotification("window/logMessage",
                               json::Object{{"type", 4}, {"message", "hello"}});

    std::string body = extractFirstBody(cap.captureStdout());
    auto parsed = json::parse(body);
    ASSERT_TRUE((bool)parsed);
    auto *obj = parsed->getAsObject();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getString("jsonrpc").value_or(""), "2.0");
    EXPECT_EQ(obj->getString("method").value_or(""), "window/logMessage");
}

TEST(LspTransport, SendNotification_ParamsPresent) {
    StdioCapture cap;
    LspTransport transport;
    transport.sendNotification("textDocument/publishDiagnostics",
                               json::Object{{"uri", "file:///b.fly"},
                                            {"diagnostics", json::Array{}}});

    std::string body = extractFirstBody(cap.captureStdout());
    auto parsed = json::parse(body);
    ASSERT_TRUE((bool)parsed);
    auto *obj = parsed->getAsObject();
    auto *params = obj->getObject("params");
    ASSERT_NE(params, nullptr);
    EXPECT_EQ(params->getString("uri").value_or(""), "file:///b.fly");
}

TEST(LspTransport, SendNotification_EmptyDiagnosticsArray) {
    StdioCapture cap;
    LspTransport transport;
    transport.sendNotification("textDocument/publishDiagnostics",
                               json::Object{{"uri", "file:///c.fly"},
                                            {"diagnostics", json::Array{}}});

    std::string body = extractFirstBody(cap.captureStdout());
    auto parsed = json::parse(body);
    ASSERT_TRUE((bool)parsed);
    auto *params = parsed->getAsObject()->getObject("params");
    ASSERT_NE(params, nullptr);
    const json::Value *diags = params->get("diagnostics");
    ASSERT_NE(diags, nullptr);
    auto *arr = diags->getAsArray();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->empty());
}
