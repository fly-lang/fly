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
            if (VDecl) {
                Var->setDecl(VDecl); // Resolved
            } else {
                Top->addUnRefGlobalVar(Var); // Resolve Later by searching into Node GlobalVars
            }
        }
    } else {
        // Resolve Later by searching into NameSpace GlobalVars
        Top->addNSUnRefGlobalVar(Var); // Push into Content but need resolve after
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
            Top->addUnRefCall(Call);
            return true;
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

bool ASTBlock::addExprStmt(ASTExprStmt *ExprStmt) {
    if (ResolveExpr(ExprStmt->getExpr())) {
        Content.push_back(ExprStmt);
        return true;
    }
    return false;
}

bool ASTBlock::addVar(ASTLocalVarRef *Var) {
    assert(Var->getExpr() && "Expr unset into VarStmt");

    if (ResolveExpr(Var->getExpr())) {
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

    const auto &It = DeclVars.find(Var->getName());
    // Check if this var is already declared
    if (It != DeclVars.end()) {
        Top->getNode()->getContext().Diag(Var->getLocation(), diag::err_conflict_vardecl) << Var->getName();
        return false;
    }
    DeclVars.insert(std::pair<StringRef, ASTLocalVar *>(Var->getName(), Var));
    Content.push_back(Var);
    Top->addDeclVars(Var);

    //Set CodeGen
    CodeGenLocalVar *CGV = new CodeGenLocalVar(Top->getNode()->getCodeGen(), Var);
    Var->setCodeGen(CGV);

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

bool ASTBlock::addBlock(const SourceLocation &Loc, ASTBlock *Block) {
    Content.push_back(Block);
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
