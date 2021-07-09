//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/BlockStmt.cpp - Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "Basic/DiagnosticIDs.h"
#include "AST/BlockStmt.h"
#include "AST/ASTNode.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTContext.h"
#include "AST/FuncDecl.h"
#include "AST/Stmt.h"
#include "AST/VarDeclStmt.h"

using namespace fly;

BlockStmt::BlockStmt(const SourceLocation &Loc, FuncDecl *Top, BlockStmt *Parent) :
    Stmt(Loc, Top, Parent) {

}

BlockStmt::BlockStmt(const SourceLocation &Loc, BlockStmt *Parent) : Stmt(Loc, Parent->getTop(), Parent) {

}

StmtKind BlockStmt::getKind() const {
    return Kind;
}

const std::vector<Stmt *> &BlockStmt::getContent() const {
    return Content;
}

bool BlockStmt::isEmpty() const {
    return Content.empty();
}

const llvm::StringMap<VarDeclStmt *> &BlockStmt::getDeclVars() const {
    return DeclVars;
}

VarDeclStmt *BlockStmt::findVarDecl(const BlockStmt *Block, VarRef *Var) {
    const auto &It = Block->DeclVars.find(Var->getName());
    if (It != Block->DeclVars.end()) { // Search into this Block
        return It->getValue();
    } else if (Block->Parent) { // Traverse Parent Block to find the right VarDeclStmt
        return findVarDecl(Block->getParent(), Var);
    }
    return nullptr;
}

bool BlockStmt::ResolveVarRef(VarRef* Var) {
    VarDecl *VDecl;
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
            // Check if var declaration happen before this var use
            if (((VarDeclStmt *) VDecl)->getOrder() < Var->getOrder()) {
                Diag(Var->getLocation(), diag::err_vardecl_notfound) << Var->getName();
                return false;
            }

            // Resolve with VarDecl
            Var->setDecl(VDecl);
        }
    } else { // Var with NameSpace refers to a GlobalVar
        // Try to resolve here or will be resolved after by addRefGlobalVar() into FuncDecl Finalize()
        const auto &It = Top->getNameSpace()->getGlobalVars().find(Var->getNameSpace());
        if (It != Top->getNameSpace()->getGlobalVars().end()) {
            VDecl = (VarDecl *) It->getValue();
            Var->setDecl(VDecl); // Resolve with VarDecl
        } else {
            Top->addUnRefGlobalVar(Var); // Push into Content but need resolve into Finalize()
        }
    }
    return true;
}

bool BlockStmt::ResolveExpr(Expr *E) {
    switch (E->getKind()) {
        case EXPR_REF_VAR: {
            VarRef *Var = static_cast<VarRefExpr *>(E)->getVarRef();
            return Var->getDecl() == nullptr ? ResolveVarRef(Var) : true;
        }
        case EXPR_REF_FUNC: {
            FuncCall *Call = static_cast<FuncCallExpr *>(E)->getCall();
            return Top->addUnRefCall(Call);
        }
        case EXPR_GROUP: {
            bool Result = true;
            for (auto &GE : static_cast<GroupExpr *>(E)->getGroup()) {
                Result &= ResolveExpr(GE);
            }
            return Result;
        }
        case EXPR_VALUE:
            return true;
        case EXPR_OPERATOR:
            return true;
    }
    assert("Unknown Expr Kind");
}

bool BlockStmt::addVar(VarStmt *Var) {
    assert(Var->getExpr() && "Expr mandatory into VarStmt");

    if (ResolveExpr(Var->getExpr())) {

        if (Var->getDecl() == nullptr) {
            ResolveVarRef(Var);
        }
        Var->setOrder(Order++);
        Content.push_back(Var);
        return true;
    }
    return Var->getDecl() != nullptr;
}

bool BlockStmt::addVar(VarDeclStmt *Var) {
    bool Result = false;
    if (Var->getExpr()) {
        Result &= ResolveExpr(Var->getExpr());
    }

    const auto &It = DeclVars.find(Var->getName());
    // Check if this var is already declared
    if (It != DeclVars.end()) {
        assert("Var already declared");
        Result = false;
    }
    DeclVars.insert(std::pair<StringRef, VarDeclStmt *>(Var->getName(), Var));
    Var->setOrder(Order++);
    Content.push_back(Var);
    return Result;
}

bool BlockStmt::addCall(FuncCall *Call) {
    FuncCallStmt *CallStmt = new FuncCallStmt(Call->getLocation(), this, Call);
    Content.push_back(CallStmt);
    return Top->addUnRefCall(Call);
}

ReturnStmt *BlockStmt::addReturn(const SourceLocation &Loc, GroupExpr *E) {
    assert(E && "Return Expr is mandatory");
    ReturnStmt *Ret;
    if (ResolveExpr(E)) {
        Ret = new ReturnStmt(Loc, this, E);
        Content.push_back(Ret);
    }
    return Ret;
}

DiagnosticBuilder BlockStmt::Diag(SourceLocation Loc, unsigned int DiagID) {
    return Top->getNode()->getContext().Diag(Loc, DiagID);
}

ConditionBlockStmt::ConditionBlockStmt(const SourceLocation &Loc, BlockStmt *Parent) : BlockStmt(Loc, Parent) {}

LoopBlockStmt::LoopBlockStmt(const SourceLocation &Loc, BlockStmt *Parent) : BlockStmt(Loc, Parent) {

}

BreakStmt::BreakStmt(const SourceLocation &Loc, BlockStmt *CurrStmt) : Stmt(Loc, CurrStmt) {

}

StmtKind BreakStmt::getKind() const {
    return Kind;
}

ContinueStmt::ContinueStmt(const SourceLocation &Loc, BlockStmt *CurrStmt) : Stmt(Loc, CurrStmt) {

}

StmtKind ContinueStmt::getKind() const {
    return Kind;
}
