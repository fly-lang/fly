//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTCastExpr.cpp - AST Cast Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTCastExpr.h"

#include <AST/ASTType.h>

using namespace fly;

ASTCastExpr::ASTCastExpr(ASTExpr *Expr, ASTType *Cast) : ASTExpr(Cast->getLocation(), ASTExprKind::EXPR_CAST),
	Expr(Expr), TypeRef(Cast) {
}

ASTExpr * ASTCastExpr::getExpr() const {
	return Expr;
}

ASTType * ASTCastExpr::getTypeRef() const {
	return TypeRef;
}

std::string ASTCastExpr::str() const {
	return ASTExpr::str();
}
