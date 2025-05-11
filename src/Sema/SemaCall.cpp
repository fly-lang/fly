//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaVar.cpp - The Symbolic Table for Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaCall.h"

using namespace fly;

SemaCall::SemaCall(ASTCall *AST) : AST(AST){
}

ASTCall *SemaCall::getAST() const {
	return AST;
}

SemaCallKind SemaCall::getKind() const {
	return Kind;
}

SemaVar *SemaCall::getParent() const {
	return Parent;
}

SemaFunctionBase *SemaCall::getFunction() const {
	return Function;
}

SemaErrorHandler *SemaCall::getErrorHandler() const {
	return ErrorHandler;
}
