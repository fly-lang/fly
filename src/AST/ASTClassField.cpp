//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClassField.cpp - Class Field implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClassField.h"

using namespace fly;

ASTClassField::ASTClassField(const SourceLocation &Loc, ASTClass *Class, ASTClassScopes *Scopes, ASTType *Type,
                             std::string &Name) :
                             ASTVar(ASTVarKind::VAR_FIELD, Type, Name),
                             Loc(Loc), Class(Class), Scopes(Scopes) {

}

const SourceLocation &ASTClassField::getLocation() const {
    return Loc;
}

const ASTClass *ASTClassField::getClass() const {
    return Class;
}

const ASTValue *ASTClassField::getValue() const {
    return Value;
}

std::string ASTClassField::str() const {
    return ASTVar::str();
}
