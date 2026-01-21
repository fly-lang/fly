//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaType.cpp - The Symbolic Table for Type
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaType.h"
#include "Sema/SemaVisitor.h"

#include <Sema/SemaClassType.h>
#include <Sema/SemaNameSpace.h>

using namespace fly;

SemaType::SemaType(SemaKind Kind, std::string Name) : SemaNode(Kind), Name(Name),
	Id(std::hash<std::string>{}(Name)) {
}

SemaType::~SemaType() {

}

const size_t SemaType::getId() const {
	return Id;
}

const std::string SemaType::getName() const {
	return Name;
}

bool SemaType::isBool() const {
	return Kind == SemaKind::TYPE_BOOL;
}

bool SemaType::isFloatingPoint() const {
	return Kind == SemaKind::TYPE_FLOATING_POINT;
}

bool SemaType::isInteger() const {
	return Kind == SemaKind::TYPE_INTEGER;
}

bool SemaType::isArray() const {
	return Kind == SemaKind::TYPE_ARRAY;
}

bool SemaType::isString() const {
	return Kind == SemaKind::TYPE_STRING;
}

bool SemaType::isClass() const {
	return Kind == SemaKind::TYPE_CLASS;
}

bool SemaType::isEnum() const {
	return Kind == SemaKind::TYPE_ENUM;
}

bool SemaType::isError() const {
	return Kind == SemaKind::TYPE_ERROR;
}

bool SemaType::isVoid() const {
	return Kind == SemaKind::TYPE_VOID;
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

CodeGenType *SemaType::getCodeGen() const {
	return CG;
}

void SemaType::setCodeGen(CodeGenType *CG) {
	this->CG = CG;
}

void SemaBoolType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

SemaIntType::SemaIntType(SemaIntTypeKind IntKind, std::string Name) : SemaType(SemaKind::TYPE_INTEGER, Name),
                                                 IntKind(IntKind) {
}

const SemaIntTypeKind SemaIntType::getIntKind() const {
	return IntKind;
}

bool SemaIntType::isSigned() {
	return IntKind == SemaIntTypeKind::TYPE_SHORT || IntKind == SemaIntTypeKind::TYPE_INT ||
		IntKind == SemaIntTypeKind::TYPE_LONG;
}

void SemaIntType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

SemaFloatType::SemaFloatType(SemaFloatTypeKind FPKind, std::string Name) : SemaType(SemaKind::TYPE_FLOATING_POINT, Name),
                                             FPKind(FPKind) {
}

const SemaFloatTypeKind SemaFloatType::getFPKind() const {
	return FPKind;
}

void SemaFloatType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

SemaArrayType::SemaArrayType(SemaType *Type, ASTExpr *SizeExpr) :
	SemaType(SemaKind::TYPE_ARRAY, "array"), Type(Type), SizeExpr(SizeExpr) {
}

SemaType *SemaArrayType::getType() {
	return Type;
}

void SemaArrayType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

void SemaErrorType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

void SemaStringType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

void SemaVoidType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}
