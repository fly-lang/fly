//===-------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTParam.cpp - AST function parameter implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTParam.h"
#include "AST/ASTModifier.h"
#include "Sema/Symbol.h"
#include <AST/ASTVisitor.h>

using namespace fly;

ASTParam::ASTParam(
	const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, SmallVector<ASTModifier *, 8> &Modifiers) :
	ASTVar(Loc, Type, Name, Modifiers) {
}

void ASTParam::accept(ASTVisitor &Visitor) { Visitor.visit(*this); }

Symbol *ASTParam::getSymbol() const { return ASTVar::getSymbol(); }

void ASTParam::setSymbol(Symbol *Sym) { ASTVar::setSymbol(Sym); }
