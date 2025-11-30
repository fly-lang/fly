//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTMethod.cpp - AST Var implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTMethod.h"
#include "AST/ASTModifier.h"

#include <AST/ASTVisitor.h>
#include <Sema/SemaClassMethod.h>

using namespace fly;

ASTMethod::ASTMethod(const SourceLocation &Loc, ASTType *ReturnType,
	llvm::SmallVector<ASTModifier *, 8> &Modifiers, llvm::StringRef Name, llvm::SmallVector<ASTParam *, 8> &Params) :
	ASTFunction(Loc, ReturnType, Modifiers, Name, Params) {
}

void ASTMethod::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}
