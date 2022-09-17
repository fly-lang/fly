//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClassMethod.cpp - Class Method implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClassMethod.h"

using namespace fly;

ASTClassMethod::ASTClassMethod(const SourceLocation &Loc, ASTClass *Class, ASTClassScopes *Scopes, ASTType *Type,
                               std::string &Name) :
                               ASTFunctionBase(Type, Name), Loc(Loc), Class(Class), Scopes(Scopes)  {

}

const SourceLocation &ASTClassMethod::getLocation() const {
    return Loc;
}

const ASTClass *ASTClassMethod::getClass() const {
    return Class;
}

std::string ASTClassMethod::str() const {
    return ASTFunctionBase::str();
}