//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTVarStmt.cpp - AST Var Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTVarStmt.h"

using namespace fly;

ASTVarStmt::ASTVarStmt(const SourceLocation &Loc, ASTVarRef *VarRef, ASTAssignOperatorKind AssignOperatorKind) :
        ASTStmt(Loc, ASTStmtKind::STMT_VAR), VarRef(VarRef), Kind(AssignOperatorKind) {

}

ASTVarRef *ASTVarStmt::getVarRef() const {
    return VarRef;
}

ASTAssignOperatorKind ASTVarStmt::getKind1() const {
    return Kind;
}

ASTExpr *ASTVarStmt::getExpr() const {
    return Expr;
}

void ASTVarStmt::setExpr(fly::ASTExpr *E) {
    Expr = E;
}

std::string ASTVarStmt::str() const {
    return Logger("ASTVarAssign").
            Super(ASTStmt::str()).
            Attr("VarRef", VarRef).
            Attr("Kind", (uint64_t) Kind).
            Attr("ExprStmt", ASTStmt::str()).
            End();
}
