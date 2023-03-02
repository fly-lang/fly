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

ASTValue::ASTValue(const ASTMacroTypeKind MacroKind, const SourceLocation &Location) : MacroKind(MacroKind), Location(Location) {

}

const SourceLocation &ASTValue::getLocation() const {
    return Location;
}

const ASTMacroTypeKind &ASTValue::getMacroKind() const {
    return MacroKind;
}

const std::string ASTValue::printMacroType() const {
    return ASTType::printMacroType(MacroKind);
}

std::string ASTValue::str() const {
    return Logger("ASTValue").
            Attr("Location", Location).
            Attr("MacroKind", (uint64_t) MacroKind).
            End();
}

ASTBoolValue::ASTBoolValue(const SourceLocation &Loc, bool Value) : ASTValue(ASTMacroTypeKind::MACRO_TYPE_BOOL, Loc), Value(Value) {

}

bool ASTBoolValue::getValue() const {
    return Value;
}

const std::string ASTBoolValue::print() const {
    return Value ? "true" : "false";
}

std::string ASTBoolValue::str() const {
    return Logger("ASTBoolValue").
            Super(ASTValue::str()).
            Attr("Value", Value).
            End();
}

ASTIntegerValue::ASTIntegerValue(const SourceLocation &Loc, uint64_t Value, bool Negative) :
        ASTValue(ASTMacroTypeKind::MACRO_TYPE_INTEGER, Loc), Value(Value), Negative(Negative) {

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
    return Logger("ASTIntegerValue").
            Super(ASTValue::str()).
            Attr("Value", Value).
            Attr("Negative", Negative).
            End();
}

ASTFloatingValue::ASTFloatingValue(const SourceLocation &Loc, std::string Value)
    : ASTValue(ASTMacroTypeKind::MACRO_TYPE_FLOATING_POINT, Loc), Value(Value) {

}

std::string ASTFloatingValue::getValue() const {
    return Value;
}

const std::string ASTFloatingValue::print() const {
    return Value;
}

std::string ASTFloatingValue::str() const {
    return Logger("ASTFloatingValue").
            Super(ASTValue::str()).
            Attr("Value", Value).
            End();
}

ASTArrayValue::ASTArrayValue(const SourceLocation &Loc) : ASTValue(ASTMacroTypeKind::MACRO_TYPE_ARRAY, Loc) {

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
    return Logger("ASTArrayValue").
            Super(ASTValue::str()).
            AttrList("Values", Values).
            End();
}

bool ASTArrayValue::empty() const {
    return Values.empty();
}

const std::vector<ASTValue *> &ASTArrayValue::getValues() const {
    return Values;
}

ASTStructValue::ASTStructValue(const SourceLocation &Loc) : ASTValue(ASTMacroTypeKind::MACRO_TYPE_STRUCT, Loc) {

}

uint64_t ASTStructValue::size() const {
    return Values.size();
}

const std::string ASTStructValue::print() const {
    std::string Str;
    for (auto &Value : Values) {
        Str += std::string(Value.getKey().data()) + ": " + Value.getValue()->str() + ", ";
    }
    Str = Str.substr(0, Str.size()-1); // remove final comma
    return Str;
}

std::string ASTStructValue::str() const {
    return Logger("ASTStructValue").
            Super(ASTValue::str()).
            Attr("Values", print()).
            End();
}

bool ASTStructValue::empty() const {
    return Values.empty();
}

const llvm::StringMap<ASTValue *> &ASTStructValue::getValues() const {
    return Values;
}

ASTNullValue::ASTNullValue(const SourceLocation &Loc) : ASTValue(ASTMacroTypeKind::MACRO_TYPE_CLASS, Loc) {

}

const std::string ASTNullValue::print() const {
    return "null";
}

std::string ASTNullValue::str() const {
    return Logger("ASTNullValue").
            Super(ASTValue::str()).
            End();
}