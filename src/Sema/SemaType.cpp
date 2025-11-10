//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaType.cpp - The Symbolic Table for Type
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaType.h"

#include <Sema/SemaClassType.h>
#include <Sema/SemaNameSpace.h>

using namespace fly;

SemaType::SemaType(SemaKind Kind, SemaTypeKind TypeKind, std::string Name) : SemaNode(Kind), TypeKind(TypeKind), Name(Name),
	Id(std::hash<std::string>{}(Name)) {
}

const size_t SemaType::getId() const {
	return Id;
}

const SemaTypeKind SemaType::getTypeKind() const {
	return TypeKind;
}

const std::string SemaType::getName() const {
	return Name;
}

SemaValue *SemaType::getDefaultValue() const {
	return DefaultValue;
}

bool SemaType::isBool() const {
	return TypeKind == SemaTypeKind::TYPE_BOOL;
}

bool SemaType::isFloatingPoint() const {
	return TypeKind == SemaTypeKind::TYPE_FLOATING_POINT;
}

bool SemaType::isInteger() const {
	return TypeKind == SemaTypeKind::TYPE_INTEGER;
}

bool SemaType::isArray() const {
	return TypeKind == SemaTypeKind::TYPE_ARRAY;
}

bool SemaType::isString() const {
	return TypeKind == SemaTypeKind::TYPE_STRING;
}

bool SemaType::isClass() const {
	return TypeKind == SemaTypeKind::TYPE_CLASS;
}

bool SemaType::isEnum() const {
	return TypeKind == SemaTypeKind::TYPE_ENUM;
}

bool SemaType::isError() const {
	return TypeKind == SemaTypeKind::TYPE_ERROR;
}

bool SemaType::isVoid() const {
	return TypeKind == SemaTypeKind::TYPE_VOID;
}

bool SemaType::isEquals(const SemaType *Type) const {
	return Type && this->getId() == Type->getId();
}

bool SemaType::operator!=(const SemaType *Type) const {
	return !isEquals(Type);
}

bool SemaType::operator==(const SemaType *Type) const {
	return isEquals(Type);
}

SemaIntType::SemaIntType(SemaIntTypeKind IntKind, std::string Name) : SemaType(SemaKind::BUILTIN_TYPE, SemaTypeKind::TYPE_INTEGER, Name),
                                                 IntKind(IntKind) {
}

const SemaIntTypeKind SemaIntType::getIntKind() const {
	return IntKind;
}

bool SemaIntType::isSigned() {
	return IntKind == SemaIntTypeKind::TYPE_SHORT || IntKind == SemaIntTypeKind::TYPE_INT ||
		IntKind == SemaIntTypeKind::TYPE_LONG;
}

SemaFloatType::SemaFloatType(SemaFloatTypeKind FPKind, std::string Name) : SemaType(SemaKind::BUILTIN_TYPE, SemaTypeKind::TYPE_FLOATING_POINT, Name),
                                             FPKind(FPKind) {
}

const SemaFloatTypeKind SemaFloatType::getFPKind() const {
	return FPKind;
}

SemaArrayType::SemaArrayType(SemaType *Type, ASTExpr *SizeExpr) :
	SemaType(SemaKind::BUILTIN_TYPE, SemaTypeKind::TYPE_ARRAY, "array"), Type(Type), SizeExpr(SizeExpr) {
}

SemaType *SemaArrayType::getType() {
	return Type;
}