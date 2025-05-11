//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaType.cpp - The Symbolic Table for Type
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaType.h"

using namespace fly;

size_t SemaType::IdCounter = 0;

SemaType::SemaType(SemaTypeKind Kind, std::string Name) : Kind(Kind), Name(Name), Id(GenerateId(Kind)) {
}

size_t SemaType::GenerateId(fly::SemaTypeKind Kind) {
	if (Kind == SemaTypeKind::TYPE_CLASS || Kind == SemaTypeKind::TYPE_ENUM) {
		return ++IdCounter;
	}
	return static_cast<size_t>(Kind);
}

const size_t SemaType::getId() const {
	return Id;
}

const SemaTypeKind SemaType::getKind() const {
	return Kind;
}

const std::string SemaType::getName() const {
	return Name;
}

SemaValue *SemaType::getDefaultValue() const {
	return DefaultValue;
}

bool SemaType::isBool() const {
	return Kind == SemaTypeKind::TYPE_BOOL;
}

bool SemaType::isFloatingPoint() const {
	return Kind == SemaTypeKind::TYPE_FLOATING_POINT;
}

bool SemaType::isInteger() const {
	return Kind == SemaTypeKind::TYPE_INTEGER;
}

bool SemaType::isArray() const {
	return Kind == SemaTypeKind::TYPE_ARRAY;
}

bool SemaType::isString() const {
	return Kind == SemaTypeKind::TYPE_STRING;
}

bool SemaType::isClass() const {
	return Kind == SemaTypeKind::TYPE_CLASS;
}

bool SemaType::isEnum() const {
	return Kind == SemaTypeKind::TYPE_ENUM;
}

bool SemaType::isError() const {
	return Kind == SemaTypeKind::TYPE_ERROR;
}

bool SemaType::isVoid() const {
	return Kind == SemaTypeKind::TYPE_VOID;
}

SemaIntType::SemaIntType(SemaIntTypeKind IntKind, std::string Name) : SemaType(SemaTypeKind::TYPE_INTEGER, Name),
                                                 IntKind(IntKind) {
}

const SemaIntTypeKind SemaIntType::getIntKind() const {
	return IntKind;
}

bool SemaIntType::isSigned() {
	return IntKind == SemaIntTypeKind::TYPE_SHORT || IntKind == SemaIntTypeKind::TYPE_INT ||
		IntKind == SemaIntTypeKind::TYPE_LONG;
}

SemaFloatType::SemaFloatType(SemaFloatTypeKind FPKind, std::string Name) : SemaType(SemaTypeKind::TYPE_FLOATING_POINT, Name),
                                             FPKind(FPKind) {
}

const SemaFloatTypeKind SemaFloatType::getFPKind() const {
	return FPKind;
}

SemaArrayType::SemaArrayType(SemaType *Type) : SemaType(SemaTypeKind::TYPE_ARRAY, "array"), Type(Type) {
}

SemaType *SemaArrayType::getType() {
	return Type;
}