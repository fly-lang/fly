//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymType.cpp - The Symbolic Table for Type
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymType.h"

using namespace fly;

SymType::SymType(SymTypeKind Kind) : Kind(Kind) {
}

const SymTypeKind SymType::getKind() const {
	return Kind;
}

bool SymType::isBool() const {
	return Kind == SymTypeKind::TYPE_BOOL;
}

bool SymType::isFloatingPoint() const {
	return Kind == SymTypeKind::TYPE_FLOATING_POINT;
}

bool SymType::isInteger() const {
	return Kind == SymTypeKind::TYPE_INTEGER;
}

bool SymType::isArray() const {
	return Kind == SymTypeKind::TYPE_ARRAY;
}

bool SymType::isString() const {
	return Kind == SymTypeKind::TYPE_STRING;
}

bool SymType::isClass() const {
	return Kind == SymTypeKind::TYPE_CLASS;
}

bool SymType::isEnum() const {
	return Kind == SymTypeKind::TYPE_ENUM;
}

bool SymType::isError() const {
	return Kind == SymTypeKind::TYPE_ERROR;
}

bool SymType::isVoid() const {
	return Kind == SymTypeKind::TYPE_VOID;
}

SymTypeInt::SymTypeInt(SymIntTypeKind IntKind) : SymType(SymTypeKind::TYPE_INTEGER), IntKind(IntKind) {
}

const SymIntTypeKind SymTypeInt::getIntKind() const {
	return IntKind;
}

bool SymTypeInt::isSigned() {
	return IntKind == SymIntTypeKind::TYPE_BYTE || IntKind == SymIntTypeKind::TYPE_USHORT ||
		IntKind == SymIntTypeKind::TYPE_UINT || IntKind == SymIntTypeKind::TYPE_ULONG;
}

SymTypeFP::SymTypeFP(SymFPTypeKind FPKind) : SymType(SymTypeKind::TYPE_FLOATING_POINT), FPKind(FPKind) {
}

const SymFPTypeKind SymTypeFP::getFPKind() const {
	return FPKind;
}

SymTypeArray::SymTypeArray(SymType Type, uint64_t Size) : SymType(SymTypeKind::TYPE_ARRAY), Type(Type), Size(Size) {
}

SymType SymTypeArray::getType() {
}

const uint64_t SymTypeArray::getSize() const {
}





