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

#include <llvm/ADT/SmallString.h>
#include <llvm/Support/TargetSelect.h>

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

std::vector<LspCompletionItem> LspAnalyzer::getCompletions(
        const std::string & /*file*/, int /*line*/, int /*col*/,
        const std::string & /*prefix*/) {
    // TODO: use SymbolTable::lookupInParents at the scope matching position
    return {};
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

} // namespace lsp
} // namespace fly
