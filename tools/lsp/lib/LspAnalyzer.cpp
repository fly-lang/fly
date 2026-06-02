#include "LspAnalyzer.h"

#include "AST/ASTArg.h"
#include "AST/ASTBinary.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTCall.h"
#include "AST/ASTClass.h"
#include "AST/ASTDeclStmt.h"
#include "AST/ASTExpr.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTFunction.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTMember.h"
#include "AST/ASTRuleStmt.h"
#include "AST/ASTStmt.h"
#include "AST/ASTType.h"
#include "AST/ASTVar.h"
#include "Basic/SourceLocation.h"
#include "Basic/SourceManager.h"
#include "Frontend/CompilerInstance.h"
#include "Sema/SemaClassAttribute.h"
#include "Sema/SemaClassMethod.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaFunction.h"
#include "Sema/SemaLocalVar.h"
#include "Sema/SemaModule.h"
#include "Sema/SemaNameSpace.h"
#include "Sema/SemaNode.h"
#include "Sema/SemaParam.h"
#include "Sema/SemaType.h"
#include "Sema/Symbol.h"
#include "Sema/SymbolTable.h"
#include "AST/ASTName.h"

#include <llvm/ADT/SmallString.h>
#include <llvm/Support/TargetSelect.h>
#include <algorithm>
#include <cctype>
#include <unordered_set>

// All method definitions are inside the explicit namespace block so MSVC
// resolves the class without relying on `using namespace` for out-of-class
// member definitions (MSVC /permissive- mode is stricter about this).
namespace fly {
namespace lsp {

// Pull compiler types into scope inside the namespace block
using fly::ASTArg;
using fly::ASTBinary;
using fly::ASTBlockStmt;
using fly::ASTCall;
using fly::ASTClass;
using fly::ASTClassKind;
using fly::ASTDeclStmt;
using fly::ASTExpr;
using fly::ASTExprKind;
using fly::ASTExprStmt;
using fly::ASTFunction;
using fly::ASTIdentifier;
using fly::ASTMember;
using fly::ASTRuleStmt;
using fly::ASTStmt;
using fly::ASTStmtKind;
using fly::CompilerInstance;
using fly::Driver;
using fly::Frontend;
using fly::PresumedLoc;
using fly::SemaClassAttribute;
using fly::SemaClassKind;
using fly::SemaClassMethod;
using fly::SemaClassType;
using fly::SemaFunction;
using fly::SemaFunctionBase;
using fly::SemaKind;
using fly::SemaLocalVar;
using fly::SemaModule;
using fly::SemaNameSpace;
using fly::SemaNode;
using fly::SemaParam;
using fly::SemaType;
using fly::SemaVar;
using fly::SourceLocation;
using fly::Symbol;
using fly::SymbolKind;

// ── In-process diagnostic consumer ───────────────────────────────────────────

// Captures compiler diagnostics as LspDiagnostic objects during Execute().
// Installed on the DiagnosticsEngine with ShouldOwnClient=false so the engine
// does not try to delete it; the caller resets the client before returning.
class LspCapturingConsumer : public fly::DiagnosticConsumer {
    fly::SourceManager           *SM_;
    std::vector<LspDiagnostic>  &out_;
public:
    LspCapturingConsumer(fly::SourceManager *sm, std::vector<LspDiagnostic> &out)
        : SM_(sm), out_(out) {}

