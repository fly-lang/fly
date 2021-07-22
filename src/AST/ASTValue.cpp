//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/Value.h - Value
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTValue.h"

using namespace fly;

ASTValue::ASTValue(const SourceLocation &Loc, llvm::StringRef Str, TypeBase *Ty) : Loc(Loc), Str(Str), Ty(Ty) {

}

const StringRef &ASTValue::str() const {
    return Str;
}

TypeBase *ASTValue::getType() const {
    return Ty;
}

bool ASTValue::empty() const {
    return Str.empty();
}

bool ASTValue::isFalse() const {
    return Str.empty() || Str.equals("0") || Str.equals("false");
}

bool ASTValue::isTrue() const {
    return !isFalse();
}

