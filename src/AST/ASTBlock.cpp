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
#include "AST/ASTIfBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTForBlock.h"
#include "Sema/Sema.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"
#include <llvm/ADT/StringMap.h>

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
 * Get LocalVars
 * @return the Block's declared vars
 */
const llvm::StringMap<ASTLocalVar *> &ASTBlock::getLocalVars() const {
    return LocalVars;
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
 * Add Local Var
 * @param LocalVar
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddLocalVar(ASTLocalVar *LocalVar) {
    FLY_DEBUG_MESSAGE("ASTBlock", "AddLocalVar", "LocalVar=" << LocalVar->str());

    // Check if LocalVar have an Expression assigned
    if (LocalVar->getExpr()) {
        if (!ASTResolver::ResolveExpr(this, LocalVar->getExpr())) {
            return false;
        }
    } else { // No Expression: add to Undefined Vars, will be removed on AddLocalVarRef()
        UndefVars.insert(std::pair<std::string, ASTLocalVar *>(LocalVar->getName(), LocalVar));
    }

    // Check if this var is already declared
    if (RecursiveFindLocalVars(this, LocalVar)) {
        Top->getNode()->getContext().Diag(LocalVar->getLocation(), diag::err_conflict_vardecl)
                << LocalVar->getName();
        return false;
    }

    //Set CodeGen
    CodeGenLocalVar *CGLV = new CodeGenLocalVar(Top->getNode()->getCodeGen(), LocalVar);
    LocalVar->setCodeGen(CGLV);

    // Add LocalVar
    Content.push_back(LocalVar);
    Top->addLocalVar(LocalVar); //Useful for Alloca into CodeGen
    return LocalVars.insert(std::pair<std::string, ASTLocalVar *>(LocalVar->getName(), LocalVar)).second;
}

/**
 * Add Local Var Reference
 * @param LocalVarRef
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddLocalVarRef(ASTLocalVarRef *LocalVarRef) {
    assert(LocalVarRef->getExpr() && "Expr unset into VarStmt");
    FLY_DEBUG_MESSAGE("ASTBlock", "AddLocalVarRef", "LocalVarRef=" << LocalVarRef->str());

    if ((LocalVarRef->getDecl() != nullptr || ASTResolver::ResolveVarRef(this, LocalVarRef)) &&
            ASTResolver::ResolveExpr(this, LocalVarRef->getExpr())) {

        // The Var is now well-defined: you can remove it from UndefVars
        RemoveUndefVar(LocalVarRef);

        // Add Var to Block Content
        Content.push_back(LocalVarRef);
        return true;
    }

    return false;
}

bool ASTBlock::RecursiveFindLocalVars(ASTBlock *Block, ASTLocalVar *LocalVar) {
    if (Block->LocalVars.find(LocalVar->getName()) != Block->LocalVars.end()) {
        return true;
    }
    return Block->Parent != nullptr && RecursiveFindLocalVars(Block->Parent, LocalVar);
}

bool ASTBlock::HasUndefVar(ASTVarRef *VarRef) {
    return UndefVars.lookup(VarRef->getName());
}

bool ASTBlock::RemoveUndefVar(ASTVarRef *VarRef) {
    if (UndefVars.lookup(VarRef->getName())) {
        return UndefVars.erase(VarRef->getName());
    }
    return false;
}

/**
 * Add Call
 * @param Call
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddCall(ASTFuncCall *Call) {
    FLY_DEBUG_MESSAGE("ASTBlock", "AddBreak", "Call=" << Call->str());
    ASTExprStmt *ExprStmt = new ASTExprStmt(Call->getLocation(), this);
    ExprStmt->setExpr(new ASTFuncCallExpr(Call));
    Content.push_back(ExprStmt);
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
 * Add ASTIfBlock
 * @param Loc
 * @param Expr
 * @return
 */
ASTIfBlock* ASTBlock::AddIfBlock(const SourceLocation &Loc, ASTExpr *Expr) {
    ASTIfBlock *Block = new ASTIfBlock(Loc, this, Expr);
    Content.push_back(Block);
    return Block;
}

/**
 * Add ASTSwitchBlock
 * @param Loc
 * @param Expr
 * @return
 */
ASTSwitchBlock *ASTBlock::AddSwitchBlock(const SourceLocation &Loc, ASTExpr *Expr) {
    ASTSwitchBlock *Block = new ASTSwitchBlock(Loc, this, Expr);
    Content.push_back(Block);
    return Block;
}

/**
 * Add ASTWhileBlock
 * @param Loc
 * @param Expr
 * @return
 */
ASTWhileBlock *ASTBlock::AddWhileBlock(const SourceLocation &Loc, ASTExpr *Expr) {
    ASTWhileBlock *Block = new ASTWhileBlock(Loc, this, Expr);
    Content.push_back(Block);
    return Block;
}

/**
 * Add ASTForBlock
 * @param Loc
 * @param Expr
 * @return
 */
ASTForBlock *ASTBlock::AddForBlock(const SourceLocation &Loc) {
    ASTForBlock *Block = new ASTForBlock(Loc, this);
    Content.push_back(Block);
    return Block;
}

/**
 *
 * @return
 */
bool ASTBlock::OnCloseBlock() {
    return false;
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
