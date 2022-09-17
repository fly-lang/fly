//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunction.cpp - Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunction.h"
#include <string>

using namespace fly;

ASTFunction::ASTFunction(const SourceLocation &Loc, ASTNode *Node, ASTType *ReturnType, const std::string Name,
                         ASTTopScopes *Scopes) :
        ASTTopDef(Loc, Node, ASTTopDefKind::DEF_FUNCTION, Scopes),
        ASTFunctionBase(ReturnType, Name) {

}

const std::string ASTFunction::getName() const {
    return ASTFunctionBase::getName();
}

std::string ASTFunction::str() const {
    return "ASTFunction { " + ASTFunctionBase::str() + " }";
}
