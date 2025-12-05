//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTCastExpr.cpp - AST Cast Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTCast.h"
#include <AST/ASTType.h>

using namespace fly;

ASTCast::ASTCast(ASTExpr *Expr, ASTType *Cast) : ASTExpr(Cast->getLocation(), ASTExprKind::EXPR_CAST),
	Expr(Expr), Cast(Cast) {
}

ASTExpr * ASTCast::getExpr() const {
	return Expr;
}

ASTType * ASTCast::getCast() const {
	return Cast;
}

std::string ASTCast::str() const {
	return ASTExpr::str();
}
