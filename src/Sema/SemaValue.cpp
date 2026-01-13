//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaValue.cpp - The Sema Value
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaValue.h"
#include "Sema/SemaVisitor.h"
#include "AST/ASTValue.h"

#include <Sema/SemaBuiltin.h>

using namespace fly;

SemaValue::SemaValue(ASTValue &AST, SemaType *Type) : AST(AST), SemaExpr(SemaKind::VALUE, Type) {
}

ASTValue *SemaValue::getAST() const {
	return &AST;
}

void SemaValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

SemaBoolValue::SemaBoolValue(ASTBoolValue &AST) : SemaValue(AST, SemaBuiltin::getBoolType()), Value(AST.getValue()) {
}

bool SemaBoolValue::getValue() const {
	return Value;
}

void SemaBoolValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

SemaIntValue::SemaIntValue(ASTNumberValue &AST, SemaIntType *Type, llvm::APInt &Value) :
	SemaValue(AST, Type), Value(Value) {
}

llvm::APInt SemaIntValue::getValue() const {
	return Value;
}

void SemaIntValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

SemaFloatValue::SemaFloatValue(ASTNumberValue &AST,  SemaFloatType *Type, llvm::APFloat &Value) :
	SemaValue(AST,  Type),
	Value(Value) {
}

llvm::APFloat SemaFloatValue::getValue() const {
	return Value;
}

void SemaFloatValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

SemaStringValue::SemaStringValue(ASTStringValue &AST) : SemaValue(AST, SemaBuiltin::getStringType()) {
}

llvm::StringRef SemaStringValue::getValue() const {
	return Value;
}

void SemaStringValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

SemaArrayValue::SemaArrayValue(ASTArrayValue &AST, SemaType *Type) : SemaValue(AST, Type) {
}

const llvm::SmallVector<SemaValue *, 8> &SemaArrayValue::getValues() const {
	return Values;
}

void SemaArrayValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

SemaStructValue::SemaStructValue(ASTStructValue &AST, SemaType *Type) : SemaValue(AST, Type) {
}

const llvm::StringMap<SemaValue *> &SemaStructValue::getValues() const {
	return Values;
}

void SemaStructValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

SemaNullValue::SemaNullValue(ASTNullValue &AST) : SemaValue(AST, nullptr) {

}

void SemaNullValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

