//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaType.cpp - type semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaType.h"
#include "Sema/SemaVisitor.h"
#include "Basic/Logger.h"

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

bool SemaType::isComplex() const {
	return Kind == SemaKind::TYPE_COMPLEX;
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

std::string SemaType::str() const {
	return Logger("SemaType")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Id", (uint64_t)getId())
		.Attr("Name", getName())
		.End();
}

void SemaBoolType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaBoolType::str() const {
	return Logger("SemaBoolType")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.End();
}

SemaNumberType::SemaNumberType(SemaKind Kind, std::string Name, unsigned Rank) :
	SemaType(Kind, Name), Rank(Rank) {
}

unsigned SemaNumberType::getRank() {
	return Rank;
}

std::string SemaNumberType::str() const {
	return Logger("SemaNumberType")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.Attr("Rank", (uint64_t)const_cast<SemaNumberType*>(this)->getRank())
		.End();
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

std::string SemaIntType::str() const {
	return Logger("SemaIntType")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.Attr("Rank", (uint64_t)const_cast<SemaIntType*>(this)->getRank())
		.Attr("IntKind", static_cast<uint64_t>(getIntKind()))
		.End();
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

std::string SemaFloatType::str() const {
	return Logger("SemaFloatType")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.Attr("Rank", (uint64_t)const_cast<SemaFloatType*>(this)->getRank())
		.Attr("FloatKind", static_cast<uint64_t>(getFloatKind()))
		.End();
}

SemaArrayType::SemaArrayType(SemaType *ElementType, SemaExpr *SizeExpr) :
	SemaType(SemaKind::TYPE_ARRAY, "array"), ElementType(ElementType), SizeExpr(SizeExpr), Size(0) {
}

SemaArrayType::SemaArrayType(SemaType *ElementType, uint64_t Size) :
	SemaType(SemaKind::TYPE_ARRAY, "array"), ElementType(ElementType), Size(Size) {
}

SemaType *SemaArrayType::getElementType() {
	return ElementType;
}
void SemaArrayType::setElementType(SemaType *ElementType) {
	this->ElementType = ElementType;
}

SemaExpr *SemaArrayType::getSizeExpr() {
	return SizeExpr;
}

uint64_t SemaArrayType::getSize() const {
	return Size;
}

void SemaArrayType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaArrayType::str() const {
	return Logger("SemaArrayType")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.Attr("ElementType", ElementType)
		.Attr("Size", getSize())
		.End();
}

void SemaErrorType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaErrorType::str() const {
	return Logger("SemaErrorType")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.End();
}

void SemaStringType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaStringType::str() const {
	return Logger("SemaStringType")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.End();
}

void SemaVoidType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaVoidType::str() const {
	return Logger("SemaVoidType")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.End();
}

void SemaComplexType::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaComplexType::str() const {
	return Logger("SemaComplexType")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.End();
}
