//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaGlobalVar.cpp - The Symbolic table of GlobalVar
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaGlobalVar.h"

using namespace fly;

SemaGlobalVar::SemaGlobalVar(ASTVar *AST) : SemaVar(AST, SemaVarKind::VAR_GLOBAL) {

}

SemaModule * SemaGlobalVar::getModule() const {
	return Module;
}

SemaComment * SemaGlobalVar::getComment() const {
	return Comment;
}

SemaVisibilityKind SemaGlobalVar::getVisibility() const {
	return Visibility;
}

CodeGenGlobalVar *SemaGlobalVar::getCodeGen() const {
	return CodeGen;
}

void SemaGlobalVar::setCodeGen(CodeGenVarBase *CGV) {
	this->CodeGen = static_cast<CodeGenGlobalVar *>(CGV);
}