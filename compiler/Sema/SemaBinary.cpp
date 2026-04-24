//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBinary.cpp - The Symbolic Table for Binary Operation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBinary.h"

#include "AST/ASTBinary.h"
#include "Sema/Helper.h"
#include "Sema/SemaBuiltin.h"
#include "Sema/SemaType.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaType *SemaBinary::SelectType(ASTBinary &AST, SemaExpr *Left, SemaExpr *Right) {
	// Select the resulting type based on the operand types
	SemaType *Type1 = Left->getType();
	SemaType *Type2 = Right->getType();

	// Return Boolean type for logical and comparison operations
	if (AST.isCompare() || AST.isLogic()) {
		return SemaBuiltin::getBoolType();
	}

	// For arithmetic operations, determine the resulting type based on operand types
	if (Type1->isNumber() && Type2->isNumber()) {
		return Helper::SelectNumberType(
			static_cast<SemaNumberType *>(Type1),
			static_cast<SemaNumberType *>(Type2)
		);
	}

	// Default return Type1
	return Type1;
}

SemaBinary::SemaBinary(ASTBinary &AST, SemaExpr *Left, SemaExpr *Right) :
	SemaExpr(SemaKind::BINARY, SelectType(AST, Left, Right)),
	AST(AST), Left(Left), Right(Right) {
}

ASTBinary &SemaBinary::getAST() const {
	return AST;
}

SemaExpr *SemaBinary::getLeft() const {
	return Left;
}

SemaExpr *SemaBinary::getRight() const {
	return Right;
}

CodeGenExpr * SemaBinary::getCodeGen() const {
	return CodeGen;
}

void SemaBinary::setCodeGen(CodeGenExpr *CodeGen) {
	this->CodeGen = CodeGen;
}

void SemaBinary::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}
