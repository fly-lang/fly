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

SemaUnary::SemaUnary(ASTUnary &AST, SemaExpr *Expr) :
	SemaExpr(SemaKind::UNARY, Expr->getType()), AST(AST), Expr(Expr) {
}

ASTUnary &SemaUnary::getAST() const {
	return AST;
}

SemaExpr *SemaUnary::getExpr() const {
	return Expr;
}

CodeGenExpr * SemaUnary::getCodeGen() const {
	return CodeGen;
}

void SemaUnary::setCodeGen(CodeGenExpr *CodeGen) {
	this->CodeGen = CodeGen;
}

void SemaUnary::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}
