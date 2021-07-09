//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/Value.h - Value
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/Value.h"

using namespace fly;

fly::Value::Value(llvm::StringRef Str, TypeBase *Ty) : Loc(Loc), Str(Str), Ty(Ty) {

}

const StringRef &Value::str() const {
    return Str;
}

TypeBase *Value::getType() const {
    return Ty;
}

