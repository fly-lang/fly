//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaValue.cpp - value semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaValue.h"
#include "Sema/SemaVisitor.h"
#include "Basic/Logger.h"
#include "AST/ASTValue.h"

#include <Sema/SemaBuiltin.h>
#include <llvm/ADT/SmallString.h>

using namespace fly;

SemaValue::SemaValue(ASTValue &AST, SemaType *Type) : AST(AST), SemaExpr(SemaKind::VALUE, Type) {
}

ASTValue *SemaValue::getAST() const {
	return &AST;
}

void SemaValue::setCodeGen(CodeGenExpr *CGC) {
	this->CodeGen = CGC;
}

std::string SemaValue::str() const {
	return Logger("SemaValue")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Type", Type)
		.Attr("AST", AST.str())
		.End();
}

SemaBoolValue::SemaBoolValue(ASTBoolValue &AST) : SemaValue(AST, SemaBuiltin::getBoolType()), Value(AST.getValue()) {
}

bool SemaBoolValue::getValue() const {
	return Value;
}

void SemaBoolValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaBoolValue::str() const {
	return Logger("SemaBoolValue")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Value", getValue())
		.End();
}

SemaIntValue::SemaIntValue(ASTNumberValue &AST, SemaIntType *Type, llvm::APInt &Value) :
	SemaValue(AST, Type), Value(Value) {
}

llvm::APInt SemaIntValue::getValue() const {
	return Value;
}

void SemaIntValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaIntValue::str() const {
	llvm::SmallString<32> Buf;
	getValue().toString(Buf, 10, true);
	return Logger("SemaIntValue")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Value", Buf.str())
		.End();
}

SemaFloatValue::SemaFloatValue(ASTNumberValue &AST,  SemaFloatType *Type, llvm::APFloat &Value) :
	SemaValue(AST,  Type),
	Value(Value) {
}

llvm::APFloat SemaFloatValue::getValue() const {
	return Value;
}

void SemaFloatValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaFloatValue::str() const {
	llvm::SmallString<32> Buf;
	getValue().toString(Buf);
	return Logger("SemaFloatValue")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Value", Buf.str())
		.End();
}

SemaStringValue::SemaStringValue(ASTStringValue &AST) : SemaValue(AST, SemaBuiltin::getStringType()) {
}

llvm::StringRef SemaStringValue::getValue() const {
	return Value;
}

void SemaStringValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaStringValue::str() const {
	return Logger("SemaStringValue")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Value", getValue())
		.End();
}

SemaArrayValue::SemaArrayValue(ASTArrayValue &AST, SemaType *Type) : SemaValue(AST, Type) {
}

const llvm::SmallVector<SemaValue *, 8> &SemaArrayValue::getValues() const {
	return Values;
}

void SemaArrayValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaArrayValue::str() const {
	return Logger("SemaArrayValue")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Values", getValues())
		.End();
}

CodeGenArrayValue *SemaArrayValue::getCodeGen() const {
	return static_cast<CodeGenArrayValue *>(SemaValue::getCodeGen());
}

SemaStructValue::SemaStructValue(ASTStructValue &AST, SemaType *Type) : SemaValue(AST, Type) {
}

const llvm::StringMap<SemaValue *> &SemaStructValue::getValues() const {
	return Values;
}

void SemaStructValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaStructValue::str() const {
	std::string Str = Logger::OPEN_LIST;
	bool first = true;
	for (auto &KV : getValues()) {
		if (!first) Str += Logger::SEP;
		Str += KV.getKey().str() + Logger::EQ + (KV.getValue() ? KV.getValue()->str() : "null");
		first = false;
	}
	Str += Logger::CLOSE_LIST;
	return Logger("SemaStructValue")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Values", Str)
		.End();
}

SemaComplexValue::SemaComplexValue(ASTNumberValue &AST, SemaComplexType *Type,
                                   llvm::APFloat &Real, llvm::APFloat &Imag) :
	SemaValue(AST, Type), Real(Real), Imag(Imag) {
}

llvm::APFloat SemaComplexValue::getReal() const {
	return Real;
}

llvm::APFloat SemaComplexValue::getImag() const {
	return Imag;
}

void SemaComplexValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaComplexValue::str() const {
	llvm::SmallString<32> RBuf, IBuf;
	getReal().toString(RBuf);
	getImag().toString(IBuf);
	return Logger("SemaComplexValue")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Real", RBuf.str())
		.Attr("Imag", IBuf.str())
		.End();
}

SemaNullValue::SemaNullValue(ASTNullValue &AST) : SemaValue(AST, nullptr) {

}

void SemaNullValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaNullValue::str() const {
	return Logger("SemaNullValue")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.End();
}

SemaUnsetValue::SemaUnsetValue(ASTUnsetValue &AST) : SemaValue(AST, nullptr) {

}

void SemaUnsetValue::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaUnsetValue::str() const {
	return Logger("SemaUnsetValue")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.End();
}

