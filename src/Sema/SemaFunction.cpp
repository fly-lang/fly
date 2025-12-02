//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaFunction.cpp - The Symbolic table of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaFunction.h"

using namespace fly;

SemaFunction::SemaFunction(ASTFunction &AST, SymbolTable *Symbols) : SemaFunctionBase(AST, SemaKind::FUNCTION, MangleFunction(AST)),
	Symbols(Symbols) {

}

SemaModule *SemaFunction::getModule() const {
	return Module;
}

SymbolTable *SemaFunction::getSymbols() const {
	return Symbols;
}

SemaComment * SemaFunction::getComment() const {
	return Comment;
}

SemaVisibilityKind SemaFunction::getVisibility() const {
	return Visibility;
}

CodeGenFunction *SemaFunction::getCodeGen() const {
	return CodeGen;
}

void SemaFunction::setCodeGen(CodeGenFunction *CGF) {
	CodeGen = CGF;
}
