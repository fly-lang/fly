//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTCastExpr.cpp - AST Cast Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTCast.h"

#include "Sema/SemaCast.h"

#include <AST/ASTType.h>
#include <AST/ASTVisitor.h>

using namespace fly;

ASTCast::ASTCast(ASTExpr *Expr, ASTType *Cast) : ASTExpr(Cast->getLocation(), ASTExprKind::EXPR_CAST),
	Expr(Expr), Cast(Cast) {
}

void ASTCast::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTExpr * ASTCast::getExpr() const {
	return Expr;
}

ASTType * ASTCast::getToType() const {
	return Cast;
}

SemaExpr *ASTCast::getSema() const {
	return Sema;
}

void ASTCast::setSema(SemaCast *Sema) {
	this->Sema = Sema;
}

std::string ASTCast::str() const {
	return ASTExpr::str();
}
