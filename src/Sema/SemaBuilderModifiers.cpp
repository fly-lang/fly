//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBuilderModifiers.cpp - The Sema Builder Modifiers
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBuilderModifiers.h"
#include "AST/ASTModifier.h"

using namespace fly;

SemaBuilderModifiers *SemaBuilderModifiers::Build() {
    return new SemaBuilderModifiers();
}

SemaBuilderModifiers *SemaBuilderModifiers::addVisibility(const SourceLocation &Loc, ASTVisibilityKind VisibilityKind) {
    ASTModifier *Modifier = new ASTModifier(Loc, ASTModifierKind::M_VISIBILITY);
    Modifier->Visibility = VisibilityKind;
    Modifiers.push_back(Modifier);
    return this;
}

SemaBuilderModifiers *SemaBuilderModifiers::addConstant(const SourceLocation &Loc) {
    ASTModifier *Scope = new ASTModifier(Loc, ASTModifierKind::M_CONSTANT);
    Scope->Constant = true;
    Modifiers.push_back(Scope);
    return this;
}

SemaBuilderModifiers *SemaBuilderModifiers::addStatic(const SourceLocation &Loc) {
    ASTModifier *Scope = new ASTModifier(Loc, ASTModifierKind::M_STATIC);
    Scope->Static = true;
    Modifiers.push_back(Scope);
    return this;
}

llvm::SmallVector<ASTModifier *, 8> SemaBuilderModifiers::getModifiers() const {
    return Modifiers;
}
