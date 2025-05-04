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

size_t SymType::IdCounter = 0;

SymType::SymType(SymTypeKind Kind, std::string Name) : Kind(Kind), Name(Name), Id(GenerateId(Kind)) {
}

size_t SymType::GenerateId(fly::SymTypeKind Kind) {
	if (Kind == SymTypeKind::TYPE_CLASS || Kind == SymTypeKind::TYPE_ENUM) {
		return ++IdCounter;
	}
	return static_cast<size_t>(Kind);
}

const size_t SymType::getId() const {
	return Id;
}

const SymTypeKind SymType::getKind() const {
	return Kind;
}

const std::string SymType::getName() const {
	return Name;
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

bool SymType::isChar() const {
	return Kind == SymTypeKind::TYPE_CHAR;
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

SymTypeInt::SymTypeInt(SymIntTypeKind IntKind, std::string Name) : SymType(SymTypeKind::TYPE_INTEGER, Name),
                                                 IntKind(IntKind) {
}

const SymIntTypeKind SymTypeInt::getIntKind() const {
	return IntKind;
}

bool SymTypeInt::isSigned() {
	return IntKind == SymIntTypeKind::TYPE_BYTE || IntKind == SymIntTypeKind::TYPE_USHORT ||
	       IntKind == SymIntTypeKind::TYPE_UINT || IntKind == SymIntTypeKind::TYPE_ULONG;
}

SymTypeFP::SymTypeFP(SymFPTypeKind FPKind, std::string Name) : SymType(SymTypeKind::TYPE_FLOATING_POINT, Name),
                                             FPKind(FPKind) {
}

const SymFPTypeKind SymTypeFP::getFPKind() const {
	return FPKind;
}

SymTypeArray::SymTypeArray(SymType *Type) : SymType(SymTypeKind::TYPE_ARRAY, "array"), Type(Type) {
}

SymType *SymTypeArray::getType() {
	return Type;
}