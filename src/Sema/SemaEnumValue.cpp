//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaEnumValue.cpp - The Symbolic Table for Enum Entry
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaEnumValue.h"

#include "AST/ASTEnumValue.h"
#include "Sema/SemaEnumType.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaEnumValue::SemaEnumValue(ASTEnumValue &AST, SemaEnumType *Type) : SemaValue(AST, Type) {

}

SemaEnumValue::~SemaEnumValue() {
	// Delete Comment if present
	delete Comment;
}

size_t SemaEnumValue::getIndex() const {
	return Index;
}

SemaComment * SemaEnumValue::getComment() const {
	return Comment;
}

CodeGenEnumValue * SemaEnumValue::getCodeGen() const {
	return static_cast<CodeGenEnumValue *>(CodeGen);
}

void SemaEnumValue::setCodeGen(CodeGenExpr *CGC) {
	this->CodeGen = static_cast<CodeGenEnumValue *>(CGC);
}

void SemaEnumValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

