//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymFunctionBase.cpp - The Symbolic table of Function Base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymFunctionBase.h"

#include <CodeGen/CodeGenFunctionBase.h>

using namespace fly;

SymFunctionBase::SymFunctionBase(ASTFunction *AST, SymFunctionKind Kind) : AST(AST), Kind(Kind) {
}

ASTFunction * SymFunctionBase::getAST() {
	return AST;
}

