//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVar.cpp - AST Var implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTAttribute.h"
#include "AST/ASTModifier.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTAttribute::ASTAttribute(
	const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, SmallVector<ASTModifier *, 8> &Modifiers) :
	ASTVar(Loc, Type, Name, ASTKind::AST_ATTRIBUTE, Modifiers) {
}

void ASTAttribute::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

SemaClassAttribute * ASTAttribute::getSema() const {
}

void ASTAttribute::setSema(SemaClassAttribute *Sema) {
}
