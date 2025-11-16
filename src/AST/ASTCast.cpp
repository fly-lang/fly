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

ASTCast::ASTCast(ASTExpr *Expr, ASTType *Type) : ASTExpr(Type->getLocation(), ASTExprKind::EXPR_CAST),
	Expr(Expr), Type(Type) {
}

ASTExpr * ASTCast::getExpr() const {
	return Expr;
}

ASTType * ASTCast::getType() const {
	return Type;
}

std::string ASTCast::str() const {
	return ASTExpr::str();
}
