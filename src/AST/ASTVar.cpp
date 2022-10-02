//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVar.cpp - Var declaration implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTVar.h"
#include "AST/ASTType.h"

using namespace fly;

ASTVar::ASTVar(ASTVarKind VarKind, ASTType *Type, const std::string Name) :
        VarKind(VarKind), Type(Type), Name(Name) {

}

ASTVarKind ASTVar::getVarKind() {
    return VarKind;
}

ASTType *ASTVar::getType() const {
    return Type;
}

const std::string ASTVar::getName() const {
    return Name;
}

ASTVar::~ASTVar() {
    delete Type;
}

std::string ASTVar::str() const {
    return "Type=" + Type->str() + ", " +
           "Name=" + Name + ", " +
           "VarKind=" + std::to_string((int) VarKind);
}
