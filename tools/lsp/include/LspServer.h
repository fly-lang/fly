//===----------------------------------------------------------------------===//
// tools/lsp/include/LspServer.h — LSP request routing + main loop
//===----------------------------------------------------------------------===//
#ifndef FLY_LSP_SERVER_H
#define FLY_LSP_SERVER_H

#include "LspAnalyzer.h"
#include "LspTransport.h"

#include <set>
#include <string>

namespace fly::lsp {

class LspServer {
public:
    explicit LspServer() = default;

    /// Start the server. Blocks until shutdown+exit or stdin closes.
    void run();

private:
    LspTransport transport_;
    LspAnalyzer  analyzer_;

    bool initialized_ = false;
    bool shutdown_    = false;

    // Files currently open in the editor (paths, not URIs)
    std::set<std::string> openFiles_;

    // ── Request handlers ───────────────────────────────────────────────────
    void dispatch(const llvm::json::Object &msg);

    void onInitialize   (llvm::json::Value id, const llvm::json::Object &params);
    void onInitialized  ();
    void onShutdown     (llvm::json::Value id);
    void onExit         ();

    void onDidOpen      (const llvm::json::Object &params);
    void onDidChange    (const llvm::json::Object &params);
    void onDidClose     (const llvm::json::Object &params);

    void onHover             (llvm::json::Value id, const llvm::json::Object &params);
    void onDefinition        (llvm::json::Value id, const llvm::json::Object &params);
    void onCompletion        (llvm::json::Value id, const llvm::json::Object &params);
    void onDocSymbols        (llvm::json::Value id, const llvm::json::Object &params);
    void onReferences        (llvm::json::Value id, const llvm::json::Object &params);
    void onDocumentHighlight (llvm::json::Value id, const llvm::json::Object &params);
    void onSignatureHelp     (llvm::json::Value id, const llvm::json::Object &params);
    void onTypeDefinition    (llvm::json::Value id, const llvm::json::Object &params);
    void onImplementation    (llvm::json::Value id, const llvm::json::Object &params);
    void onFoldingRanges     (llvm::json::Value id, const llvm::json::Object &params);
    void onInlayHints        (llvm::json::Value id, const llvm::json::Object &params);
    void onSemanticTokens    (llvm::json::Value id, const llvm::json::Object &params);
    void onWorkspaceSymbols  (llvm::json::Value id, const llvm::json::Object &params);

    // Recompile all open files and push diagnostics for the given file.
    void recompileAndPublish(const std::string &triggerFile);

    // Extract file path from a params object that contains "textDocument.uri".
    static std::string extractPath(const llvm::json::Object &params);
};

} // namespace fly::lsp

#endif // FLY_LSP_SERVER_H
