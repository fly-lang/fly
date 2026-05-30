#include "LspTransport.h"

#include <llvm/Support/raw_ostream.h>

#include <cstdio>
#include <cstring>
#include <string>

using namespace fly::lsp;
using namespace llvm;

// ── Read ─────────────────────────────────────────────────────────────────────

bool LspTransport::readExact(char *buf, size_t n) {
    size_t read = 0;
    while (read < n) {
        int c = std::fgetc(stdin);
        if (c == EOF) return false;
        buf[read++] = (char)c;
    }
    return true;
}

std::optional<json::Value> LspTransport::readMessage() {
    // Parse headers until blank line
    size_t contentLength = 0;
    std::string headerLine;
    while (true) {
        headerLine.clear();
        while (true) {
            int c = std::fgetc(stdin);
            if (c == EOF) return std::nullopt;
            if (c == '\n') break;
            if (c != '\r') headerLine += (char)c;
        }
        if (headerLine.empty()) break; // blank line → end of headers
        const std::string prefix = "Content-Length: ";
        if (headerLine.substr(0, prefix.size()) == prefix)
            contentLength = std::stoul(headerLine.substr(prefix.size()));
    }

    if (contentLength == 0) return std::nullopt;

    std::string body(contentLength, '\0');
    if (!readExact(body.data(), contentLength)) return std::nullopt;

    auto parsed = json::parse(body);
    if (!parsed) return std::nullopt;
    return std::move(*parsed);
}

// ── Write ────────────────────────────────────────────────────────────────────

void LspTransport::send(const json::Object &msg) {
    std::string body;
    raw_string_ostream ss(body);
    ss << json::Value(json::Object(msg));   // copy Object to construct Value
    ss.flush();

    std::string frame = "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
    // Write atomically to stdout (no buffering issues)
    std::fwrite(frame.data(), 1, frame.size(), stdout);
    std::fflush(stdout);
}

void LspTransport::sendResult(json::Value id, json::Value result) {
    send(json::Object{
        {"jsonrpc", "2.0"},
        {"id",      std::move(id)},
        {"result",  std::move(result)},
    });
}

void LspTransport::sendError(json::Value id, int code, StringRef message) {
    send(json::Object{
        {"jsonrpc", "2.0"},
        {"id",      std::move(id)},
        {"error",   json::Object{{"code", code}, {"message", message}}},
    });
}

void LspTransport::sendNotification(StringRef method, json::Object params) {
    send(json::Object{
        {"jsonrpc", "2.0"},
        {"method",  method},
        {"params",  std::move(params)},
    });
}
