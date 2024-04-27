//===-------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTScopes.cpp - AST Scopes implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTScopes.h"

using namespace fly;

ASTScopes::ASTScopes(const SourceLocation &Loc) :
        ASTBase(Loc), Visibility(ASTVisibilityKind::V_DEFAULT), Constant(false), Static(false) {

}

ASTVisibilityKind ASTScopes::getVisibility() const {
    return Visibility;
}

void ASTScopes::setVisibility(ASTVisibilityKind V) {
    this->Visibility = V;
}

bool ASTScopes::isConstant() const {
    return Constant;
}

void ASTScopes::setConstant(bool C) {
    this->Constant = C;
}

bool ASTScopes::isStatic() const {
    return Static;
}

void ASTScopes::setStatic(bool S) {
    this->Static = S;
}

std::string ASTScopes::str() const {
    return Logger("ASTScopes").
            Attr("Visibility", (uint64_t) Visibility).
            Attr("Constant", Constant).
            Attr("Constant", Static).
            End();
}
