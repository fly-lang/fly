//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTExprStmt.cpp - Expression Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTExprStmt.h"
#include "AST/ASTExpr.h"

using namespace fly;

ASTExprStmt::ASTExprStmt(const SourceLocation &Loc) : ASTStmt(Loc) {

}

StmtKind ASTExprStmt::getKind() const {
    return STMT_EXPR;
}

ASTExpr *ASTExprStmt::getExpr() const {
    return Expr;
}

std::string ASTExprStmt::str() const {
    return "{ Expr=" + (Expr ? Expr->str() : "") + " }";
}