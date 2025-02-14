//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymGlobalVar.cpp - The Symbolic table of GlobalVar
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymGlobalVar.h"

using namespace fly;

SymGlobalVar::SymGlobalVar(ASTVar *AST) : SymVar(AST, SymVarKind::VAR_GLOBAL) {

}

SymModule * SymGlobalVar::getModule() const {
	return Module;
}

SymComment * SymGlobalVar::getComment() const {
	return Comment;
}

SymVisibilityKind SymGlobalVar::getVisibility() const {
	return Visibility;
}

CodeGenGlobalVar *SymGlobalVar::getCodeGen() const {
	return CodeGen;
}

void SymGlobalVar::setCodeGen(CodeGenVarBase *CGV) {
	this->CodeGen = static_cast<CodeGenGlobalVar *>(CGV);
}