//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTEnumEntry.cpp - AST Var implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTEnumEntry.h"
#include "AST/ASTModifier.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTEnumEntry::ASTEnumEntry(
	const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, SmallVector<ASTModifier *, 8> &Modifiers) :
	ASTVar(Loc, Type, Name, ASTVarKind::VAR_ENUM_ENTRY, Modifiers) {
}

void ASTEnumEntry::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

SemaEnumEntry * ASTEnumEntry::getSema() const {
	return Sema;
}

void ASTEnumEntry::setSema(SemaEnumEntry *Sema) {
	this->Sema = Sema;
}
