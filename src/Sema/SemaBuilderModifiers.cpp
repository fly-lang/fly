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

SemaBuilderModifiers * SemaBuilderModifiers::Build(llvm::SmallVector<ASTModifier *, 8> Modifiers) {
	SemaBuilderModifiers *Instance = new SemaBuilderModifiers();
	for (auto &Modifier : Modifiers) {
		switch (Modifier->getModifierKind()) {
			case ASTModifierKind::MOD_CONSTANT:
				Instance->Constant = true;
				break;
			case ASTModifierKind::MOD_STATIC:
				Instance->Static = true;
				break;
			case ASTModifierKind::MOD_PUBLIC:
				Instance->Visibility = SemaVisibilityKind::PUBLIC;
				break;
			case ASTModifierKind::MOD_PRIVATE:
				Instance->Visibility = SemaVisibilityKind::PRIVATE;
				break;
			case ASTModifierKind::MOD_PROTECTED:
				Instance->Visibility = SemaVisibilityKind::PROTECTED;
				break;
			case ASTModifierKind::MOD_DEFAULT:
				Instance->Visibility = SemaVisibilityKind::DEFAULT;
				break;
		}
	}
	return Instance;
}

SemaVisibilityKind SemaBuilderModifiers::getVisibility() {
	return Visibility;
}

bool SemaBuilderModifiers::isConstant() {
	return Constant;
}

bool SemaBuilderModifiers::isStatic() {
	return Static;
}
