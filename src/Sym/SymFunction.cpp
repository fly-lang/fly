//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymFunction.cpp - The Symbolic table of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymFunction.h"
#include <AST/ASTVar.h>

using namespace fly;

SymFunction::SymFunction(ASTFunction *AST) : SymFunctionBase(AST, SymFunctionKind::FUNCTION), MangledName(MangleFunction(AST)) {

}

SymModule * SymFunction::getModule() const {
	return Module;
}

std::string SymFunction::getMangledName() const {
	return MangledName;
}

SymComment * SymFunction::getComment() const {
	return Comment;
}

SymVisibilityKind SymFunction::getVisibility() const {
	return Visibility;
}

CodeGenFunction *SymFunction::getCodeGen() const {
	return CodeGen;
}

void SymFunction::setCodeGen(CodeGenFunction *CGF) {
	CodeGen = CGF;
}
