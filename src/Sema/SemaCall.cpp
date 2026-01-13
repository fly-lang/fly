//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaVar.cpp - The Symbolic Table for Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaCall.h"
#include "Sema/SemaVisitor.h"

#include <AST/ASTCall.h>

using namespace fly;

SemaCall::SemaCall(ASTCall &AST, SemaType *Type) : SemaExpr(SemaKind::CALL, Type), AST(AST) {
}

ASTCall &SemaCall::getAST() const {
	return AST;
}

SemaFunctionBase *SemaCall::getFunction() const {
	return Function;
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

void SemaCall::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

