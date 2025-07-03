//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaMemberVar.cpp - The Symbolic Table for Member Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaMemberVar.h"

using namespace fly;

SemaMemberVar::SemaMemberVar(ASTVar *AST, SemaResult *Parent) : SemaVar(AST, SemaVarKind::MEMBER_VAR) {
	assert(Parent && "Parent cannot be null");
	setParent(Parent);
}

uint64_t SemaMemberVar::getIndex() const {
    return Index;
}

CodeGenVar * SemaMemberVar::getCodeGen() const {
	return CodeGen;
}

void SemaMemberVar::setCodeGen(CodeGenVarBase *CodeGen) {
	this->CodeGen = static_cast<CodeGenVar *>(CodeGen);
}
