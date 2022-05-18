//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTValue.h - Value
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTValue.h"
#include "AST/ASTType.h"
#include <string>

using namespace fly;

ASTValue::ASTValue(const SourceLocation &Location) : Location(Location) {

}

const SourceLocation &ASTValue::getLocation() const {
    return Location;
}

ASTSingleValue::ASTSingleValue(const SourceLocation &Loc) : ASTValue(Loc) {

}

ASTBoolValue::ASTBoolValue(const SourceLocation &Loc, bool Value) : ASTSingleValue(Loc), Value(Value) {

}

bool ASTBoolValue::getValue() const {
    return Value;
}

std::string ASTBoolValue::str() const {
    return Value ? "true" : "false";
}

ASTIntegerValue::ASTIntegerValue(const SourceLocation &Loc, uint64_t Value, bool Negative) :
        ASTSingleValue(Loc), Value(Value), Negative(Negative) {

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

ASTFloatingValue::ASTFloatingValue(const SourceLocation &Loc, std::string &Value)
    : ASTSingleValue(Loc), Value(Value) {

}

std::string ASTFloatingValue::getValue() const {
    return Value;
}

std::string ASTFloatingValue::str() const {
    return Value;
}

ASTArrayValue::ASTArrayValue(const SourceLocation &Loc) : ASTValue(Loc) {

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

ASTNullValue::ASTNullValue(const SourceLocation &Loc) : ASTValue(Loc) {

}

std::string ASTNullValue::str() const {
    return "null";
}