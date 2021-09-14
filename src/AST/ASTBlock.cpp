//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTBlockStmt.cpp - Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "Basic/DiagnosticIDs.h"
#include "AST/ASTBlock.h"
#include "AST/ASTNode.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTContext.h"
#include "AST/ASTFunc.h"
#include "AST/ASTStmt.h"
#include "AST/ASTLocalVar.h"

using namespace fly;

/**
 * ASTBlock constructor
 * @param Loc
 * @param Top
 * @param Parent
 */
ASTBlock::ASTBlock(const SourceLocation &Loc, ASTFunc *Top, ASTBlock *Parent) :
    ASTStmt(Loc, Top, Parent) {

}

/**
 * ASTBlock constructor
 * @param Loc
 * @param Parent
 */
ASTBlock::ASTBlock(const SourceLocation &Loc, ASTBlock *Parent) : ASTStmt(Loc, Parent->getTop(), Parent) {

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
 * Search a VarRef into declared Block's vars
 * If found set LocalVar
 * @param Block
 * @param LocalVar
 * @param VarRef
 * @return the found LocalVar
 */
ASTLocalVar *ASTBlock::FindVarDecl(const ASTBlock *Block, ASTVarRef *VarRef) {
    const auto &It = Block->DeclVars.find(VarRef->getName());
    if (It != Block->DeclVars.end()) { // Search into this Block
        return It->getValue();
    } else if (Block->Parent) { // Traverse Parent Block to find the right VarDeclStmt
        return FindVarDecl(Block->getParent(), VarRef);
    }
    return nullptr;
}

/**
 * Resolve a VarRef with its declaration
 * @param VarRef
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::ResolveVarRef(ASTVarRef* VarRef) {
    // Check if var is not a GlobalVar
    if (VarRef->getNameSpace().empty()) {

        // Search into parameters
        for (auto &Param : Top->getHeader()->getParams()) {
            if (VarRef->getName().equals(Param->getName())) {
                // Resolve with Param
                VarRef->setDecl(Param);
                break;
            }
        }

        // If VarRef is not resolved with parameters, search into declaration
        if (VarRef->getDecl() == nullptr) {
            // Search recursively into current Block or in one of Parents
            ASTLocalVar *LocalVar = FindVarDecl(this, VarRef);
            // Check if var declaration var is resolved
            if (LocalVar != nullptr) {
                VarRef->setDecl(LocalVar); // Resolved
            } else {
                Top->addUnRefGlobalVar(VarRef); // Resolve Later by searching into Node GlobalVars
            }
        }
    } else {
        // Resolve Later by searching into NameSpace GlobalVars
        Top->addNSUnRefGlobalVar(VarRef); // Push into Content but need resolve after
    }
    return true;
}

/**
 * Resolve Expr contents
 * @param Expr
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::ResolveExpr(ASTExpr *Expr) {
    switch (Expr->getKind()) {
        case EXPR_REF_VAR: {
            ASTVarRef *Var = static_cast<ASTVarRefExpr *>(Expr)->getVarRef();
            return Var->getDecl() == nullptr ? ResolveVarRef(Var) : true;
        }
        case EXPR_REF_FUNC: {
            ASTFuncCall *Call = static_cast<ASTFuncCallExpr *>(Expr)->getCall();
            Top->addUnRefCall(Call);
            return true;
        }
        case EXPR_GROUP: {
            bool Result = true;
            for (auto &GE : static_cast<ASTGroupExpr *>(Expr)->getGroup()) {
                Result &= ResolveExpr(GE);
            }
            return Result;
        }
        case EXPR_VALUE:
            return true;
        case EXPR_OPERATOR:
            return true;
    }

    assert(0 && "Invalid ASTExprKind");
}

/**
 * Add ExprStmt to Content
 * @param ExprStmt
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddExprStmt(ASTExprStmt *ExprStmt) {
    if (ResolveExpr(ExprStmt->getExpr())) {
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
bool ASTBlock::AddVarRef(ASTLocalVarRef *LocalVarRef) {
    assert(LocalVarRef->getExpr() && "Expr unset into VarStmt");

    if (ResolveExpr(LocalVarRef->getExpr())) {
        Content.push_back(LocalVarRef);
        if (LocalVarRef->getDecl() == nullptr) {
            return ResolveVarRef(LocalVarRef);
        }
    }
    return true;
}

/**
 * Add LocalVar
 * @param LocalVar
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddVar(ASTLocalVar *LocalVar) {
    bool Result = true;
    if (LocalVar->getExpr()) {
        Result &= ResolveExpr(LocalVar->getExpr());
    }

    const auto &It = DeclVars.find(LocalVar->getName());
    // Check if this var is already declared
    if (It != DeclVars.end()) {
        Top->getNode()->getContext().Diag(LocalVar->getLocation(), diag::err_conflict_vardecl) << LocalVar->getName();
        return false;
    }
    DeclVars.insert(std::pair<StringRef, ASTLocalVar *>(LocalVar->getName(), LocalVar));
    Content.push_back(LocalVar);
    Top->addDeclVars(LocalVar);

    //Set CodeGen
    CodeGenLocalVar *CGV = new CodeGenLocalVar(Top->getNode()->getCodeGen(), LocalVar);
    LocalVar->setCodeGen(CGV);

    return Result;
}

/**
 * Add Call
 * @param Call
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddCall(ASTFuncCall *Call) {
    ASTFuncCallStmt *CallStmt = new ASTFuncCallStmt(Call->getLocation(), this, Call);
    Content.push_back(CallStmt);
    return Top->addUnRefCall(Call);
}

/**
 * Add Return
 * @param Loc
 * @param Expr
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddReturn(const SourceLocation &Loc, ASTExpr *Expr) {
    bool Success = true;
    if (Expr != nullptr) {
        Success &= ResolveExpr(Expr);
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
    Content.push_back(new BreakStmt(Loc, this));
    return true;
}

/**
 *
 * @param Loc
 * @return true if no error occurs, otherwise false
 */
bool ASTBlock::AddContinue(const SourceLocation &Loc) {
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
 * BreakStmt constructor
 * @param Loc
 * @param Parent
 */
BreakStmt::BreakStmt(const SourceLocation &Loc, ASTBlock *Parent) : ASTStmt(Loc, Parent) {

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
 * Get the Kind of Stmt
 * @return the StmtKind
 */
StmtKind ContinueStmt::getKind() const {
    return Kind;
}
