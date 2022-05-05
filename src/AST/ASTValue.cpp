//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTValue.h - Value
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTValue.h"
#include <string>

using namespace fly;

ASTValue::ASTValue(const SourceLocation &Loc, ASTType *Type) : Loc(Loc), Type(Type) {

}

const SourceLocation &ASTValue::getLocation() const {
    return Loc;
}

ASTType *ASTValue::getType() const {
    return Type;
}

ASTSingleValue::ASTSingleValue(const SourceLocation &Loc, ASTType *Type) : ASTValue(Loc, Type) {

}

ASTBoolValue::ASTBoolValue(const SourceLocation &Loc, bool Value) :
        ASTSingleValue(Loc, new ASTBoolType(Loc)), Value(Value) {

}

bool ASTBoolValue::getValue() const {
    return Value;
}

std::string ASTBoolValue::str() const {
    return Value ? "true" : "false";
}

ASTIntegerValue::ASTIntegerValue(const SourceLocation &Loc, ASTType *Type, uint64_t Value, bool Negative) :
        ASTSingleValue(Loc, Type), Value(Value), Negative(Negative) {

}

bool ASTIntegerValue::isNegative() const {
    return Negative;
}

bool ASTIntegerValue::isPositive() const {
    return !Negative;
}

uint64_t ASTIntegerValue::getValue() const {
    return Value;
}

std::string ASTIntegerValue::str() const {
    return (Negative ? "-" : "") + std::to_string(Value);
}

ASTFloatingValue::ASTFloatingValue(const SourceLocation &Loc, ASTType *Type, std::string &Value)
    : ASTSingleValue(Loc, Type), Value(Value) {

}

std::string ASTFloatingValue::getValue() const {
    return Value;
}

std::string ASTFloatingValue::str() const {
    return Value;
}

ASTArrayValue::ASTArrayValue(const SourceLocation &Loc, ASTType *Type) :
    ASTValue(Loc, new ASTArrayType(Type->getLocation(), Type,new ASTIntegerValue(Loc, Type, 0))) {

}

void ASTArrayValue::addValue(ASTValue * Value) {
    Values.push_back(Value);
}

uint64_t ASTArrayValue::size() const {
    return Values.size();
}

std::string ASTArrayValue::str() const {
    std::string Str = "{";
    for (auto Value : Values) {
        Str += Value->str() + ", ";
    }
    Str = Str.substr(0, Str.size()-1); // remove final comma
    Str += "}";
    return Str;
}

bool ASTArrayValue::empty() const {
    return Values.empty();
}

const std::vector<ASTValue *> &ASTArrayValue::getValues() const {
    return Values;
}

ASTClassValue::ASTClassValue(const SourceLocation &Loc, ASTClassType *ClassType) :
        ASTValue(Loc, ClassType) {

}

bool ASTClassValue::isNull() const {
    return true;
}

std::string ASTClassValue::str() const {
    return getType()->str() + "@ffffff";
}