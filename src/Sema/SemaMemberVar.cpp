//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaMemberVar.cpp - The Symbolic Table for Member Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaMemberVar.h"
#include <Sema/SemaClassAttribute.h>

using namespace fly;

SemaMemberVar::SemaMemberVar(ASTVar &AST, SemaExpr &Parent, SemaClassAttribute *Attribute) :
	SemaVar(&AST, SemaVarKind::MEMBER_VAR, Attribute->getType()), ClassAttribute(Attribute) {
	setParent(Parent);
}

SemaClassAttribute * SemaMemberVar::getClassAttribute() const {
	return ClassAttribute;
}

void SemaMemberVar::setClassAttribute(SemaClassAttribute *ClassAttribute) {
	this->ClassAttribute = ClassAttribute;
}

CodeGenVar * SemaMemberVar::getCodeGen() const {
	return CodeGen;
}

void SemaMemberVar::setCodeGen(CodeGenVarBase *CodeGen) {
	this->CodeGen = static_cast<CodeGenVar *>(CodeGen);
}
