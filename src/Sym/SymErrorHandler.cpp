//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymErrorHandler.cpp - The Symbolic Table for Error Handler
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymErrorHandler.h"

#include <Sym/SymType.h>

using namespace fly;

SymErrorHandler::SymErrorHandler() : SymVar(nullptr, SymVarKind::VAR_ERROR), Type(new SymErrorType()) {

}

CodeGenError * SymErrorHandler::getCodeGen() const {
	return CodeGen;
}

void SymErrorHandler::setCodeGen(CodeGenVarBase *CodeGen) {
	this->CodeGen = static_cast<CodeGenError *>(CodeGen);
}
