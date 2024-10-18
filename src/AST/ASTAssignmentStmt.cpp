//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTVarStmt.cpp - AST Var Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTAssignmentStmt.h"
#include "AST/ASTBlockStmt.h"

using namespace fly;

ASTAssignmentStmt::ASTAssignmentStmt(const SourceLocation &Loc, ASTVarRef *VarRef, ASTAssignOperatorKind AssignOperatorKind) :
        ASTStmt(Loc, ASTStmtKind::STMT_ASSIGN), VarRef(VarRef), Kind(AssignOperatorKind) {

}

ASTVarRef *ASTAssignmentStmt::getVarRef() const {
    return VarRef;
}

ASTAssignOperatorKind ASTAssignmentStmt::getKind1() const {
    return Kind;
}

ASTExpr *ASTAssignmentStmt::getExpr() const {
    return Expr;
}

void ASTAssignmentStmt::setExpr(fly::ASTExpr *E) {
    Expr = E;
}

std::string ASTAssignmentStmt::str() const {
    return Logger("ASTVarAssign").
            Super(ASTStmt::str()).
            Attr("VarRef", VarRef).
            Attr("Kind", (uint64_t) Kind).
            Attr("ExprStmt", ASTStmt::str()).
            End();
}
