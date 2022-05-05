//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTLocalVar.cpp - AST Local Var Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTVarAssign.h"
#include "AST/ASTVar.h"

using namespace fly;

ASTVarAssign::ASTVarAssign(const SourceLocation &Loc, ASTBlock *Block, ASTVarRef *VarRef, ASTExpr *Expr) :
        ASTStmt(Loc, Block), VarRef(VarRef), Expr(Expr) {

}

StmtKind ASTVarAssign::getKind() const {
    return STMT_VAR_ASSIGN;
}

std::string ASTVarAssign::str() const {
    return "VarRef=" + VarRef->str() +
        ", Expr=" + Expr->str();
}

ASTVarRef *ASTVarAssign::getVarRef() const {
    return VarRef;
}

void ASTVarAssign::setVarRef(ASTVarRef *varRef) {
    VarRef = varRef;
}

ASTExpr *ASTVarAssign::getExpr() const {
    return Expr;
}

void ASTVarAssign::setExpr(ASTExpr *expr) {
    Expr = expr;
}
