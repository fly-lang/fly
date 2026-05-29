//===-------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTLocalVar.cpp - AST local variable implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTLocalVar.h"
#include "AST/ASTModifier.h"
#include "Sema/Symbol.h"
#include <AST/ASTVisitor.h>

using namespace fly;

ASTLocalVar::ASTLocalVar(
	const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, SmallVector<ASTModifier *, 8> &Modifiers) :
	ASTVar(Loc, Type, Name, Modifiers) {
}

void ASTLocalVar::accept(ASTVisitor &Visitor) { Visitor.visit(*this); }

Symbol *ASTLocalVar::getSymbol() const { return ASTVar::getSymbol(); }

void ASTLocalVar::setSymbol(Symbol *Sym) { ASTVar::setSymbol(Sym); }
