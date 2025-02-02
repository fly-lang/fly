//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTValue.h - Value
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTValue.h"

using namespace fly;

ASTValue::ASTValue(const ASTValueKind ValueKind, const SourceLocation &Location) :
        ASTBase(Location, ASTKind::AST_VALUE), ValueKind(ValueKind) {

}

const ASTValueKind &ASTValue::getTypeKind() const {
    return ValueKind;
}

std::string ASTValue::str() const {
    return Logger("ASTValue").
            Super(ASTBase::str()).
            Attr("Kind", (uint64_t) ValueKind).
            End();
}

ASTBoolValue::ASTBoolValue(const SourceLocation &Loc, bool Value) : ASTValue(ASTValueKind::VAL_BOOL, Loc), Value(Value) {

}

bool ASTBoolValue::getValue() const {
    return Value;
}

std::string ASTBoolValue::print() const {
    return Value ? "true" : "false";
}

std::string ASTBoolValue::str() const {
    return Logger("ASTBoolValue").
            Super(ASTValue::str()).
            Attr("Value", Value).
            End();
}

ASTIntegerValue::ASTIntegerValue(const SourceLocation &Loc, llvm::StringRef Value, uint8_t Radix) :
        ASTValue(ASTValueKind::VAL_INT, Loc), Value(Value), Radix(Radix) {

}

llvm::StringRef ASTIntegerValue::getValue() const {
    return Value;
}

uint8_t ASTIntegerValue::getRadix() const {
    return Radix;
}

std::string ASTIntegerValue::str() const {
    return Logger("ASTIntegerValue").
            Super(ASTValue::str()).
            Attr("Value", Value).
            Attr("Radix", (uint64_t) Radix).
            End();
}

ASTFloatingValue::ASTFloatingValue(const SourceLocation &Loc, llvm::StringRef Value)
    : ASTValue(ASTValueKind::VAL_FLOAT, Loc), Value(Value) {

}

std::string ASTFloatingValue::getValue() const {
    return Value;
}

std::string ASTFloatingValue::print() const {
    return Value;
}

std::string ASTFloatingValue::str() const {
    return Logger("ASTFloatingValue").
            Super(ASTValue::str()).
            Attr("Value", Value).
            End();
}

ASTCharValue::ASTCharValue(const SourceLocation &Loc, llvm::StringRef Value)
        : ASTValue(ASTValueKind::VAL_STRING, Loc), Value(Value) {

}

llvm::StringRef ASTCharValue::getValue() const {
    return Value;
}

std::string ASTCharValue::str() const {
    return Logger("ASTCharValue").
            Super(ASTValue::str()).
            Attr("Value", Value).
            End();
}

ASTStringValue::ASTStringValue(const SourceLocation &Loc, llvm::StringRef Value)
        : ASTValue(ASTValueKind::VAL_STRING, Loc), Value(Value) {

}

llvm::StringRef ASTStringValue::getValue() const {
    return Value;
}

std::string ASTStringValue::print() const {
    return Value.data();
}

std::string ASTStringValue::str() const {
    return Logger("ASTStringValue").
            Super(ASTValue::str()).
            Attr("Value", Value).
            End();
}

ASTArrayValue::ASTArrayValue(const SourceLocation &Loc) : ASTValue(ASTValueKind::VAL_ARRAY, Loc) {

}

uint64_t ASTArrayValue::size() const {
    return Values.size();
}

std::string ASTArrayValue::print() const {
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

const llvm::SmallVector<ASTValue *, 8> &ASTArrayValue::getValues() const {
    return Values;
}

ASTStructValue::ASTStructValue(const SourceLocation &Loc) : ASTValue(ASTValueKind::VAL_STRUCT, Loc) {

}

uint64_t ASTStructValue::size() const {
    return Values.size();
}

std::string ASTStructValue::print() const {
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

ASTNullValue::ASTNullValue(const SourceLocation &Loc) : ASTValue(ASTValueKind::VAL_NULL, Loc) {

}

std::string ASTNullValue::print() const {
    return "null";
}

std::string ASTNullValue::str() const {
    return Logger("ASTNullValue").
            Super(ASTValue::str()).
            End();
}

ASTZeroValue::ASTZeroValue(const SourceLocation &Loc) : ASTValue(ASTValueKind::VAL_ZERO, Loc) {

}

std::string ASTZeroValue::print() const {
    return "zero";
}

std::string ASTZeroValue::str() const {
    return Logger("ASTZeroValue").
            Super(ASTValue::str()).
            End();
}