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

ASTScope::ASTScope(const SourceLocation &Loc, ASTScopeKind Kind) :
        ASTBase(Loc, ASTKind::AST_SCOPE), Kind(Kind), Visibility(ASTVisibilityKind::V_DEFAULT), Constant(false), Static(false) {

}

ASTScopeKind ASTScope::getKind() {
    return Kind;
}

ASTVisibilityKind ASTScope::getVisibility() const {
    return Visibility;
}

bool ASTScope::isConstant() const {
    return Constant;
}

bool ASTScope::isStatic() const {
    return Static;
}

std::string ASTScope::str() const {
    return Logger("ASTScope").
            Attr("Visibility", (uint64_t) Visibility).
            Attr("Constant", Constant).
            Attr("Constant", Static).
            End();
}
