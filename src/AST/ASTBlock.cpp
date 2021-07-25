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

ASTBlock::ASTBlock(const SourceLocation &Loc, ASTFunc *Top, ASTBlock *Parent) :
    ASTStmt(Loc, Top, Parent) {

}

ASTBlock::ASTBlock(const SourceLocation &Loc, ASTBlock *Parent) : ASTStmt(Loc, Parent->getTop(), Parent) {

}

StmtKind ASTBlock::getKind() const {
    return Kind;
}

const std::vector<ASTStmt *> &ASTBlock::getContent() const {
    return Content;
}

bool ASTBlock::isEmpty() const {
    return Content.empty();
}

const llvm::StringMap<ASTLocalVar *> &ASTBlock::getDeclVars() const {
    return DeclVars;
}

ASTLocalVar *ASTBlock::findVarDecl(const ASTBlock *Block, ASTVarRef *Var) {
    const auto &It = Block->DeclVars.find(Var->getName());
    if (It != Block->DeclVars.end()) { // Search into this Block
        return It->getValue();
    } else if (Block->Parent) { // Traverse Parent Block to find the right VarDeclStmt
        return findVarDecl(Block->getParent(), Var);
    }
    return nullptr;
}

bool ASTBlock::ResolveVarRef(ASTVarRef* Var) {
    ASTVar *VDecl;
    // Check if var is not a GlobalVar
    if (Var->getNameSpace().empty()) {

        // Search into parameters
        for (auto &Param : Top->getHeader()->getParams()) {
            if (Var->getName().equals(Param->getName())) {
                // Resolve with Param
                Var->setDecl(Param);
                break;
            }
        }

        // If Var is not resolved with parameters, search into declaration
        if (Var->getDecl() == nullptr) {
            // Search recursively into current Block or in one of Parents
            VDecl = findVarDecl(this, Var);
            // Check if var declaration var is resolved
            if (!VDecl) {
                Diag(Var->getLocation(), diag::err_vardecl_notfound) << Var->getName();
                return false;
            }
            // Check if var declaration happen before this var use // FIXME need or not? TODO add order into Expr
//            if (((VarDeclStmt *) VDecl)->getOrder() > Var->getOrder()) {
//                Diag(Var->getLocation(), diag::err_vardecl_notfound) << Var->getName();
//                return false;
//            }

            // Resolve with VarDecl
            Var->setDecl(VDecl);
        }
    } else { // Var with NameSpace refers to a GlobalVar
        // Try to resolve here or will be resolved after by addRefGlobalVar() into FuncDecl Finalize()
        const auto &It = Top->getNameSpace()->getGlobalVars().find(Var->getNameSpace());
        if (It != Top->getNameSpace()->getGlobalVars().end()) {
            VDecl = (ASTVar *) It->getValue();
            Var->setDecl(VDecl); // Resolve with VarDecl
        } else {
            Top->addUnRefGlobalVar(Var); // Push into Content but need resolve into Finalize()
        }
    }
    return true;
}

bool ASTBlock::ResolveExpr(ASTExpr *E) {
    switch (E->getKind()) {
        case EXPR_REF_VAR: {
            ASTVarRef *Var = static_cast<ASTVarRefExpr *>(E)->getVarRef();
            return Var->getDecl() == nullptr ? ResolveVarRef(Var) : true;
        }
        case EXPR_REF_FUNC: {
            ASTFuncCall *Call = static_cast<ASTFuncCallExpr *>(E)->getCall();
            return Top->addUnRefCall(Call);
        }
        case EXPR_GROUP: {
            bool Result = true;
            for (auto &GE : static_cast<ASTGroupExpr *>(E)->getGroup()) {
                Result &= ResolveExpr(GE);
            }
            return Result;
        }
        case EXPR_VALUE:
            return true;
        case EXPR_OPERATOR:
            return true;
    }

    assert(0 && "Unknown Expr Kind");
}

bool ASTBlock::addVar(ASTLocalVarStmt *Var) {
    assert(Var->getExpr() && "Expr unset into VarStmt");

    if (ResolveExpr(Var->getExpr())) {

        Var->setOrder(Order++);
        Content.push_back(Var);
        if (Var->getDecl() == nullptr) {
            return ResolveVarRef(Var);
        }
    }
    return true;
}

bool ASTBlock::addVar(ASTLocalVar *Var) {
    bool Result = true;
    if (Var->getExpr()) {
        Result &= ResolveExpr(Var->getExpr());
    }
    Var->setOrder(Order++);

    const auto &It = DeclVars.find(Var->getName());
    // Check if this var is already declared
    if (It != DeclVars.end()) {
        Top->getNode()->getContext().Diag(Var->getLocation(), diag::err_conflict_vardecl) << Var->getName();
        return false;
    }
    DeclVars.insert(std::pair<StringRef, ASTLocalVar *>(Var->getName(), Var));
    Var->setOrder(Order++);
    Content.push_back(Var);
    return Result;
}

bool ASTBlock::addCall(ASTFuncCall *Call) {
    ASTFuncCallStmt *CallStmt = new ASTFuncCallStmt(Call->getLocation(), this, Call);
    Content.push_back(CallStmt);
    return Top->addUnRefCall(Call);
}

bool ASTBlock::addReturn(const SourceLocation &Loc, ASTExpr *Expr) {
    bool Success = true;
    if (Expr != nullptr) {
        Success &= ResolveExpr(Expr);
    }
    ASTReturn *Ret = new ASTReturn(Loc, this, Expr);
    Content.push_back(Ret);
    return Success;
}

bool ASTBlock::addBreak(const SourceLocation &Loc) {
    Content.push_back(new BreakStmt(Loc, this));
    return true;
}

bool ASTBlock::addContinue(const SourceLocation &Loc) {
    Content.push_back(new ContinueStmt(Loc, this));
    return true;
}

DiagnosticBuilder ASTBlock::Diag(SourceLocation Loc, unsigned int DiagID) {
    return Top->getNode()->getContext().Diag(Loc, DiagID);
}

ConditionBlockStmt::ConditionBlockStmt(const SourceLocation &Loc, ASTBlock *Parent) : ASTBlock(Loc, Parent) {}

LoopBlockStmt::LoopBlockStmt(const SourceLocation &Loc, ASTBlock *Parent) : ASTBlock(Loc, Parent) {

}

BreakStmt::BreakStmt(const SourceLocation &Loc, ASTBlock *Parent) : ASTStmt(Loc, Parent) {

}

StmtKind BreakStmt::getKind() const {
    return Kind;
}

ContinueStmt::ContinueStmt(const SourceLocation &Loc, ASTBlock *Parent) : ASTStmt(Loc, Parent) {

}

StmtKind ContinueStmt::getKind() const {
    return Kind;
}
