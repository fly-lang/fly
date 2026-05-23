//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaComment.cpp - comment semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaComment.h"

using namespace fly;

SemaComment::SemaComment(ASTComment &AST) : AST(AST) {

}

ASTComment &SemaComment::getAST() const {
	return AST;
}