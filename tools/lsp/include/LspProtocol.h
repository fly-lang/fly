//===----------------------------------------------------------------------===//
// tools/lsp/include/LspProtocol.h — LSP type definitions
// NOTE: structs use "Lsp" prefix to avoid collisions with fly:: compiler types
//===----------------------------------------------------------------------===//
#ifndef FLY_LSP_PROTOCOL_H
#define FLY_LSP_PROTOCOL_H

#include <llvm/Support/JSON.h>
#include <string>

namespace fly::lsp {

struct LspPosition  { int line = 0; int character = 0; };
struct LspRange     { LspPosition start; LspPosition end; };
struct LspLocation  { std::string uri; LspRange range; };

// severity: 1=Error 2=Warning 3=Information 4=Hint
struct LspDiagnostic {
    LspRange    range;
    int         severity = 1;
    std::string message;
    std::string file;   // internal — not sent over LSP
};

// LSP SymbolKind values (subset used by Fly)
enum class LspSymbolKind {
    Function  = 12,
    Variable  = 13,
    Class     = 5,
    Interface = 11,
    Enum      = 10,
    Struct    = 23,
    Field     = 8,
    Method    = 6,
    Module    = 2,
};

struct LspDocSymbol {
    std::string   name;
    LspSymbolKind kind;
    LspRange      range;
    LspRange      selectionRange;
};

enum class LspCompletionKind {
    Text      = 1,
    Function  = 3,
    Field     = 5,
    Variable  = 6,
    Class     = 7,
    Interface = 8,
    Module    = 9,
    Keyword   = 14,
};

struct LspCompletionItem {
    std::string       label;
    LspCompletionKind kind = LspCompletionKind::Text;
    std::string       detail;
    std::string       insertText;
};

// ── JSON helpers ──────────────────────────────────────────────────────────────

inline std::string fileUriToPath(llvm::StringRef uri) {
    if (uri.starts_with("file://")) return uri.substr(7).str();
    return uri.str();
}

inline std::string pathToFileUri(const std::string &path) {
    return "file://" + path;
}

llvm::json::Object toJson(const LspPosition &p);
llvm::json::Object toJson(const LspRange &r);
llvm::json::Object toJson(const LspLocation &l);
llvm::json::Object toJson(const LspDiagnostic &d);
llvm::json::Object toJson(const LspDocSymbol &s);
llvm::json::Object toJson(const LspCompletionItem &c);

LspPosition positionFromJson(const llvm::json::Object &obj);

} // namespace fly::lsp

#endif // FLY_LSP_PROTOCOL_H