    void HandleDiagnostic(fly::DiagnosticsEngine::Level level,
                          const fly::Diagnostic &info) override {
        DiagnosticConsumer::HandleDiagnostic(level, info);  // update counts
        if (level == fly::DiagnosticsEngine::Ignored) return;

        // Format the diagnostic message text.
        llvm::SmallString<256> msg;
        info.FormatDiagnostic(msg);

        // Resolve source location to file path and 0-based position.
        LspPosition pos{0, 0};
        std::string file;
        fly::SourceLocation loc = info.getLocation();
        if (SM_ && loc.isValid()) {
            fly::PresumedLoc pl = SM_->getPresumedLoc(loc);
            if (pl.isValid()) {
                pos  = {(int)pl.getLine() - 1, (int)pl.getColumn() - 1};
                file = pl.getFilename();
            }
        }

        int severity;
        switch (level) {
        case fly::DiagnosticsEngine::Error:
        case fly::DiagnosticsEngine::Fatal:   severity = 1; break;
        case fly::DiagnosticsEngine::Warning: severity = 2; break;
        default:                              severity = 3; break; // Note/Remark
        }

        LspDiagnostic d;
        d.range    = {pos, pos};
        d.severity = severity;
        d.message  = msg.str().str();
        d.file     = std::move(file);
        out_.push_back(std::move(d));
    }
};

// ── Compile ───────────────────────────────────────────────────────────────────

std::vector<LspDiagnostic> LspAnalyzer::compile(
        const std::vector<std::string> &files) {
    // Init LLVM targets once per process.
    static bool targetInit = false;
    if (!targetInit) {
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();
        targetInit = true;
    }

    // Build argv: fly --no-output file1 file2 ...
    std::vector<const char *> argv;
    argv.push_back("fly");
    argv.push_back("--no-output");
    for (const auto &f : files) argv.push_back(f.c_str());

    driver_.reset();
    frontend_.reset();
    SM_ = nullptr;

    driver_   = std::make_unique<Driver>(
                    llvm::ArrayRef<const char *>(argv.data(), argv.size()));
    CompilerInstance &ci = driver_->BuildCompilerInstance();
    SM_ = &ci.getSourceManager();

    // Install our capturing consumer before Execute() so every diagnostic
    // emitted during parsing / sema / codegen is captured in-process.
    // ShouldOwnClient=false: we manage the lifetime ourselves.
    std::vector<LspDiagnostic> results;
    {
        LspCapturingConsumer consumer(SM_, results);
        ci.getDiagnostics().setClient(&consumer, /*ShouldOwnClient=*/false);

        frontend_ = std::make_unique<Frontend>(ci);
        frontend_->Execute();

        // Detach before the consumer goes out of scope so the engine never
        // holds a dangling pointer (e.g. if an LLVM handler fires later).
        ci.getDiagnostics().setClient(nullptr, /*ShouldOwnClient=*/false);
    }
    return results;
}

// ── Position helpers ──────────────────────────────────────────────────────────

LspPosition LspAnalyzer::toPosition(SourceLocation loc) const {
    if (!SM_ || !loc.isValid()) return {0, 0};
    PresumedLoc pl = SM_->getPresumedLoc(loc);
    if (pl.isInvalid()) return {0, 0};
    // PresumedLoc is 1-based; LSP is 0-based
    return {(int)pl.getLine() - 1, (int)pl.getColumn() - 1};
}

// ── AST walkers ───────────────────────────────────────────────────────────────

Symbol *LspAnalyzer::walkExpr(ASTExpr *expr,
                               const std::string &file, int line, int col) const {
    if (!expr) return nullptr;

    LspPosition epos = toPosition(expr->getLocation());

    switch (expr->getExprKind()) {

    case ASTExprKind::EXPR_IDENTIFIER: {
        auto *id = static_cast<ASTIdentifier *>(expr);
        // Identifier starts at epos and spans getName().size() characters
        if (epos.line == line &&
            col >= epos.character &&
            col <  epos.character + (int)id->getName().size())
            return id->getSymbol();
        break;
    }

    case ASTExprKind::EXPR_MEMBER: {
        auto *mem = static_cast<ASTMember *>(expr);
        if (epos.line == line &&
            col >= epos.character &&
            col <  epos.character + (int)mem->getName().size())
            return mem->getSymbol();
        // Walk parent (the receiver expression)
        if (auto *p = expr->getParent())
            if (auto *s = walkExpr(p, file, line, col)) return s;
        break;
    }

    case ASTExprKind::EXPR_CALL: {
        auto *call = static_cast<ASTCall *>(expr);
        if (epos.line == line &&
            col >= epos.character &&
            col <  epos.character + (int)call->getName().size())
            return call->getSymbol();
        // Walk receiver and arguments
        if (auto *p = expr->getParent())
            if (auto *s = walkExpr(p, file, line, col)) return s;
        for (auto *arg : call->getArgs())
            if (auto *s = walkExpr(arg->getExpr(), file, line, col)) return s;
        break;
    }

    case ASTExprKind::EXPR_BINARY: {
        auto *bin = static_cast<ASTBinary *>(expr);
        if (auto *s = walkExpr(bin->getLeftExpr(),  file, line, col)) return s;
        if (auto *s = walkExpr(bin->getRightExpr(), file, line, col)) return s;
        break;
    }

    default:
        break;
    }
    return nullptr;
}

Symbol *LspAnalyzer::walkStmt(ASTStmt *stmt,
                               const std::string &file, int line, int col) const {
    if (!stmt) return nullptr;
    switch (stmt->getStmtKind()) {
    case ASTStmtKind::STMT_BLOCK:
        return walkBlock(static_cast<ASTBlockStmt *>(stmt), file, line, col);
    case ASTStmtKind::STMT_EXPR:
        return walkExpr(static_cast<ASTExprStmt *>(stmt)->getExpr(),
                        file, line, col);
    case ASTStmtKind::STMT_DECL:
        return walkExpr(static_cast<ASTDeclStmt *>(stmt)->getExpr(),
                        file, line, col);
    case ASTStmtKind::STMT_RULE:
    case ASTStmtKind::STMT_IF:
    case ASTStmtKind::STMT_SWITCH: {
        auto *rule = static_cast<ASTRuleStmt *>(stmt);
        if (auto *s = walkExpr(rule->getExpr(), file, line, col)) return s;
        return walkStmt(rule->getStmt(), file, line, col);
    }
    default:
        return nullptr;
    }
}

Symbol *LspAnalyzer::walkBlock(ASTBlockStmt *block,
                                const std::string &file, int line, int col) const {
    if (!block) return nullptr;
    for (auto *stmt : block->getContent())
        if (auto *s = walkStmt(stmt, file, line, col)) return s;
    return nullptr;
}

// ── findSymbolAt ─────────────────────────────────────────────────────────────

Symbol *LspAnalyzer::findSymbolAt(const std::string &file, int line, int col) {
    if (!frontend_) return nullptr;

    for (SemaModule *mod : frontend_->getSemaModules()) {
        const std::string &modFile = mod->getAST().getName();
        if (!modFile.empty() && modFile != file) continue;

        for (SemaNode *node : mod->getNodes()) {
            switch (node->getKind()) {
            case SemaKind::FUNCTION: {
                auto *fn = static_cast<SemaFunction *>(node);
                if (fn->getAST().getBody())
                    if (auto *s = walkBlock(fn->getAST().getBody(),
                                            file, line, col))
                        return s;
                break;
            }
            case SemaKind::TYPE_CLASS: {
                auto *cls = static_cast<SemaClassType *>(node);
                for (auto it = cls->getMethods().begin();
                         it != cls->getMethods().end(); ++it) {
                    SemaClassMethod *method = it->getValue();
                    if (method->getAST().getBody())
                        if (auto *s = walkBlock(method->getAST().getBody(),
                                                file, line, col))
                            return s;
                }
                break;
            }
            default:
                break;
            }
        }
    }
    return nullptr;
}

// ── getDefinitionLocation ─────────────────────────────────────────────────────

std::optional<LspLocation> LspAnalyzer::getDefinitionLocation(Symbol *sym) {
    if (!sym || !SM_) return std::nullopt;

    SourceLocation loc;
    using SK = SymbolKind;
    switch (sym->getKind()) {
    case SK::FUNCTION: {
        auto *fn = static_cast<SemaFunctionBase *>(sym->getRef());
        loc = fn->getAST().getLocation();
        break;
    }
    case SK::CLASS: {
        auto *cls = static_cast<SemaClassType *>(sym->getRef());
        loc = cls->getAST().getLocation();
        break;
    }
    case SK::LOCAL_VAR: {
        auto *var = static_cast<SemaLocalVar *>(sym->getRef());
        if (var->getAST()) loc = var->getAST()->getLocation();
        break;
    }
    case SK::PARAM: {
        auto *par = static_cast<SemaParam *>(sym->getRef());
        if (par->getAST()) loc = par->getAST()->getLocation();
        break;
    }
    case SK::ATTRIBUTE: {
        auto *attr = static_cast<SemaClassAttribute *>(sym->getRef());
        if (attr->getAST()) loc = attr->getAST()->getLocation();
        break;
    }
    default:
        return std::nullopt;
    }

    if (!loc.isValid()) return std::nullopt;
    PresumedLoc pl = SM_->getPresumedLoc(loc);
    if (pl.isInvalid()) return std::nullopt;

    LspPosition p = {(int)pl.getLine() - 1, (int)pl.getColumn() - 1};
    LspRange    r{p, p};
    return LspLocation{pathToFileUri(pl.getFilename()), r};
}

// ── getTypeDefinitionLocation ─────────────────────────────────────────────────

std::optional<LspLocation> LspAnalyzer::getTypeDefinitionLocation(Symbol *sym) {
    if (!sym || !SM_) return std::nullopt;

    ASTType *astType = nullptr;

    switch (sym->getKind()) {
    case SymbolKind::LOCAL_VAR: {
        auto *var = static_cast<SemaLocalVar *>(sym->getRef());
        if (var->getAST()) astType = var->getAST()->getType();
        break;
    }
    case SymbolKind::PARAM: {
        auto *par = static_cast<SemaParam *>(sym->getRef());
        if (par->getAST()) astType = par->getAST()->getType();
        break;
    }
    case SymbolKind::ATTRIBUTE: {
        auto *attr = static_cast<SemaClassAttribute *>(sym->getRef());
        if (attr->getAST()) astType = attr->getAST()->getType();
        break;
    }
    case SymbolKind::CLASS:
        // The symbol IS a type — reuse getDefinitionLocation.
        return getDefinitionLocation(sym);
    default:
        return std::nullopt;
    }

    if (!astType) return std::nullopt;

    // Built-in types (int, bool, string, …) have no declaration location.
    if (astType->getTypeKind() == ASTTypeKind::TYPE_BUILTIN) return std::nullopt;

    if (astType->getTypeKind() == ASTTypeKind::TYPE_NAMED) {
        auto *named = static_cast<ASTNamedType *>(astType);
        const auto &names = named->getNames();
        if (names.empty()) return std::nullopt;
        // Resolve the type name through all modules' symbol tables.
        std::string typeName = names.back()->getName().str();
        if (!frontend_) return std::nullopt;
        for (SemaModule *mod : frontend_->getSemaModules()) {
            for (SemaNode *node : mod->getNodes()) {
                if (node->getKind() == SemaKind::TYPE_CLASS) {
                    auto *cls = static_cast<SemaClassType *>(node);
                    if (cls->getAST().getName().str() == typeName) {
                        // Found the class — return its location.
                        SourceLocation loc = cls->getAST().getLocation();
                        if (!loc.isValid()) continue;
                        PresumedLoc pl = SM_->getPresumedLoc(loc);
                        if (pl.isInvalid()) continue;
                        LspPosition p{(int)pl.getLine() - 1, (int)pl.getColumn() - 1};
                        return LspLocation{pathToFileUri(pl.getFilename()), {p, p}};
                    }
                }
            }
        }
    }

    return std::nullopt;
}

// ── getHoverMarkdown ─────────────────────────────────────────────────────────

std::string LspAnalyzer::getHoverMarkdown(Symbol *sym) {
    if (!sym) return "";
    std::string md;
    using SK = SymbolKind;

    switch (sym->getKind()) {
    case SK::FUNCTION: {
        auto *fn = static_cast<SemaFunctionBase *>(sym->getRef());
        md = "```fly\n";
        if (fn->getReturnType())
            md += fn->getReturnType()->getName() + " ";
        md += fn->getAST().getName().str() + "(";
        bool first = true;
        for (auto *p : fn->getParams()) {
            if (!first) md += ", ";
            md += "const ";
            if (p->getAST() && p->getAST()->getType())
                md += p->getAST()->getType()->str() + " ";
            if (p->getAST()) md += p->getAST()->getName().str();
            first = false;
        }
        md += ")\n```";
        break;
    }
    case SK::CLASS: {
        auto *cls = static_cast<SemaClassType *>(sym->getRef());
        const char *kw = cls->getClassKind() == SemaClassKind::STRUCT    ? "struct"
                       : cls->getClassKind() == SemaClassKind::INTERFACE  ? "interface"
                                                                           : "class";
        md = "```fly\n" + std::string(kw) + " "
           + cls->getAST().getName().str() + "\n```";
        break;
    }
    case SK::LOCAL_VAR: {
        auto *var = static_cast<SemaLocalVar *>(sym->getRef());
        if (var->getAST()) {
            md = "```fly\n(local) ";
            if (var->getAST()->getType())
                md += var->getAST()->getType()->str() + " ";
            md += var->getAST()->getName().str() + "\n```";
        }
        break;
    }
    case SK::PARAM: {
        auto *par = static_cast<SemaParam *>(sym->getRef());
        if (par->getAST()) {
            md = "```fly\n(param) const ";
            if (par->getAST()->getType())
                md += par->getAST()->getType()->str() + " ";
            md += par->getAST()->getName().str() + "\n```";
        }
        break;
    }
    case SK::ATTRIBUTE: {
        auto *attr = static_cast<SemaClassAttribute *>(sym->getRef());
        md = "```fly\n(field) ";
        if (attr->getAST() && attr->getAST()->getType())
            md += attr->getAST()->getType()->str() + " ";
        if (attr->getAST())
            md += attr->getAST()->getName().str();
        md += "\n```";
        break;
    }
    case SK::NAMESPACE: {
        auto *ns = static_cast<SemaNameSpace *>(sym->getRef());
        md = "```fly\nnamespace " + ns->getName().str() + "\n```";
        break;
    }
    default:
        md = "```fly\n" + sym->getName() + "\n```";
        break;
    }
    return md;
}

// ── getCompletions ────────────────────────────────────────────────────────────

// Map SymbolKind → LspCompletionKind for getCompletions.
static LspCompletionKind symbolToCompletionKind(SymbolKind k) {
    switch (k) {
    case SymbolKind::FUNCTION:   return LspCompletionKind::Function;
    case SymbolKind::CLASS:      return LspCompletionKind::Class;
    case SymbolKind::LOCAL_VAR:  return LspCompletionKind::Variable;
    case SymbolKind::PARAM:      return LspCompletionKind::Variable;
    case SymbolKind::ATTRIBUTE:  return LspCompletionKind::Field;
    case SymbolKind::NAMESPACE:  return LspCompletionKind::Module;
    case SymbolKind::ENUM_ENTRY: return LspCompletionKind::Field;
    default:                     return LspCompletionKind::Text;
    }
}

// Collect all symbols from a SymbolTable (local scope + parent chain).
static void collectSymbols(SymbolTable *table,
                            const std::string &prefix,
                            std::vector<LspCompletionItem> &out,
                            std::unordered_set<std::string> &seen) {
    for (SymbolTable *t = table; t; t = t->getParent()) {
        for (const auto &entry : t->getAll()) {
            std::string name = entry.first().str();
            if (seen.count(name)) continue;
            if (!prefix.empty()) {
                std::string nameLow = name, pfxLow = prefix;
                for (auto &c : nameLow) c = (char)std::tolower((unsigned char)c);
                for (auto &c : pfxLow)  c = (char)std::tolower((unsigned char)c);
                if (nameLow.find(pfxLow) == std::string::npos) continue;
            }
            seen.insert(name);
            for (Symbol *sym : entry.second) {
                LspCompletionItem item;
                item.label      = name;
                item.kind       = symbolToCompletionKind(sym->getKind());
                item.insertText = name;
                out.push_back(std::move(item));
                break;  // one item per name
            }
        }
    }
}

std::vector<LspCompletionItem> LspAnalyzer::getCompletions(
        const std::string &file, int line, int col,
        const std::string &prefix) {
    if (!frontend_) return {};

    std::vector<LspCompletionItem> result;
    std::unordered_set<std::string> seen;

    for (SemaModule *mod : frontend_->getSemaModules()) {
        const std::string &mf = mod->getAST().getName();

        if (!mf.empty() && mf == file) {
            // Local scope: find the enclosing function and use its SymbolTable.
            for (SemaNode *node : mod->getNodes()) {
                if (node->getKind() != SemaKind::FUNCTION) continue;
                auto *fn = static_cast<SemaFunction *>(node);
                if (!fn->getAST().getBody()) continue;
                // Check if cursor is inside this function body.
                LspPosition fnPos = toPosition(fn->getAST().getLocation());
                if (fnPos.line <= line && fn->getSymbols()) {
                    collectSymbols(fn->getSymbols(), prefix, result, seen);
                }
            }
        }

        // Global symbols from every module (functions, classes, namespaces).
        if (mod->getSymbols())
            collectSymbols(mod->getSymbols(), prefix, result, seen);
    }

    return result;
}

// ── getDocumentSymbols ────────────────────────────────────────────────────────

std::vector<LspDocSymbol> LspAnalyzer::getDocumentSymbols(
        const std::string &file) {
    std::vector<LspDocSymbol> result;
    if (!frontend_) return result;

    for (SemaModule *mod : frontend_->getSemaModules()) {
        const std::string &mf = mod->getAST().getName();
        if (!mf.empty() && mf != file) continue;

        for (SemaNode *node : mod->getNodes()) {
            switch (node->getKind()) {
            case SemaKind::FUNCTION: {
                auto *fn = static_cast<SemaFunction *>(node);
                LspPosition p = toPosition(fn->getAST().getLocation());
                LspRange r{p, p};
                result.push_back({fn->getAST().getName().str(),
                                   LspSymbolKind::Function, r, r});
                break;
            }
            case SemaKind::TYPE_CLASS: {
                auto *cls = static_cast<SemaClassType *>(node);
                LspSymbolKind kind =
                    cls->getClassKind() == SemaClassKind::STRUCT    ? LspSymbolKind::Struct
                  : cls->getClassKind() == SemaClassKind::INTERFACE  ? LspSymbolKind::Interface
                                                                     : LspSymbolKind::Class;
                LspPosition p = toPosition(cls->getAST().getLocation());
                LspRange r{p, p};
                result.push_back({cls->getAST().getName().str(), kind, r, r});
                break;
            }
            default:
                break;
            }
        }
    }
    return result;
}

// ── getDocumentHighlights — collectors ────────────────────────────────────────

void LspAnalyzer::collectExpr(ASTExpr *expr, Symbol *sym,
                               std::vector<LspDocumentHighlight> &out) const {
    if (!expr || !sym) return;

    LspPosition epos = toPosition(expr->getLocation());

    auto push = [&](int nameLen, int kind) {
        LspPosition start = epos;
        LspPosition end   = {epos.line, epos.character + nameLen};
        out.push_back({LspRange{start, end}, kind});
    };

    switch (expr->getExprKind()) {

    case ASTExprKind::EXPR_IDENTIFIER: {
        auto *id = static_cast<ASTIdentifier *>(expr);
        if (id->getSymbol() == sym)
            push((int)id->getName().size(), 2); // Read
        break;
    }

    case ASTExprKind::EXPR_MEMBER: {
        auto *mem = static_cast<ASTMember *>(expr);
        if (mem->getSymbol() == sym)
            push((int)mem->getName().size(), 2);
        if (auto *p = expr->getParent())
            collectExpr(p, sym, out);
        break;
    }

    case ASTExprKind::EXPR_CALL: {
        auto *call = static_cast<ASTCall *>(expr);
        if (call->getSymbol() == sym)
            push((int)call->getName().size(), 2);
        if (auto *p = expr->getParent())
            collectExpr(p, sym, out);
        for (auto *arg : call->getArgs())
            collectExpr(arg->getExpr(), sym, out);
        break;
    }

    case ASTExprKind::EXPR_BINARY: {
        auto *bin = static_cast<ASTBinary *>(expr);
        collectExpr(bin->getLeftExpr(),  sym, out);
        collectExpr(bin->getRightExpr(), sym, out);
        break;
    }

    default:
        break;
    }
}

void LspAnalyzer::collectStmt(ASTStmt *stmt, Symbol *sym,
                               std::vector<LspDocumentHighlight> &out) const {
    if (!stmt) return;
    switch (stmt->getStmtKind()) {
    case ASTStmtKind::STMT_BLOCK:
        collectBlock(static_cast<ASTBlockStmt *>(stmt), sym, out);
        break;
    case ASTStmtKind::STMT_EXPR:
        collectExpr(static_cast<ASTExprStmt *>(stmt)->getExpr(), sym, out);
        break;
    case ASTStmtKind::STMT_DECL:
        // The RHS expression — mark write site
        collectExpr(static_cast<ASTDeclStmt *>(stmt)->getExpr(), sym, out);
        break;
    case ASTStmtKind::STMT_RULE:
    case ASTStmtKind::STMT_IF:
    case ASTStmtKind::STMT_SWITCH: {
        auto *rule = static_cast<ASTRuleStmt *>(stmt);
        collectExpr(rule->getExpr(), sym, out);
        collectStmt(rule->getStmt(), sym, out);
        break;
    }
    default:
        break;
    }
}

void LspAnalyzer::collectBlock(ASTBlockStmt *block, Symbol *sym,
                                std::vector<LspDocumentHighlight> &out) const {
    if (!block) return;
    for (auto *stmt : block->getContent())
        collectStmt(stmt, sym, out);
}

// ── getDocumentHighlights ─────────────────────────────────────────────────────

std::vector<LspDocumentHighlight>
LspAnalyzer::getDocumentHighlights(const std::string &file, int line, int col) {
    if (!frontend_) return {};

    // Identify which symbol is under the cursor.
    Symbol *sym = findSymbolAt(file, line, col);
    if (!sym) return {};

    // Walk the entire file collecting all references to that symbol.
    std::vector<LspDocumentHighlight> out;
    for (SemaModule *mod : frontend_->getSemaModules()) {
        const std::string &mf = mod->getAST().getName();
        if (!mf.empty() && mf != file) continue;

        for (SemaNode *node : mod->getNodes()) {
            switch (node->getKind()) {
            case SemaKind::FUNCTION: {
                auto *fn = static_cast<SemaFunction *>(node);
                if (fn->getAST().getBody())
                    collectBlock(fn->getAST().getBody(), sym, out);
                break;
            }
            case SemaKind::TYPE_CLASS: {
                auto *cls = static_cast<SemaClassType *>(node);
                for (auto it = cls->getMethods().begin();
                         it != cls->getMethods().end(); ++it) {
                    SemaClassMethod *method = it->getValue();
                    if (method->getAST().getBody())
                        collectBlock(method->getAST().getBody(), sym, out);
                }
                break;
            }
            default:
                break;
            }
        }
    }
    return out;
}

// ── findReferences ────────────────────────────────────────────────────────────

std::vector<LspLocation>
LspAnalyzer::findReferences(const std::string &file, int line, int col,
                             bool includeDeclaration) {
    if (!frontend_) return {};

    Symbol *sym = findSymbolAt(file, line, col);
    if (!sym) return {};

    std::vector<LspLocation> result;

    // Optionally prepend the declaration location.
    if (includeDeclaration) {
        if (auto declLoc = getDefinitionLocation(sym))
            result.push_back(std::move(*declLoc));
    }

    // Walk ALL compiled modules (not just the current file) and collect every
    // expression that resolves to the same Symbol pointer.
    for (SemaModule *mod : frontend_->getSemaModules()) {
        const std::string &mf = mod->getAST().getName();
        std::string moduleUri = mf.empty() ? pathToFileUri(file) : pathToFileUri(mf);

        // Collect highlights (position ranges) within this module.
        std::vector<LspDocumentHighlight> hits;
        for (SemaNode *node : mod->getNodes()) {
            switch (node->getKind()) {
            case SemaKind::FUNCTION: {
                auto *fn = static_cast<SemaFunction *>(node);
                if (fn->getAST().getBody())
                    collectBlock(fn->getAST().getBody(), sym, hits);
                break;
            }
            case SemaKind::TYPE_CLASS: {
                auto *cls = static_cast<SemaClassType *>(node);
                for (auto it = cls->getMethods().begin();
                         it != cls->getMethods().end(); ++it) {
                    SemaClassMethod *method = it->getValue();
                    if (method->getAST().getBody())
                        collectBlock(method->getAST().getBody(), sym, hits);
                }
                break;
            }
            default:
                break;
            }
        }

        // Convert each highlight to an LspLocation in this module's file.
        for (const auto &h : hits)
            result.push_back({moduleUri, h.range});
    }

    return result;
}

// ── getSignatureHelp — helpers ────────────────────────────────────────────────

// Returns {innermost enclosing ASTCall, active param index} for the position
// (line, col), or {nullptr, 0} if the cursor is not inside a call arg list.
std::pair<ASTCall *, int>
LspAnalyzer::findEnclosingCall(ASTExpr *expr, int line, int col) const {
    if (!expr) return {nullptr, 0};

    if (expr->getExprKind() == ASTExprKind::EXPR_CALL) {
        auto *call = static_cast<ASTCall *>(expr);
        LspPosition callPos = toPosition(call->getLocation());

        // Cursor must be after the call name (i.e., inside the argument list).
        if (callPos.line > line || (callPos.line == line && callPos.character > col))
            return {nullptr, 0};

        // Count the active parameter by checking argument positions.
        int activeParam = 0;
        for (auto *arg : call->getArgs()) {
            if (!arg->getExpr()) continue;
            LspPosition argPos = toPosition(arg->getExpr()->getLocation());
            if (argPos.line < line ||
                (argPos.line == line && argPos.character <= col))
                activeParam++;
            else
                break;
        }
        if (activeParam > 0) activeParam--;  // args after = next param still current

        // Recurse into arguments for nested calls (prefer innermost).
        for (auto *arg : call->getArgs()) {
            auto [inner, ip] = findEnclosingCall(arg->getExpr(), line, col);
            if (inner) return {inner, ip};
        }

        return {call, activeParam};
    }

    // Recurse into binary and member sub-expressions.
    if (expr->getExprKind() == ASTExprKind::EXPR_BINARY) {
        auto *bin = static_cast<ASTBinary *>(expr);
        if (auto [c, i] = findEnclosingCall(bin->getLeftExpr(),  line, col); c) return {c, i};
        if (auto [c, i] = findEnclosingCall(bin->getRightExpr(), line, col); c) return {c, i};
    }
    return {nullptr, 0};
}

std::pair<ASTCall *, int>
LspAnalyzer::findEnclosingCallInBlock(ASTBlockStmt *block, int line, int col) const {
    if (!block) return {nullptr, 0};
    for (auto *stmt : block->getContent()) {
        if (!stmt) continue;
        ASTExpr *expr = nullptr;
        switch (stmt->getStmtKind()) {
        case ASTStmtKind::STMT_EXPR: expr = static_cast<ASTExprStmt *>(stmt)->getExpr(); break;
        case ASTStmtKind::STMT_DECL: expr = static_cast<ASTDeclStmt *>(stmt)->getExpr(); break;
        case ASTStmtKind::STMT_BLOCK:
            if (auto [c, i] = findEnclosingCallInBlock(
                    static_cast<ASTBlockStmt *>(stmt), line, col); c) return {c, i};
            break;
        case ASTStmtKind::STMT_RULE:
        case ASTStmtKind::STMT_IF:
        case ASTStmtKind::STMT_SWITCH: {
            auto *rule = static_cast<ASTRuleStmt *>(stmt);
            if (auto [c, i] = findEnclosingCall(rule->getExpr(), line, col); c) return {c, i};
            break;
        }
        default: break;
        }
        if (expr) {
            if (auto [c, i] = findEnclosingCall(expr, line, col); c) return {c, i};
        }
    }
    return {nullptr, 0};
}

// ── getSignatureHelp ──────────────────────────────────────────────────────────

std::optional<LspSignatureHelp>
LspAnalyzer::getSignatureHelp(const std::string &file, int line, int col) {
    if (!frontend_) return std::nullopt;

    // Find the innermost call enclosing the cursor.
    ASTCall *call  = nullptr;
    int activeParam = 0;
    for (SemaModule *mod : frontend_->getSemaModules()) {
        const std::string &mf = mod->getAST().getName();
        if (!mf.empty() && mf != file) continue;

        for (SemaNode *node : mod->getNodes()) {
            switch (node->getKind()) {
            case SemaKind::FUNCTION: {
                auto *fn = static_cast<SemaFunction *>(node);
                if (fn->getAST().getBody()) {
                    auto [c, i] = findEnclosingCallInBlock(fn->getAST().getBody(), line, col);
                    if (c) { call = c; activeParam = i; }
                }
                break;
            }
            case SemaKind::TYPE_CLASS: {
                auto *cls = static_cast<SemaClassType *>(node);
                for (auto it = cls->getMethods().begin();
                         it != cls->getMethods().end(); ++it) {
                    SemaClassMethod *method = it->getValue();
                    if (method->getAST().getBody()) {
                        auto [c, i] = findEnclosingCallInBlock(
                            method->getAST().getBody(), line, col);
                        if (c) { call = c; activeParam = i; }
                    }
                }
                break;
            }
            default: break;
            }
        }
        if (call) break;
    }

    if (!call) return std::nullopt;

    Symbol *sym = call->getSymbol();
    if (!sym || sym->getKind() != SymbolKind::FUNCTION) return std::nullopt;

    auto *fn = static_cast<SemaFunctionBase *>(sym->getRef());
    if (!fn) return std::nullopt;

    // Build the signature label: "retType name(type param, …)"
    std::string label;
    if (fn->getReturnType()) label += fn->getReturnType()->getName() + " ";
    label += fn->getAST().getName().str() + "(";

    LspSignatureInfo sig;
    bool first = true;
    for (auto *p : fn->getParams()) {
        if (!first) label += ", ";
        std::string paramLabel;
        if (p->getAST() && p->getAST()->getType())
            paramLabel += p->getAST()->getType()->str() + " ";
        if (p->getAST()) paramLabel += p->getAST()->getName().str();
        label += paramLabel;
        sig.parameters.push_back({paramLabel});
        first = false;
    }
    label += ")";
    sig.label = label;

    LspSignatureHelp help;
    help.signatures.push_back(std::move(sig));
    help.activeSignature = 0;
    help.activeParameter = activeParam < (int)help.signatures[0].parameters.size()
                         ? activeParam : 0;
    return help;
}

// ── getFoldingRanges ──────────────────────────────────────────────────────────

std::vector<LspFoldingRange> LspAnalyzer::getFoldingRanges(const std::string &file) {
    std::vector<LspFoldingRange> out;
    if (!frontend_) return out;

    for (SemaModule *mod : frontend_->getSemaModules()) {
        const std::string &mf = mod->getAST().getName();
        if (!mf.empty() && mf != file) continue;

        for (SemaNode *node : mod->getNodes()) {
            switch (node->getKind()) {
            case SemaKind::FUNCTION: {
                auto *fn = static_cast<SemaFunction *>(node);
                LspPosition start = toPosition(fn->getAST().getLocation());
                ASTBlockStmt *body = fn->getAST().getBody();
                if (body && !body->getContent().empty()) {
                    LspPosition last = toPosition(
                        body->getContent().back()->getLocation());
                    if (last.line > start.line)
                        out.push_back({start.line, last.line, ""});
                }
                break;
            }
            case SemaKind::TYPE_CLASS: {
                auto *cls = static_cast<SemaClassType *>(node);
                LspPosition start = toPosition(cls->getAST().getLocation());
                // Estimate end from the last method's body.
                int endLine = start.line;
                for (auto it = cls->getMethods().begin();
                         it != cls->getMethods().end(); ++it) {
                    SemaClassMethod *m = it->getValue();
                    if (m->getAST().getBody() &&
                        !m->getAST().getBody()->getContent().empty()) {
                        LspPosition mp = toPosition(
                            m->getAST().getBody()->getContent().back()->getLocation());
                        if (mp.line > endLine) endLine = mp.line;
                    }
                }
                if (endLine > start.line)
                    out.push_back({start.line, endLine, ""});
                // Fold each method body.
                for (auto it = cls->getMethods().begin();
                         it != cls->getMethods().end(); ++it) {
                    SemaClassMethod *m = it->getValue();
                    LspPosition ms = toPosition(m->getAST().getLocation());
                    ASTBlockStmt *body = m->getAST().getBody();
                    if (body && !body->getContent().empty()) {
                        LspPosition ml = toPosition(
                            body->getContent().back()->getLocation());
                        if (ml.line > ms.line)
                            out.push_back({ms.line, ml.line, ""});
                    }
                }
                break;
            }
            default: break;
            }
        }
    }
    return out;
}

// ── Inlay hints ───────────────────────────────────────────────────────────────

// Map SymbolKind → LSP semantic token type index (matches legend in onInitialize).
static int symbolKindToTokenType(SymbolKind k) {
    switch (k) {
    case SymbolKind::NAMESPACE:  return 0;
    case SymbolKind::CLASS:      return 1;
    case SymbolKind::FUNCTION:   return 2;
    case SymbolKind::LOCAL_VAR:  return 3;
    case SymbolKind::PARAM:      return 4;
    case SymbolKind::ATTRIBUTE:  return 5;
    case SymbolKind::ENUM_ENTRY: return 6;
    case SymbolKind::TYPE:       return 7;
    default:                     return -1;  // skip
    }
}

void LspAnalyzer::collectHintsExpr(ASTExpr *expr, LspRange range,
                                    std::vector<LspInlayHint> &out) const {
    if (!expr) return;

    if (expr->getExprKind() == ASTExprKind::EXPR_CALL) {
        auto *call = static_cast<ASTCall *>(expr);
        Symbol *sym = call->getSymbol();
        if (sym && sym->getKind() == SymbolKind::FUNCTION) {
            auto *fn = static_cast<SemaFunctionBase *>(sym->getRef());
            const auto &params = fn->getParams();
            const auto &args   = call->getArgs();
            // Show hints when there are ≥1 params and the call is in range.
            for (size_t i = 0; i < args.size() && i < params.size(); ++i) {
                ASTExpr *argExpr = args[i]->getExpr();
                if (!argExpr) continue;
                LspPosition argPos = toPosition(argExpr->getLocation());
                // Skip if outside the requested range.
                if (argPos.line < range.start.line || argPos.line > range.end.line)
                    continue;
                SemaParam *p = params[i];
                if (!p->getAST()) continue;
                std::string name = p->getAST()->getName().str();
                if (name.empty()) continue;
                out.push_back({argPos, name + ":", 2});
                // Recurse into the argument expression.
                collectHintsExpr(argExpr, range, out);
            }
        }
        // Recurse into receiver and remaining sub-expressions.
        if (auto *parent = expr->getParent())
            collectHintsExpr(parent, range, out);
        return;
    }

    if (expr->getExprKind() == ASTExprKind::EXPR_BINARY) {
        auto *bin = static_cast<ASTBinary *>(expr);
        collectHintsExpr(bin->getLeftExpr(),  range, out);
        collectHintsExpr(bin->getRightExpr(), range, out);
    }
}

void LspAnalyzer::collectHintsBlock(ASTBlockStmt *block, LspRange range,
                                     std::vector<LspInlayHint> &out) const {
    if (!block) return;
    for (auto *stmt : block->getContent()) {
        if (!stmt) continue;
        switch (stmt->getStmtKind()) {
        case ASTStmtKind::STMT_EXPR:
            collectHintsExpr(static_cast<ASTExprStmt *>(stmt)->getExpr(), range, out);
            break;
        case ASTStmtKind::STMT_DECL:
            collectHintsExpr(static_cast<ASTDeclStmt *>(stmt)->getExpr(), range, out);
            break;
        case ASTStmtKind::STMT_BLOCK:
            collectHintsBlock(static_cast<ASTBlockStmt *>(stmt), range, out);
            break;
        case ASTStmtKind::STMT_RULE:
        case ASTStmtKind::STMT_IF:
        case ASTStmtKind::STMT_SWITCH: {
            auto *rule = static_cast<ASTRuleStmt *>(stmt);
            collectHintsExpr(rule->getExpr(), range, out);
            collectHintsBlock(static_cast<ASTBlockStmt *>(rule->getStmt()), range, out);
            break;
        }
        default: break;
        }
    }
}

std::vector<LspInlayHint>
LspAnalyzer::getInlayHints(const std::string &file, LspRange range) {
    std::vector<LspInlayHint> out;
    if (!frontend_) return out;

    for (SemaModule *mod : frontend_->getSemaModules()) {
        const std::string &mf = mod->getAST().getName();
        if (!mf.empty() && mf != file) continue;

        for (SemaNode *node : mod->getNodes()) {
            switch (node->getKind()) {
            case SemaKind::FUNCTION: {
                auto *fn = static_cast<SemaFunction *>(node);
                if (fn->getAST().getBody())
                    collectHintsBlock(fn->getAST().getBody(), range, out);
                break;
            }
            case SemaKind::TYPE_CLASS: {
                auto *cls = static_cast<SemaClassType *>(node);
                for (auto it = cls->getMethods().begin();
                         it != cls->getMethods().end(); ++it) {
                    SemaClassMethod *method = it->getValue();
                    if (method->getAST().getBody())
                        collectHintsBlock(method->getAST().getBody(), range, out);
                }
                break;
            }
            default: break;
            }
        }
    }
    return out;
}

// ── Semantic tokens ───────────────────────────────────────────────────────────

void LspAnalyzer::collectTokensExpr(ASTExpr *expr,
                                     std::vector<SemanticToken> &out) const {
    if (!expr) return;

    auto addToken = [&](LspPosition pos, int nameLen, SymbolKind kind) {
        int tt = symbolKindToTokenType(kind);
        if (tt >= 0 && nameLen > 0)
            out.push_back({pos.line, pos.character, nameLen, tt});
    };

    LspPosition epos = toPosition(expr->getLocation());

    switch (expr->getExprKind()) {
    case ASTExprKind::EXPR_IDENTIFIER: {
        auto *id = static_cast<ASTIdentifier *>(expr);
        if (id->getSymbol())
            addToken(epos, (int)id->getName().size(), id->getSymbol()->getKind());
        break;
    }
    case ASTExprKind::EXPR_MEMBER: {
        auto *mem = static_cast<ASTMember *>(expr);
        if (mem->getSymbol())
            addToken(epos, (int)mem->getName().size(), mem->getSymbol()->getKind());
        if (auto *p = expr->getParent()) collectTokensExpr(p, out);
        break;
    }
    case ASTExprKind::EXPR_CALL: {
        auto *call = static_cast<ASTCall *>(expr);
        if (call->getSymbol())
            addToken(epos, (int)call->getName().size(), call->getSymbol()->getKind());
        if (auto *p = expr->getParent()) collectTokensExpr(p, out);
        for (auto *arg : call->getArgs())
            collectTokensExpr(arg->getExpr(), out);
        break;
    }
    case ASTExprKind::EXPR_BINARY: {
        auto *bin = static_cast<ASTBinary *>(expr);
        collectTokensExpr(bin->getLeftExpr(),  out);
        collectTokensExpr(bin->getRightExpr(), out);
        break;
    }
    default: break;
    }
}

void LspAnalyzer::collectTokensBlock(ASTBlockStmt *block,
                                      std::vector<SemanticToken> &out) const {
    if (!block) return;
    for (auto *stmt : block->getContent()) {
        if (!stmt) continue;
        switch (stmt->getStmtKind()) {
        case ASTStmtKind::STMT_EXPR:
            collectTokensExpr(static_cast<ASTExprStmt *>(stmt)->getExpr(), out);
            break;
        case ASTStmtKind::STMT_DECL:
            collectTokensExpr(static_cast<ASTDeclStmt *>(stmt)->getExpr(), out);
            break;
        case ASTStmtKind::STMT_BLOCK:
            collectTokensBlock(static_cast<ASTBlockStmt *>(stmt), out);
            break;
        case ASTStmtKind::STMT_RULE:
        case ASTStmtKind::STMT_IF:
        case ASTStmtKind::STMT_SWITCH: {
            auto *rule = static_cast<ASTRuleStmt *>(stmt);
            collectTokensExpr(rule->getExpr(), out);
            if (rule->getStmt())
                collectTokensBlock(static_cast<ASTBlockStmt *>(rule->getStmt()), out);
            break;
        }
        default: break;
        }
    }
}

llvm::json::Array LspAnalyzer::getSemanticTokens(const std::string &file) {
    if (!frontend_) return {};

    std::vector<SemanticToken> tokens;
    for (SemaModule *mod : frontend_->getSemaModules()) {
        const std::string &mf = mod->getAST().getName();
        if (!mf.empty() && mf != file) continue;

        for (SemaNode *node : mod->getNodes()) {
            switch (node->getKind()) {
            case SemaKind::FUNCTION: {
                auto *fn = static_cast<SemaFunction *>(node);
                if (fn->getAST().getBody())
                    collectTokensBlock(fn->getAST().getBody(), tokens);
                break;
            }
            case SemaKind::TYPE_CLASS: {
                auto *cls = static_cast<SemaClassType *>(node);
                for (auto it = cls->getMethods().begin();
                         it != cls->getMethods().end(); ++it) {
                    SemaClassMethod *method = it->getValue();
                    if (method->getAST().getBody())
                        collectTokensBlock(method->getAST().getBody(), tokens);
                }
                break;
            }
            default: break;
            }
        }
    }

    // Sort by position (required for delta encoding).
    std::sort(tokens.begin(), tokens.end(),
              [](const SemanticToken &a, const SemanticToken &b) {
                  return a.line != b.line ? a.line < b.line
                                          : a.character < b.character;
              });

    // Delta-encode into flat integer array.
    llvm::json::Array data;
    int prevLine = 0, prevChar = 0;
    for (const auto &t : tokens) {
        int dLine = t.line - prevLine;
        int dChar = (dLine == 0) ? t.character - prevChar : t.character;
        data.push_back(dLine);
        data.push_back(dChar);
        data.push_back(t.length);
        data.push_back(t.tokenType);
        data.push_back(0);  // modifiers
        prevLine = t.line;
        prevChar = t.character;
    }
    return data;
}

// ── Workspace symbols ─────────────────────────────────────────────────────────

std::vector<LspWorkspaceSymbol>
LspAnalyzer::getWorkspaceSymbols(const std::string &query) {
    std::vector<LspWorkspaceSymbol> result;
    if (!frontend_) return result;

    // Simple case-insensitive substring match helper.
    auto matches = [&](const std::string &name) -> bool {
        if (query.empty()) return true;
        std::string nameLow = name, qLow = query;
        for (auto &c : nameLow) c = (char)std::tolower((unsigned char)c);
        for (auto &c : qLow)   c = (char)std::tolower((unsigned char)c);
        return nameLow.find(qLow) != std::string::npos;
    };

    for (SemaModule *mod : frontend_->getSemaModules()) {
        const std::string &mf = mod->getAST().getName();
        std::string uri = mf.empty() ? "" : pathToFileUri(mf);

        for (SemaNode *node : mod->getNodes()) {
            switch (node->getKind()) {
            case SemaKind::FUNCTION: {
                auto *fn = static_cast<SemaFunction *>(node);
                std::string name = fn->getAST().getName().str();
                if (matches(name)) {
                    LspPosition p = toPosition(fn->getAST().getLocation());
                    result.push_back({name, LspSymbolKind::Function, {uri, {p, p}}, ""});
                }
                break;
            }
            case SemaKind::TYPE_CLASS: {
                auto *cls = static_cast<SemaClassType *>(node);
                std::string name = cls->getAST().getName().str();
                LspSymbolKind kind =
                    cls->getClassKind() == SemaClassKind::STRUCT    ? LspSymbolKind::Struct
                  : cls->getClassKind() == SemaClassKind::INTERFACE  ? LspSymbolKind::Interface
                                                                     : LspSymbolKind::Class;
                if (matches(name)) {
                    LspPosition p = toPosition(cls->getAST().getLocation());
                    result.push_back({name, kind, {uri, {p, p}}, ""});
                }
                // Also expose methods within the class.
                for (auto it = cls->getMethods().begin();
                         it != cls->getMethods().end(); ++it) {
                    SemaClassMethod *method = it->getValue();
                    std::string mname = method->getAST().getName().str();
                    if (matches(mname)) {
                        LspPosition mp = toPosition(method->getAST().getLocation());
                        result.push_back({mname, LspSymbolKind::Method, {uri, {mp, mp}}, name});
                    }
                }
                break;
            }
            default: break;
            }
            if (result.size() >= 100) return result;  // cap for performance
        }
    }
    return result;
}

} // namespace lsp
} // namespace fly
