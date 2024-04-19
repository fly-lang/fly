//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTLocalVar.cpp - AST Local Var Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFailStmt.h"

using namespace fly;

ASTFailStmt::ASTFailStmt(ASTStmt *Parent, const SourceLocation &Loc) :
        ASTStmt(Parent, Loc, ASTStmtKind::STMT_FAIL) {

}

ASTExpr *ASTFailStmt::getExpr() const {
    return Expr;
}

void ASTFailStmt::setExpr(fly::ASTExpr *E) {
    Expr = E;
}

std::string ASTFailStmt::str() const {
    return Logger("ASTFailStmt").
            Super(ASTStmt::str()).
            Attr("Expr", Expr).
            End();
}
