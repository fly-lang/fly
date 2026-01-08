//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaFunction.cpp - The Symbolic table of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaFunction.h"
#include "Sema/Helper.h"

using namespace fly;

SemaFunction::SemaFunction(ASTFunction &AST, SymbolTable *Symbols) : SemaFunctionBase(AST, SemaKind::FUNCTION),
	Symbols(Symbols) {

}

SemaFunction::~SemaFunction() {
	// Delete Comment if present
	delete Comment;
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

void SemaFunction::setVisibility(SemaVisibilityKind Visibility) {
	this->Visibility = Visibility;
}

const llvm::SmallVector<SemaLocalVar *, 4> &SemaFunction::getLocalVars() const {
	return LocalVars;
}

CodeGenFunction *SemaFunction::getCodeGen() const {
	return CodeGen;
}

void SemaFunction::setCodeGen(CodeGenFunction *CGF) {
	CodeGen = CGF;
}
