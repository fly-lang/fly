//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaTernary.cpp - The Symbolic Table for Ternary Operation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaTernary.h"

#include "AST/ASTOp.h"
#include "Sema/Helper.h"

using namespace fly;

SemaType *SemaTernary::SelectType(SemaExpr *LeftExpr, SemaExpr *RightExpr) {
	// For now, we just return the type of the left expression
	if (LeftExpr->getType()->isInteger() && RightExpr->getType()->isInteger())
		return Helper::SelectIntType(LeftExpr, RightExpr);
	if (LeftExpr->getType()->isFloatingPoint() && RightExpr->getType()->isFloatingPoint())
		return Helper::SelectFloatType(LeftExpr, RightExpr);
	return LeftExpr->getType();
}

SemaTernary::SemaTernary(ASTTernary &AST) :
	SemaExpr(SemaKind::TERNARY, SelectType(AST.getTrueExpr()->getSema(), AST.getFalseExpr()->getSema())),
	AST(AST) {
}

ASTTernary &SemaTernary::getAST() const {
	return AST;
}

