//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaErrorHandler.cpp - error handler semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTVar.h"
#include "Sema/SemaError.h"
#include "Sema/SemaVisitor.h"

#include <Sema/SemaType.h>

using namespace fly;

SemaError::SemaError(ASTVar *AST) : SemaVar(AST, SemaKind::ERROR_VAR, new SemaErrorType()) {

}

CodeGenError * SemaError::getCodeGen() const {
	return static_cast<CodeGenError *>(CodeGen);
}

void SemaError::setCodeGen(CodeGenError *CodeGen) {
	this->CodeGen = CodeGen;
}

void SemaError::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

