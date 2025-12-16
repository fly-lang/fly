//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaValue.cpp - The Sema Value
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaValue.h"
#include "AST/ASTValue.h"

#include <Sema/SemaBuiltin.h>

using namespace fly;

SemaValue::SemaValue(ASTValue &AST, SemaType *Type) : AST(AST), SemaExpr(SemaKind::VALUE, Type) {
}

SemaType *SemaValue::getType() const {
	return Type;
}

SemaBoolValue::SemaBoolValue(ASTBoolValue &AST) : SemaValue(AST, SemaBuiltin::getBoolType()), Value(AST.getValue()) {
}

bool SemaBoolValue::getValue() const {
	return Value;
}

SemaIntValue::SemaIntValue(ASTNumberValue &AST) : SemaValue(AST, SemaBuiltin::getLongType()), Value(llvm::APInt(64, 0, true)) {
}

llvm::APInt SemaIntValue::getValue() const {
	return Value;
}

SemaFloatValue::SemaFloatValue(ASTNumberValue &AST) : SemaValue(AST, SemaBuiltin::getDoubleType()),
	Value(llvm::APFloat(llvm::APFloat::IEEEdouble(), "0.0")) {
}

llvm::APFloat SemaFloatValue::getValue() const {
	return Value;
}

SemaStringValue::SemaStringValue(ASTStringValue &AST) : SemaValue(AST, SemaBuiltin::getStringType()) {
}

llvm::StringRef SemaStringValue::getValue() const {
	return Value;
}

SemaArrayValue::SemaArrayValue(ASTArrayValue &AST, SemaType *Type) : SemaValue(AST, Type) {
}

const llvm::SmallVector<SemaValue *, 8> &SemaArrayValue::getValues() const {
	return Values;
}

SemaStructValue::SemaStructValue(ASTStructValue &AST, SemaType *Type) : SemaValue(AST, Type) {
}

const llvm::StringMap<SemaValue *> &SemaStructValue::getValues() const {
	return Values;
}

SemaNullValue::SemaNullValue(ASTNullValue &AST) : SemaValue(AST, nullptr) {

}
