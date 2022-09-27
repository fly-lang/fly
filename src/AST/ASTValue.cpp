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

using namespace fly;

ASTValue::ASTValue(const MacroTypeKind MacroKind, const SourceLocation &Location) : MacroKind(MacroKind), Location(Location) {

}

const SourceLocation &ASTValue::getLocation() const {
    return Location;
}

const MacroTypeKind &ASTValue::getMacroKind() const {
    return MacroKind;
}

const std::string ASTValue::printMacroType() const {
    return ASTType::printMacroType(MacroKind);
}

ASTBoolValue::ASTBoolValue(const SourceLocation &Loc, bool Value) : ASTValue(MacroTypeKind::MACRO_TYPE_BOOL, Loc), Value(Value) {

}

bool ASTBoolValue::getValue() const {
    return Value;
}

const std::string ASTBoolValue::print() const {
    return Value ? "true" : "false";
}

std::string ASTBoolValue::str() const {
    return Value ? "true" : "false";
}

ASTIntegerValue::ASTIntegerValue(const SourceLocation &Loc, uint64_t Value, bool Negative) :
        ASTValue(MacroTypeKind::MACRO_TYPE_INTEGER, Loc), Value(Value), Negative(Negative) {

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

const std::string ASTIntegerValue::print() const {
    return (Negative ? "-" : "") + std::to_string(Value);
}


std::string ASTIntegerValue::str() const {
    return "ASTIntegerValue{Negative=" + std::to_string(Negative) + ",Value=" + std::to_string(Value) + "}";
}

ASTFloatingValue::ASTFloatingValue(const SourceLocation &Loc, std::string Value)
    : ASTValue(MacroTypeKind::MACRO_TYPE_FLOATING_POINT, Loc), Value(Value) {

}

std::string ASTFloatingValue::getValue() const {
    return Value;
}

const std::string ASTFloatingValue::print() const {
    return Value;
}

std::string ASTFloatingValue::str() const {
    return "ASTIntegerValue{Value=" + Value + "}";
}

ASTArrayValue::ASTArrayValue(const SourceLocation &Loc) : ASTValue(MacroTypeKind::MACRO_TYPE_ARRAY, Loc) {

}

uint64_t ASTArrayValue::size() const {
    return Values.size();
}

const std::string ASTArrayValue::print() const {
    std::string Str;
    for (auto Value : Values) {
        Str += Value->str() + ", ";
    }
    Str = Str.substr(0, Str.size()-1); // remove final comma
    return Str;
}

std::string ASTArrayValue::str() const {
    return "ASTArrayValue{" + print() + "}";
}

bool ASTArrayValue::empty() const {
    return Values.empty();
}

const std::vector<ASTValue *> &ASTArrayValue::getValues() const {
    return Values;
}

ASTNullValue::ASTNullValue(const SourceLocation &Loc) : ASTValue(MacroTypeKind::MACRO_TYPE_CLASS, Loc) {

}

const std::string ASTNullValue::print() const {
    return "null";
}

std::string ASTNullValue::str() const {
    return "ASTNullValue{}";
}