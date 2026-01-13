//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaParam.cpp - The Symbolic Table for Param
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaParam.h"
#include "Sema/SemaVisitor.h"
#include "AST/ASTParam.h"

using namespace fly;

SemaParam::SemaParam(ASTParam &AST, SemaType *Type) : SemaVar(&AST, SemaVarKind::PARAM_VAR, Type) {

}

CodeGenVar * SemaParam::getCodeGen() const {
	return CodeGen;
}

void SemaParam::setCodeGen(CodeGenVarBase *CGV) {
	this->CodeGen = static_cast<CodeGenVar *>(CGV);
}

void SemaParam::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

