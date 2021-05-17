//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/VarDecl.h - Var declaration implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/VarDecl.h"

using namespace fly;

VarDecl::VarDecl(TypeBase *Type, const StringRef &Name) : Type(Type), Name(Name) {}

TypeBase *VarDecl::getType() const {
    return Type;
}

const llvm::StringRef &VarDecl::getName() const {
    return Name;
}

VarDecl::~VarDecl() {
    delete Type;
    delete Expression;
}

bool VarDecl::isConstant() const {
    return Constant;
}

GroupExpr *VarDecl::getExpr() const {
    return Expression;
}
