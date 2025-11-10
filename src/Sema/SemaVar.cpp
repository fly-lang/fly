//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaVar.cpp - The Symbolic Table for Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaVar.h"

#include <AST/ASTVar.h>

using namespace fly;

SemaVar::SemaVar(ASTVar &AST, SemaVarKind Kind) : SemaResult(SemaKind::VAR), AST(AST), VarKind(Kind) {
}

ASTVar &SemaVar::getAST() const {
	return AST;
}

SemaVarKind SemaVar::getVarKind() const {
	return VarKind;
}

bool SemaVar::isConstant() const {
	return Constant;
}

