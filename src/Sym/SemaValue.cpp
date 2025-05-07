//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaValue.cpp - The Sema Value
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SemaValue.h"
#include "AST/ASTValue.h"

#include <llvm/ADT/APFloat.h>


using namespace fly;

SemaValue::SemaValue() {
}

SymType *SemaValue::getType() const {
	return Type;
}

SemaBoolValue::SemaBoolValue(bool Value) : SemaValue(), Value(Value) {
}

bool SemaBoolValue::getValue() const {
	return Value;
}

SemaIntValue::SemaIntValue(llvm::StringRef Value, uint8_t Radix) : SemaValue(), Value(llvm::APInt(64, Value, Radix)) {
}

llvm::APInt SemaIntValue::getValue() const {
	return Value;
}

SemaFloatValue::SemaFloatValue(llvm::StringRef Value) : SemaValue(), Value(llvm::APFloat(llvm::APFloat::IEEEdouble(), Value)) {
}

llvm::APFloat SemaFloatValue::getValue() const {
	return Value;
}

SemaStringValue::SemaStringValue(llvm::StringRef Value) : SemaValue(), Value(Value) {
}

llvm::StringRef SemaStringValue::getValue() const {
}

SemaArrayValue::SemaArrayValue() : SemaValue() {
}

llvm::SmallVector<SemaValue *, 8> &SemaArrayValue::getValues() const {
}

SemaStructValue::SemaStructValue() : SemaValue() {
}

llvm::StringMap<SemaValue *> &SemaStructValue::getValues() const {
}
