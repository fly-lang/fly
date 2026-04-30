//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaCast.cpp - The Symbolic Table for Cast Operation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaCast.h"
#include "Sema/SemaVisitor.h"

#include "AST/ASTCast.h"
#include "AST/ASTType.h"

using namespace fly;

SemaCast::SemaCast(ASTCast &AST, SemaExpr *Expr, SemaType *ToType) :
	SemaExpr(SemaKind::CAST, ToType), AST(AST), Expr(Expr), ToType(ToType) {
}

ASTCast &SemaCast::getAST() const {
	return AST;
}

SemaExpr *SemaCast::getExpr() const {
	return Expr;
}

SemaType *SemaCast::getToType() const {
	return ToType;
}

CodeGenExpr * SemaCast::getCodeGen() const {
	return CodeGen;
}

void SemaCast::setCodeGen(CodeGenExpr *CodeGen) {
	this->CodeGen = CodeGen;
}

void SemaCast::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}
