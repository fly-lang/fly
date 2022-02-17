//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTBlockStmt.cpp - Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTBlock.h"
#include "AST/ASTContext.h"
#include "AST/ASTNode.h"
#include "AST/ASTFunc.h"
#include "AST/ASTStmt.h"
#include "AST/ASTExpr.h"
#include "AST/ASTLocalVar.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"

using namespace fly;

/**
 * ASTBlock constructor
 * @param Loc
 * @param Top
 * @param Parent
 */
ASTBlock::ASTBlock(const SourceLocation &Loc, ASTFunc *Top, ASTBlock *Parent) :
    ASTStmt(Loc, Top, Parent) {
    FLY_DEBUG("ASTBlock", "ASTBlock");
}

/**
 * ASTBlock constructor
 * @param Loc
 * @param Parent
 */
ASTBlock::ASTBlock(const SourceLocation &Loc, ASTBlock *Parent) : ASTStmt(Loc, Parent->getTop(), Parent) {
        FLY_DEBUG("ASTBlock", "~ASTBlock");
}

/**
 * Get Kind
 * @return StmtKind
 */
StmtKind ASTBlock::getKind() const {
    return Kind;
}

/**
 * Get Content
 * @return the Block's content
 */
const std::vector<ASTStmt *> &ASTBlock::getContent() const {
    return Content;
}

/**
 * Check if Content is empty
 * @return true if empty, or false
 */
bool ASTBlock::isEmpty() const {
    return Content.empty();
}

void ASTBlock::Clear() {
    return Content.clear();
}

/**
 * Get DeclVars
 * @return the Block's declared vars
 */
const llvm::StringMap<ASTLocalVar *> &ASTBlock::getDeclVars() const {
    return DeclVars;
}

/**
 * Add ExprStmt to Content
 * @param ExprStmt
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddExprStmt(ASTExprStmt *ExprStmt) {
    FLY_DEBUG_MESSAGE("ASTBlock", "AddExprStmt", "ExprStmt=" << ExprStmt->str());
    if (ASTResolver::ResolveExpr(this, ExprStmt->getExpr())) {
        Content.push_back(ExprStmt);
        return true;
    }
    return false;
}

/**
 * Add LocalVarRef
 * @param LocalVarRef
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddLocalVarRef(ASTLocalVarRef *LocalVarRef) {
    assert(LocalVarRef->getExpr() && "Expr unset into VarStmt");
    FLY_DEBUG_MESSAGE("ASTBlock", "AddLocalVarRef", "LocalVarRef=" << LocalVarRef->str());

    if (ASTResolver::ResolveExpr(this, LocalVarRef->getExpr()) &&
            (LocalVarRef->getDecl() != nullptr || ASTResolver::ResolveVarRef(this, LocalVarRef))) {
        Content.push_back(LocalVarRef);
        return true;
    }
    return true;
}

bool ASTBlock::RecursiveFindDeclVars(ASTBlock *Block, ASTLocalVar *LocalVar) {
    if (Block->DeclVars.find(LocalVar->getName()) != Block->DeclVars.end()) {
        return true;
    }
    return Block->Parent ? RecursiveFindDeclVars(Block->Parent, LocalVar) : false;
}

/**
 * Add LocalVar
 * @param LocalVar
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddLocalVar(ASTLocalVar *LocalVar) {
    FLY_DEBUG_MESSAGE("ASTBlock", "AddLocalVar", "LocalVar=" << LocalVar->str());
    bool Result = true;
    if (LocalVar->getExpr()) {
        Result = ASTResolver::ResolveExpr(this, LocalVar->getExpr());

        // Check if this var is already declared
        if (RecursiveFindDeclVars(this, LocalVar)) {
            Top->getNode()->getContext().Diag(LocalVar->getLocation(), diag::err_conflict_vardecl)
                    << LocalVar->getName();
            return false;
        }

        //Set CodeGen
        CodeGenLocalVar *CGLV = new CodeGenLocalVar(Top->getNode()->getCodeGen(), LocalVar);
        LocalVar->setCodeGen(CGLV);
    }

    // Add LocalVar
    DeclVars.insert(std::pair<std::string, ASTLocalVar *>(LocalVar->getName(), LocalVar));
    Content.push_back(LocalVar);
    Top->addDeclVars(LocalVar); //Useful for Alloca into CodeGen

    return Result;
}

/**
 * Add Call
 * @param Call
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddCall(ASTFuncCall *Call) {
    FLY_DEBUG_MESSAGE("ASTBlock", "AddBreak", "Call=" << Call->str());
    ASTFuncCallStmt *CallStmt = new ASTFuncCallStmt(Call->getLocation(), this, Call);
    Content.push_back(CallStmt);
    return Call->getDecl() || Top->getNode()->AddUnrefCall(Call);
}

/**
 * Add Return
 * @param Loc
 * @param Expr
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddReturn(const SourceLocation &Loc, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("ASTBlock", "AddReturn", "Expr=" << (Expr ? Expr->str() : ""));
    bool Success = true;
    if (Expr) {
        Success = ASTResolver::ResolveExpr(this, Expr);
    }
    ASTReturn *Ret = new ASTReturn(Loc, this, Expr);
    Content.push_back(Ret);
    return Success;
}

/**
 *
 * @param Loc
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddBreak(const SourceLocation &Loc) {
    FLY_DEBUG("ASTBlock", "AddBreak");
    Content.push_back(new BreakStmt(Loc, this));
    return true;
}

/**
 *
 * @param Loc
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddContinue(const SourceLocation &Loc) {
    FLY_DEBUG("ASTBlock", "AddContinue");
    Content.push_back(new ContinueStmt(Loc, this));
    return true;
}

/**
 *
 * @param Loc
 * @param Block
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddBlock(const SourceLocation &Loc, ASTBlock *Block) {
    FLY_DEBUG("ASTBlock", "AddBlock");
    Content.push_back(Block);
    return true;
}

/**
 * Write Diagnostics
 * @param Loc
 * @param DiagID
 * @return DiagnosticBuilder
 */
DiagnosticBuilder ASTBlock::Diag(SourceLocation Loc, unsigned int DiagID) {
    return Top->getNode()->getContext().Diag(Loc, DiagID);
}

/**
 * Convert to String
 * @return string info for debugging
 */
std::string ASTBlock::str() const {
    return "{ Kind=" + std::to_string(Kind) +
            ", BlockKind=" + std::to_string(BlockKind) +
            + " }";
}

/**
 * BreakStmt constructor
 * @param Loc
 * @param Parent
 */
BreakStmt::BreakStmt(const SourceLocation &Loc, ASTBlock *Parent) : ASTStmt(Loc, Parent) {

}
/**
 * Convert to String
 * @return string info for debugging
 */
std::string BreakStmt::str() const {
    return "{ Kind=" + std::to_string(Kind) + " }";
}


/**
 * Get the Kind of Stmt
 * @return the StmtKind
 */
StmtKind BreakStmt::getKind() const {
    return Kind;
}

/**
 * ContinueStmt constructor
 * @param Loc
 * @param Parent
 */
ContinueStmt::ContinueStmt(const SourceLocation &Loc, ASTBlock *Parent) : ASTStmt(Loc, Parent) {

}

/**
 * Convert to String
 * @return string info for debugging
 */
std::string ContinueStmt::str() const {
    return "{ Kind=" + std::to_string(Kind) + " }";
}

/**
 * Get the Kind of Stmt
 * @return the StmtKind
 */
StmtKind ContinueStmt::getKind() const {
    return Kind;
}
