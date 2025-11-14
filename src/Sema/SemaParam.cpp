//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaParam.cpp - The Symbolic Table for Param
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaParam.h"

using namespace fly;

SemaParam::SemaParam(ASTVar &AST) : SemaVar(&AST, SemaVarKind::PARAM_VAR) {

}

CodeGenVar * SemaParam::getCodeGen() const {
	return CodeGen;
}

void SemaParam::setCodeGen(CodeGenVarBase *CGV) {
	this->CodeGen = static_cast<CodeGenVar *>(CGV);
}
