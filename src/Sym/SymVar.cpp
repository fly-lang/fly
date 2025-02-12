//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymVar.cpp - The Symbolic Table for Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymVar.h"

using namespace fly;

SymVar::SymVar(ASTVar *AST, SymVarKind Kind) : AST(AST), Kind(Kind) {
}

ASTVar *SymVar::getAST() const {
	return AST;
}

SymVarKind SymVar::getKind() const {
	return Kind;
}

SymType *SymVar::getType() const {
	return Type;
}

bool SymVar::isConstant() const {
	return Constant;
}

