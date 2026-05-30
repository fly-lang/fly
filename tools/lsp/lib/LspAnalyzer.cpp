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

#include <llvm/Support/TargetSelect.h>

using namespace fly;
using namespace fly::lsp;

// ── Compile ───────────────────────────────────────────────────────────────────

std::vector<LspDiagnostic> LspAnalyzer::compile(
        const std::vector<std::string> &files) {
    // Init LLVM targets once
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

    frontend_ = std::make_unique<Frontend>(ci);
    frontend_->Execute();

    // Diagnostics are currently printed to stderr by the engine's default
    // consumer. To surface them as LspDiagnostic we would install a custom
    // DiagnosticConsumer before Execute(); that is left as future work.
    // The VS Code extension's existing subprocess+JSON path covers this
    // until an in-process consumer is wired here.
    return {};
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
                    SemaClassMethod *method = it->second;
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
