//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTDeclStmt.cpp - AST Decl Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTDeclStmt.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTDeclStmt::ASTDeclStmt(const SourceLocation &Loc, ASTLocalVar *LocalVar, ASTExpr *Expr) :
	ASTStmt(Loc, ASTStmtKind::STMT_DECL), LocalVar(LocalVar), Expr(Expr)  {

}

void ASTDeclStmt::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTLocalVar *ASTDeclStmt::getLocalVar() const {
	return LocalVar;
}

ASTExpr *ASTDeclStmt::getExpr() const {
	return Expr;
}

void ASTDeclStmt::setExpr(ASTExpr *Expr) {
	this->Expr = Expr;
}

std::string ASTDeclStmt::str() const {
	return Logger("ASTDeclStmt").
	Attr("Location", getLocation()).
 Attr("Kind", static_cast<size_t>(getKind())).
		   Attr("LocalVar", LocalVar).
		   Attr("Expr", Expr).
		   End();
}
