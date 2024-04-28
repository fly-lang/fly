//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTParam.cpp - AST Param implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTParam.h"

using namespace fly;

ASTParam::ASTParam(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes) :
        ASTLocalVar(ASTVarKind::VAR_PARAM, Loc, Type, Name, Scopes) {

}

ASTValue *ASTParam::getDefaultValue() const {
    return DefaultValue;
}

void ASTParam::setDefaultValue(ASTValue *Value) {
    DefaultValue = Value;
}

std::string ASTParam::print() const {
    return getName().data();
}

std::string ASTParam::str() const {
    return Logger("ASTParam").
            Super(ASTLocalVar::str()).
            End();
}
