//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaVar.cpp - The Symbolic Table for Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaVar.h"

using namespace fly;

SemaVar::SemaVar(ASTVar *AST, SemaVarKind Kind) : AST(AST), Kind(Kind) {
}

ASTVar *SemaVar::getAST() const {
	return AST;
}

SemaVarKind SemaVar::getKind() const {
	return Kind;
}

SemaVar *SemaVar::getParent() const {
	return Parent;
}

SemaType *SemaVar::getType() const {
	return Type;
}

bool SemaVar::isConstant() const {
	return Constant;
}

