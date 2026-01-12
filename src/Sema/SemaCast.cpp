//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaCast.cpp - The Symbolic Table for Cast Operation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaCast.h"

#include "AST/ASTCast.h"

using namespace fly;

SemaCast::SemaCast(ASTCast &AST) :
	SemaExpr(SemaKind::CAST, AST.getToType()->getSema()), AST(AST) {
}

ASTCast &SemaCast::getAST() const {
	return AST;
}


