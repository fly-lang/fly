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
        ASTFunctionBase(ASTFunctionKind::FUNCTION, ReturnType, Name) {

}

const SourceLocation &ASTFunction::getLocation() const {
    return ASTTopDef::getLocation();
}

std::string ASTFunction::getName() const {
    return ASTFunctionBase::getName();
}

std::string ASTFunction::str() const {
    return Logger("ASTFunction").
            Super(ASTFunctionBase::str()).
            End();
}
