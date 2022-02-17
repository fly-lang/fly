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

ASTValue::ASTValue(const SourceLocation &Loc, std::string Str, ASTType *Ty) : Loc(Loc), Str(Str), Ty(Ty) {

}

std::string ASTValue::str() const {
    return Str;
}

ASTType *ASTValue::getType() const {
    return Ty;
}

bool ASTValue::empty() const {
    return Str.empty();
}

bool ASTValue::isFalse() const {
    return Str.empty() || Str == "0" || Str == "false";
}

bool ASTValue::isTrue() const {
    return !isFalse();
}

