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
	// Delete CodeGen if present
	delete CG;
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

bool SemaType::isFloat() const {
	return Kind == SemaKind::TYPE_FLOAT;
}

bool SemaType::isInteger() const {
	return Kind == SemaKind::TYPE_INTEGER;
}

bool SemaType::isNumber() const {
	return isInteger() || isFloat();
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

SemaNumberType::SemaNumberType(SemaKind Kind, std::string Name, unsigned Rank) :
	SemaType(Kind, Name), Rank(Rank) {
}

unsigned SemaNumberType::getRank() {
	return Rank;
}

SemaIntType::SemaIntType(SemaIntTypeKind IntKind, std::string Name) :
	SemaNumberType(SemaKind::TYPE_INTEGER, Name, static_cast<unsigned>(IntKind)), IntKind(IntKind) {
}

const SemaIntTypeKind SemaIntType::getIntKind() const {
	return IntKind;
}

bool SemaIntType::isUnsigned() {
	return static_cast<int>(IntKind) % 2 == 0;
}

bool SemaIntType::isSigned() {
	return !isUnsigned();
}

void SemaIntType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

SemaFloatType::SemaFloatType(SemaFloatTypeKind FPKind, std::string Name) :
	SemaNumberType(SemaKind::TYPE_FLOAT, Name, static_cast<unsigned>(FPKind) * 10), FloatKind(FPKind) {
}

const SemaFloatTypeKind SemaFloatType::getFloatKind() const {
	return FloatKind;
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
