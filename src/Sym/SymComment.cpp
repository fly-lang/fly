//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymClass.cpp - The Sema identity Symbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymComment.h"

using namespace fly;

SymComment::SymComment(ASTComment *AST) {
	this->AST = AST;
}

ASTComment *SymComment::getAST() const {
	return AST;
}