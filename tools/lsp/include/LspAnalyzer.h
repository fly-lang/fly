//===----------------------------------------------------------------------===//
// tools/lsp/include/LspAnalyzer.h — compiler integration + position lookup
//===----------------------------------------------------------------------===//
#ifndef FLY_LSP_ANALYZER_H
#define FLY_LSP_ANALYZER_H

#include "LspProtocol.h"

#include "Basic/Diagnostic.h"
#include "Driver/Driver.h"
#include "Frontend/Frontend.h"

#include <llvm/ADT/SmallVector.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace fly {
    // Forward declarations for AST types used in private helpers
    class ASTExpr;
    class ASTStmt;
    class ASTBlockStmt;
    class SemaModule;
    struct Symbol;
    class SourceManager;
    class SourceLocation;
}

namespace fly::lsp {

// ── LspAnalyzer ──────────────────────────────────────────────────────────────

class LspAnalyzer {
public:
    LspAnalyzer() = default;
    ~LspAnalyzer() = default;

    /// Compile the given files.
    /// Returns LSP diagnostics collected during compilation.
    /// After this call, findSymbolAt / getDefinitionLocation / etc. are valid.
    std::vector<LspDiagnostic> compile(const std::vector<std::string> &files);

    /// Given a file path (not URI) and a 0-based (line, col), return the
    /// resolved Symbol under the cursor, or nullptr if nothing found.
    fly::Symbol *findSymbolAt(const std::string &file, int line, int col);

    /// Return the declaration location of a resolved symbol (as file:// URI).
    std::optional<LspLocation> getDefinitionLocation(fly::Symbol *sym);

    /// Format Markdown hover text for a symbol.
    std::string getHoverMarkdown(fly::Symbol *sym);

    /// Return completion candidates visible at the given position.
    std::vector<LspCompletionItem> getCompletions(const std::string &file,
                                                   int line, int col,
                                                   const std::string &prefix);

    /// Return all declared symbols in the given file (for document outline).
    std::vector<LspDocSymbol> getDocumentSymbols(const std::string &file);

private:
    // Kept alive so AST StringRefs (pointing into Lexer tables) remain valid.
    std::unique_ptr<Driver>   driver_;
    std::unique_ptr<Frontend> frontend_;

    // Pointer into CI's SourceManager; valid while driver_ is alive.
    fly::SourceManager *SM_ = nullptr;

    // ── Helpers ───────────────────────────────────────────────────────────
    LspPosition toPosition(fly::SourceLocation loc) const;

    // Recursive AST walkers for findSymbolAt
    fly::Symbol *walkExpr (fly::ASTExpr  *e, const std::string &f, int l, int c) const;
    fly::Symbol *walkStmt (fly::ASTStmt  *s, const std::string &f, int l, int c) const;
    fly::Symbol *walkBlock(fly::ASTBlockStmt *b, const std::string &f, int l, int c) const;
};

} // namespace fly::lsp

#endif // FLY_LSP_ANALYZER_H
