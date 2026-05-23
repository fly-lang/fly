//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTExprStmt.cpp - AST expression statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTExprStmt.h"
#include "AST/ASTExpr.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTExprStmt::ASTExprStmt(const SourceLocation &Loc) :
            ASTStmt(Loc, ASTStmtKind::STMT_EXPR) {

}

void ASTExprStmt::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTExprStmt::ASTExprStmt(const SourceLocation &Loc, ASTStmtKind Kind) :
            ASTStmt(Loc, Kind) {

}

ASTExpr *ASTExprStmt::getExpr() const {
    return Expr;
}

void ASTExprStmt::setExpr(ASTExpr *Expr) {
	this->Expr = Expr;
}

std::string ASTExprStmt::str() const {
    return Logger("ASTExprStmt").
	Attr("Location", getLocation()).
 Attr("Kind", static_cast<size_t>(getKind())).
           Attr("Expr", Expr).
           End();
}