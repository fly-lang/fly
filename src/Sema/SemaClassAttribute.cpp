//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaClassAttribute.cpp - The Symbolic Table for Class Attribute
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaClassAttribute.h"

using namespace fly;

SemaClassAttribute::SemaClassAttribute(ASTVar *AST, SemaClassType *Class) : SemaVar(AST, SemaVarKind::VAR_CLASS), Class(Class) {

}

SemaClassType * SemaClassAttribute::getClass() const {
	return Class;
}

CodeGenClassVar * SemaClassAttribute::getCodeGen() const {
	return CodeGen;
}

void SemaClassAttribute::setCodeGen(CodeGenVarBase *CodeGen) {
	this->CodeGen = static_cast<CodeGenClassVar *>(CodeGen);
}

SemaComment * SemaClassAttribute::getComment() const {
	return Comment;
}

bool SemaClassAttribute::isStatic() {
	return Static;
}
