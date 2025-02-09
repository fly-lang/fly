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

std::string getTypeName(SymTypeKind Kind) {
	switch (Kind) {

	case SymTypeKind::TYPE_VOID:
		return "void";
	case SymTypeKind::TYPE_BOOL:
		return "bool";
	case SymTypeKind::TYPE_STRING:
		return "string";
	case SymTypeKind::TYPE_CHAR:
		return "char";
	case SymTypeKind::TYPE_ERROR:
		return "error";
	}
}

SymType::SymType(SymTypeKind Kind) : Kind(Kind), Name(getTypeName(Kind)) {
}

SymType::SymType(SymTypeKind Kind, std::string Name) : Kind(Kind), Name(Name) {
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

std::string getIntTypeName(SymIntTypeKind Kind) {
	switch (Kind) {
	case SymIntTypeKind::TYPE_BYTE:
		return "byte";
	case SymIntTypeKind::TYPE_USHORT:
		return "ushort";
	case SymIntTypeKind::TYPE_UINT:
		return "uint";
	case SymIntTypeKind::TYPE_ULONG:
		return "ulong";
	case SymIntTypeKind::TYPE_INT:
		return "int";
	case SymIntTypeKind::TYPE_SHORT:
		return "short";
	case SymIntTypeKind::TYPE_LONG:
		return "long";
	default:
		return "int";
	}
}

SymTypeInt::SymTypeInt(SymIntTypeKind IntKind) : SymType(SymTypeKind::TYPE_INTEGER, getIntTypeName(IntKind)), IntKind(IntKind) {
}

const SymIntTypeKind SymTypeInt::getIntKind() const {
	return IntKind;
}

bool SymTypeInt::isSigned() {
	return IntKind == SymIntTypeKind::TYPE_BYTE || IntKind == SymIntTypeKind::TYPE_USHORT ||
		IntKind == SymIntTypeKind::TYPE_UINT || IntKind == SymIntTypeKind::TYPE_ULONG;
}

std::string getFPTypeName(SymFPTypeKind Kind) {
	switch (Kind) {
	case SymFPTypeKind::TYPE_FLOAT:
		return "float";
	case SymFPTypeKind::TYPE_DOUBLE:
		return "double";
	default:
		return "double";
	}
}

SymTypeFP::SymTypeFP(SymFPTypeKind FPKind) : SymType(SymTypeKind::TYPE_FLOATING_POINT, getFPTypeName(FPKind)), FPKind(FPKind) {
}

const SymFPTypeKind SymTypeFP::getFPKind() const {
	return FPKind;
}

SymTypeArray::SymTypeArray(SymType *Type, uint64_t Size) : SymType(SymTypeKind::TYPE_ARRAY, "array"), Type(Type), Size(Size) {
}

SymType *SymTypeArray::getType() {
	return Type;
}

const uint64_t SymTypeArray::getSize() const {
}
