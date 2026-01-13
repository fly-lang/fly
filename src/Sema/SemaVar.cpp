//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaVar.cpp - The Symbolic Table for Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaVar.h"
#include "Sema/SemaVisitor.h"

#include <AST/ASTVar.h>

using namespace fly;

SemaVar::SemaVar(ASTVar *AST, SemaVarKind Kind, SemaType *Type) :
	SemaExpr(SemaKind::VAR, Type), AST(AST), VarKind(Kind) {
}

ASTVar *SemaVar::getAST() const {
	return AST;
}

llvm::StringRef SemaVar::getName() const {
	return AST->getName();
}

SemaVarKind SemaVar::getVarKind() const {
	return VarKind;
}

bool SemaVar::isConstant() const {
	return Constant;
}

void SemaVar::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}
