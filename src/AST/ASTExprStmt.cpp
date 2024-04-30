//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTExprStmt.cpp - AST Expression Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTExprStmt.h"

using namespace fly;

ASTExprStmt::ASTExprStmt(const SourceLocation &Loc) :
            ASTStmt(Loc, ASTStmtKind::STMT_EXPR) {

}

ASTExprStmt::ASTExprStmt(const SourceLocation &Loc, ASTStmtKind Kind) :
            ASTStmt(Loc, Kind) {

}

ASTExpr *ASTExprStmt::getExpr() const {
    return Expr;
}

std::string ASTExprStmt::str() const {
    return Logger("ASTExprStmt").
           Super(ASTStmt::str()).
           Attr("Expr", Expr).
           End();
}