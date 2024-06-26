//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTLocalVar.cpp - AST Local Var Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTVarStmt.h"
#include "AST/ASTBlock.h"
#include "AST/ASTVar.h"
#include "AST/ASTVarRef.h"

using namespace fly;

ASTVarStmt::ASTVarStmt(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *VarRef) :
        ASTStmt(Parent, Loc, ASTStmtKind::STMT_VAR), VarRef(VarRef) {

}

std::string ASTVarStmt::str() const {
    return Logger("ASTVarAssign").
            Super(ASTStmt::str()).
            Attr("VarRef", VarRef).
            Attr("ExprStmt", ASTStmt::str()).
            End();
}

ASTVarRef *ASTVarStmt::getVarRef() const {
    return VarRef;
}

ASTExpr *ASTVarStmt::getExpr() const {
    return Expr;
}

void ASTVarStmt::setExpr(fly::ASTExpr *E) {
    Expr = E;
}

ASTBlock *ASTVarStmt::getBlock() const {
    return Block;
}
