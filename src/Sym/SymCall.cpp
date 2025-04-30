//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymVar.cpp - The Symbolic Table for Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymCall.h"

using namespace fly;

SymCall::SymCall(ASTCall *AST) : AST(AST){
}

ASTCall *SymCall::getAST() const {
	return AST;
}

SymCallKind SymCall::getKind() const {
	return Kind;
}

SymVar *SymCall::getParent() const {
	return Parent;
}

SymFunctionBase *SymCall::getFunction() const {
	return Function;
}

SymErrorHandler *SymCall::getErrorHandler() const {
	return ErrorHandler;
}
