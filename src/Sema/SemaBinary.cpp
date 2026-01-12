//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBinary.cpp - The Symbolic Table for Binary Operation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBinary.h"

#include "AST/ASTOp.h"
#include "Sema/Helper.h"

using namespace fly;

SemaType * SemaBinary::SelectType(SemaExpr * LeftExpr, SemaExpr * RightExpr) {
	// For now, we just return the type of the left expression
	if (LeftExpr->getType()->isInteger() && RightExpr->getType()->isInteger())
		return Helper::SelectIntType(LeftExpr, RightExpr);
	if (LeftExpr->getType()->isFloatingPoint() && RightExpr->getType()->isFloatingPoint())
		return Helper::SelectFloatType(LeftExpr, RightExpr);
	return LeftExpr->getType();
}

SemaBinary::SemaBinary(ASTBinary &AST) :
	SemaExpr(SemaKind::BINARY, SelectType(AST.getLeftExpr()->getSema(), AST.getRightExpr()->getSema())),
	AST(AST) {
}

ASTBinary &SemaBinary::getAST() const {
	return AST;
}
