//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymClassAttribute.cpp - The Symbolic Table for Class Attribute
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymLocalVar.h"

using namespace fly;

SymLocalVar::SymLocalVar(ASTVar *AST) : SymVar(AST, SymVarKind::VAR_LOCAL) {
}

CodeGenVar * SymLocalVar::getCodeGen() const {
	return CodeGen;
}
