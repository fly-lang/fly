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

ASTValue::ASTValue(const SourceLocation &Loc, ASTType *Ty) : Loc(Loc), Ty(Ty) {

}

const SourceLocation &ASTValue::getLocation() const {
    return Loc;
}

ASTType *ASTValue::getType() const {
    return Ty;
}

ASTSingleValue::ASTSingleValue(const SourceLocation &Loc, ASTType *Ty) : ASTValue(Loc, Ty) {
    if (Ty->isInteger() || Ty->isFloatingPoint()) {
        Str = "0";
    } else if (Ty->isBool()) {
        Str = "false";
    } else {
        Str = "";
    }
}

ASTSingleValue::ASTSingleValue(const SourceLocation &Loc, ASTType *Ty, std::string Str) : ASTValue(Loc, Ty), Str(Str) {

}

std::string ASTSingleValue::str() const {
    return Str;
}

bool ASTSingleValue::empty() const {
    return Str.empty();
}

ASTArrayValue::ASTArrayValue(const SourceLocation &Loc, ASTType *Type) :
    ASTValue(Loc, new ASTArrayType(Type->getLocation(), Type)) {

}

void ASTArrayValue::addValue(ASTValue * Value) {
    Values.push_back(Value);
}

unsigned int ASTArrayValue::size() const {
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
