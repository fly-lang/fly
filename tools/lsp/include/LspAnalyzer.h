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
    class ASTCall;
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

    /// Return highlight ranges for every occurrence of the symbol at (line,col).
    std::vector<LspDocumentHighlight>
        getDocumentHighlights(const std::string &file, int line, int col);

    /// Return all reference locations of the symbol at (line,col) across all
    /// compiled modules.  If includeDeclaration is true, the declaration site
    /// is prepended to the result.
    std::vector<LspLocation>
        findReferences(const std::string &file, int line, int col,
                       bool includeDeclaration);

    /// Return signature help for the function call enclosing (line,col),
    /// or nullopt if the cursor is not inside a call argument list.
    std::optional<LspSignatureHelp>
        getSignatureHelp(const std::string &file, int line, int col);

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

    // Recursive AST collectors for getDocumentHighlights
    void collectExpr (fly::ASTExpr      *e, fly::Symbol *sym,
                      std::vector<LspDocumentHighlight> &out) const;
    void collectStmt (fly::ASTStmt      *s, fly::Symbol *sym,
                      std::vector<LspDocumentHighlight> &out) const;
    void collectBlock(fly::ASTBlockStmt *b, fly::Symbol *sym,
                      std::vector<LspDocumentHighlight> &out) const;

    // Find the innermost ASTCall whose argument list spans (line,col).
    // Returns {call, activeParam} or {nullptr, 0}.
    std::pair<fly::ASTCall *, int>
        findEnclosingCall(fly::ASTExpr *e, int line, int col) const;
    std::pair<fly::ASTCall *, int>
        findEnclosingCallInBlock(fly::ASTBlockStmt *b, int line, int col) const;
};

} // namespace fly::lsp

#endif // FLY_LSP_ANALYZER_H
