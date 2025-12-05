//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTValue.h - Value
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTValue.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTValue::ASTValue(const ASTValueKind ValueKind, const SourceLocation &Location) :
        ASTExpr(Location, ASTExprKind::EXPR_VALUE), ValueKind(ValueKind) {

}

const ASTValueKind &ASTValue::getValueKind() const {
    return ValueKind;
}

SemaValue *ASTValue::getSema() const {
	return static_cast<SemaValue *>(Sema);
}

void ASTValue::setSema(SemaValue *Sema) {
	this->Sema = Sema;
}

ASTDefaultValue::ASTDefaultValue() : ASTValue(ASTValueKind::VAL_DEFAULT, SourceLocation()) {
}

void ASTDefaultValue::accept(ASTVisitor &Visitor) {
}

std::string ASTDefaultValue::str() const {
	return Logger("ASTDefaultValue").
		Attr("Kind", static_cast<size_t>(getKind())).
		End();
}

ASTBoolValue::ASTBoolValue(const SourceLocation &Loc, bool Value) : ASTValue(ASTValueKind::VAL_BOOL, Loc), Value(Value) {

}

void ASTBoolValue::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

bool ASTBoolValue::getValue() const {
    return Value;
}

std::string ASTBoolValue::str() const {
    return Logger("ASTBoolValue").
	Attr("Location", getLocation()).
Attr("Kind", static_cast<size_t>(getKind())).
            Attr("Value", Value).
            End();
}

ASTNumberValue::ASTNumberValue(const SourceLocation &Loc, llvm::StringRef Value) :
        ASTValue(ASTValueKind::VAL_NUMBER, Loc), Value(Value) {

}

void ASTNumberValue::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

llvm::StringRef ASTNumberValue::getValue() const {
    return Value;
}

std::string ASTNumberValue::str() const {
    return Logger("ASTIntegerValue").
	Attr("Location", getLocation()).
	Attr("Kind", static_cast<size_t>(getKind())).
            Attr("Value", Value).
            End();
}

ASTStringValue::ASTStringValue(const SourceLocation &Loc, llvm::StringRef Value)
        : ASTValue(ASTValueKind::VAL_STRING, Loc), Value(Value) {

}

void ASTStringValue::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

llvm::StringRef ASTStringValue::getValue() const {
    return Value;
}

std::string ASTStringValue::str() const {
    return Logger("ASTStringValue").
	Attr("Location", getLocation()).
Attr("Kind", static_cast<size_t>(getKind())).
            Attr("Value", Value).
            End();
}

ASTArrayValue::ASTArrayValue(const SourceLocation &Loc) : ASTValue(ASTValueKind::VAL_ARRAY, Loc) {

}

void ASTArrayValue::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

uint64_t ASTArrayValue::size() const {
    return Values.size();
}

std::string ASTArrayValue::str() const {
	std::string Str;
	for (auto Value : Values) {
	    Str += Value->str() + ", ";
	}
	Str = Str.substr(0, Str.size()-1); // remove final comma
    return Logger("ASTArrayValue").
	Attr("Location", getLocation()).
	Attr("Kind", static_cast<size_t>(getKind())).
	Attr("Values", Str).
	End();
}

bool ASTArrayValue::empty() const {
    return Values.empty();
}

const llvm::SmallVector<ASTValue *, 8> &ASTArrayValue::getValues() const {
    return Values;
}

ASTStructValue::ASTStructValue(const SourceLocation &Loc) : ASTValue(ASTValueKind::VAL_STRUCT, Loc) {

}

void ASTStructValue::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

uint64_t ASTStructValue::size() const {
    return Values.size();
}

std::string ASTStructValue::str() const {
	std::string Str;
	for (auto &Value : Values) {
	    Str += std::string(Value.getKey().data()) + ": " + Value.getValue()->str() + ", ";
	}
	Str = Str.substr(0, Str.size()-1); // remove final comma
    return Logger("ASTStructValue").
		Attr("Location", getLocation()).
		Attr("Kind", static_cast<size_t>(getKind())).
		Attr("Values", Str).
		End();
}

bool ASTStructValue::empty() const {
    return Values.empty();
}

const llvm::StringMap<ASTValue *> &ASTStructValue::getValues() const {
    return Values;
}

ASTNullValue::ASTNullValue(const SourceLocation &Loc) : ASTValue(ASTValueKind::VAL_NULL, Loc) {

}

void ASTNullValue::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string ASTNullValue::str() const {
    return Logger("ASTNullValue").
	Attr("Location", getLocation()).
Attr("Kind", static_cast<size_t>(getKind())).
            End();
}
