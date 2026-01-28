//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaVar.cpp - The Symbolic Table for Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaVar.h"
#include <AST/ASTVar.h>

using namespace fly;

SemaVar::SemaVar(ASTVar *AST, SemaKind Kind, SemaType *Type) :
	SemaExpr(Kind, Type), AST(AST) {
}

SemaVar::~SemaVar() {
	// CodeGen is deleted by parent SemaExpr destructor
}

ASTVar *SemaVar::getAST() const {
	return AST;
}

llvm::StringRef SemaVar::getName() const {
	return AST->getName();
}

bool SemaVar::isConstant() const {
	return Constant;
}

CodeGenVar * SemaVar::getCodeGen() const {
	return static_cast<CodeGenVar *>(CodeGen);
}

void SemaVar::setCodeGen(CodeGenVar *CodeGen) {
	this->CodeGen = CodeGen;
}