//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymClassAttribute.cpp - The Symbolic Table for Class Attribute
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymClassAttribute.h"

using namespace fly;

SymClassAttribute::SymClassAttribute(ASTVar *AST) : SymVar(AST, SymVarKind::VAR_CLASS) {

}

SymClass * SymClassAttribute::getClass() const {
	return Class;
}

CodeGenClassVar * SymClassAttribute::getCodeGen() const {
	return CodeGen;
}

void SymClassAttribute::setCodeGen(CodeGenClassVar *CodeGen) {
	this->CodeGen = CodeGen;
}

SymComment * SymClassAttribute::getComment() const {
	return Comment;
}

bool SymClassAttribute::isStatic() {
	return Static;
}
