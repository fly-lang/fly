//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaVar.cpp - The Symbolic Table for Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaCall.h"

#include <AST/ASTCall.h>

using namespace fly;

SemaCall::SemaCall(ASTCall &AST) : SemaResult(SemaKind::CALL), AST(AST) {
}

ASTCall &SemaCall::getAST() const {
	return AST;
}

SemaFunctionBase *SemaCall::getFunction() const {
	return Function;
}

void SemaCall::setFunction(SemaFunctionBase *Function) {
	this->Function = Function;
}

SemaErrorHandler *SemaCall::getErrorHandler() const {
	return ErrorHandler;
}

void SemaCall::setErrorHandler(SemaErrorHandler *ErrorHandler) {
	this->ErrorHandler = ErrorHandler;
}

bool SemaCall::isNew() const {
	return AST.getCallKind() >= ASTCallKind::CALL_NEW;
}
