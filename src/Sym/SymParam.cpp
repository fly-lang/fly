//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymParam.cpp - The Symbolic Table for Param
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymParam.h"

using namespace fly;

SymParam::SymParam(ASTVar *AST) : SymVar(AST, SymVarKind::VAR_PARAM) {

}

CodeGenVar * SymParam::getCodeGen() const {
	return CodeGen;
}

void SymParam::setCodeGen(CodeGenVarBase *CGV) {
	this->CodeGen = static_cast<CodeGenVar *>(CGV);
}
