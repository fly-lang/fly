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

ASTValue::ASTValue(const ASTTypeKind TypeKind, const SourceLocation &Location) : TypeKind(TypeKind), Location(Location) {

}

const SourceLocation &ASTValue::getLocation() const {
    return Location;
}

const ASTTypeKind &ASTValue::getTypeKind() const {
    return TypeKind;
}

const std::string ASTValue::printType() const {
    return ASTType::printType(TypeKind);
}

std::string ASTValue::str() const {
    return Logger("ASTValue").
            Attr("Location", Location).
            Attr("Kind", (uint64_t) TypeKind).
            End();
}

ASTBoolValue::ASTBoolValue(const SourceLocation &Loc, bool Value) : ASTValue(ASTTypeKind::TYPE_BOOL, Loc), Value(Value) {

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
        ASTValue(ASTTypeKind::TYPE_INTEGER, Loc), Value(Value), Negative(Negative) {

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
    : ASTValue(ASTTypeKind::TYPE_FLOATING_POINT, Loc), Value(Value) {

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

ASTStringValue::ASTStringValue(const SourceLocation &Loc, llvm::StringRef Value)
        : ASTValue(ASTTypeKind::TYPE_STRING, Loc), Value(Value) {

}

llvm::StringRef ASTStringValue::getValue() const {
    return Value;
}

const std::string ASTStringValue::print() const {
    return Value.data();
}

std::string ASTStringValue::str() const {
    return Logger("ASTFloatingValue").
            Super(ASTValue::str()).
            Attr("Value", Value).
            End();
}

ASTArrayValue::ASTArrayValue(const SourceLocation &Loc) : ASTValue(ASTTypeKind::TYPE_ARRAY, Loc) {

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

ASTStructValue::ASTStructValue(const SourceLocation &Loc) : ASTValue(ASTTypeKind::TYPE_IDENTITY, Loc) {

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

ASTNullValue::ASTNullValue(const SourceLocation &Loc) : ASTValue(ASTTypeKind::TYPE_IDENTITY, Loc) {

}

const std::string ASTNullValue::print() const {
    return "null";
}

std::string ASTNullValue::str() const {
    return Logger("ASTNullValue").
            Super(ASTValue::str()).
            End();
}

ASTZeroValue::ASTZeroValue(const SourceLocation &Loc) : ASTValue(ASTTypeKind::TYPE_IDENTITY, Loc) {

}

const std::string ASTZeroValue::print() const {
    return "zero";
}

std::string ASTZeroValue::str() const {
    return Logger("ASTZeroValue").
            Super(ASTValue::str()).
            End();
}