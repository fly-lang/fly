//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaErrorHandler.cpp - The Symbolic Table for Error Handler
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaErrorHandler.h"

#include "AST/ASTVar.h"
#include "Sema/SemaVisitor.h"

#include <Sema/SemaType.h>

using namespace fly;

SemaErrorHandler::SemaErrorHandler(ASTVar *AST) : SemaVar(AST, SemaKind::ERROR_VAR, new SemaErrorType()) {

}

CodeGenError * SemaErrorHandler::getCodeGen() const {
	return CodeGen;
}

void SemaErrorHandler::setCodeGen(CodeGenVarBase *CodeGen) {
	this->CodeGen = static_cast<CodeGenError *>(CodeGen);
}

void SemaErrorHandler::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

