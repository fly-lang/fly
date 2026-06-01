//===----------------------------------------------------------------------===//
// tools/lsp/include/LspTransport.h — stdin/stdout JSON-RPC framing
//
// LSP wire protocol:
//   Content-Length: <N>\r\n
//   \r\n
//   <N bytes of JSON>
//===----------------------------------------------------------------------===//
#ifndef FLY_LSP_TRANSPORT_H
#define FLY_LSP_TRANSPORT_H

#include <llvm/Support/JSON.h>
#include <optional>
#include <string>

namespace fly::lsp {

class LspTransport {
public:
    /// Read the next LSP message from stdin.
    /// Returns nullopt when stdin is closed or a fatal framing error occurs.
    std::optional<llvm::json::Value> readMessage();

    /// Write a JSON-RPC response/notification to stdout with correct framing.
    void send(const llvm::json::Object &msg);

    /// Convenience: build and send a success response for request \p id.
    void sendResult(llvm::json::Value id, llvm::json::Value result);

    /// Convenience: build and send an error response for request \p id.
    void sendError(llvm::json::Value id, int code, llvm::StringRef message);

    /// Convenience: send a server-push notification (no id).
    void sendNotification(llvm::StringRef method, llvm::json::Object params);

private:
    // Read exactly n bytes from stdin into buf. Returns false on EOF/error.
    bool readExact(char *buf, size_t n);
};

} // namespace fly::lsp

#endif // FLY_LSP_TRANSPORT_H
