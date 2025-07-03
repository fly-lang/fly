//===-------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTModifier.cpp - AST Modifier implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTModifier.h"
#include "Basic/Logger.h"

using namespace fly;

ASTModifier::ASTModifier(const SourceLocation &Loc, ASTModifierKind Kind) :
        ASTBase(Loc, ASTKind::AST_MODIFIER), Kind(Kind) {

}

ASTModifierKind ASTModifier::getModifierKind() {
    return Kind;
}

std::string ASTModifier::str() const {
    return Logger("ASTModifier").
	Attr("Location", getLocation()).
	Attr("Kind", static_cast<size_t>(getKind())).
    End();
}
