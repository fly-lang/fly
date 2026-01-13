//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaUnary.cpp - The Symbolic Table for Unary Operation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaUnary.h"
#include "Sema/SemaVisitor.h"
#include "AST/ASTUnary.h"

using namespace fly;

SemaUnary::SemaUnary(ASTUnary &AST) :
	SemaExpr(SemaKind::UNARY, AST.getExpr()->getSema()->getType()), AST(AST) {
}

ASTUnary &SemaUnary::getAST() const {
	return AST;
}

void SemaUnary::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}
