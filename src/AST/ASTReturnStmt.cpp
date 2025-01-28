//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTReturnStmt.cpp - AST Return Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTReturnStmt.h"

using namespace fly;

ASTReturnStmt::ASTReturnStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_RETURN) {

}

ASTExpr *ASTReturnStmt::getExpr() const {
    return Expr;
}

std::string ASTReturnStmt::str() const {
    return Logger("ASTReturn").
            Attr("Expr", Expr).
            End();
}
