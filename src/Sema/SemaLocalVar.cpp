//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaClassAttribute.cpp - The Symbolic Table for Class Attribute
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaLocalVar.h"

using namespace fly;

SemaLocalVar::SemaLocalVar(ASTVar &AST) : SemaVar(&AST, SemaVarKind::LOCAL_VAR) {
}

CodeGenVar * SemaLocalVar::getCodeGen() const {
	return CodeGen;
}

void SemaLocalVar::setCodeGen(CodeGenVarBase *CodeGen) {
	this->CodeGen = static_cast<CodeGenVar *>(CodeGen);
}
