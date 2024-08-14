//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBuilderScopes.cpp - The Sema Builder Scopes
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBuilderScopes.h"
#include "AST/ASTScopes.h"

using namespace fly;

SemaBuilderScopes *SemaBuilderScopes::Create() {
    return new SemaBuilderScopes();
}

SemaBuilderScopes *SemaBuilderScopes::addVisibility(const SourceLocation &Loc, ASTVisibilityKind VisibilityKind) {
    ASTScope *Scope = new ASTScope(Loc, ASTScopeKind::SCOPE_VISIBILITY);
    Scope->Visibility = VisibilityKind;
    Scopes.push_back(Scope);
    return this;
}

SemaBuilderScopes *SemaBuilderScopes::addConstant(const SourceLocation &Loc, bool Constant) {
    ASTScope *Scope = new ASTScope(Loc, ASTScopeKind::SCOPE_VISIBILITY);
    Scope->Constant = Constant;
    Scopes.push_back(Scope);
    return this;
}

SemaBuilderScopes *SemaBuilderScopes::addStatic(const SourceLocation &Loc, bool Static) {
    ASTScope *Scope = new ASTScope(Loc, ASTScopeKind::SCOPE_VISIBILITY);
    Scope->Static = Static;
    Scopes.push_back(Scope);
    return this;
}

llvm::SmallVector<ASTScope *, 8> SemaBuilderScopes::getScopes() const {
    return Scopes;
}
