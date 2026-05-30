//===----------------------------------------------------------------------===//
// tools/lsp/FlyLsp.cpp — fly-lsp entry point
//
// Usage: fly-lsp
//   Communicates over stdin/stdout using the Language Server Protocol.
//   Launch from a VS Code extension (or any LSP client) pointing the
//   server command to this binary.
//===----------------------------------------------------------------------===//

#include "LspServer.h"

#include <llvm/Support/raw_ostream.h>

int main(int /*argc*/, char ** /*argv*/) {
    // Redirect stderr so accidental prints don't corrupt the LSP stdout stream.
    // Real logging should use a --log-file flag (future work).
    llvm::errs().SetBuffered();

    fly::lsp::LspServer server;
    server.run();
    return 0;
}
