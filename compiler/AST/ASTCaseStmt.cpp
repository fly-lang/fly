//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTCaseStmt.cpp - AST case statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTCaseStmt.h"
#include "AST/ASTExpr.h"
#include "AST/ASTVisitor.h"
#include "Basic/Logger.h"

using namespace fly;

ASTCaseStmt::ASTCaseStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_CASE) {}

void ASTCaseStmt::accept(ASTVisitor &Visitor) {
    Visitor.visit(*this);
}

ASTExpr *ASTCaseStmt::getExpr() const { return Expr; }

ASTStmt *ASTCaseStmt::getStmt() const { return Stmt; }

std::string ASTCaseStmt::str() const {
    return Logger("ASTCaseStmt")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("Expr", Expr)
        .Attr("Stmt", Stmt)
        .End();
}
