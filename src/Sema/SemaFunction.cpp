//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaFunction.cpp - The Symbolic table of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaFunction.h"

#include <AST/ASTFunction.h>
#include <AST/ASTVar.h>

using namespace fly;

SemaFunction::SemaFunction(ASTFunction *AST) : SemaFunctionBase(AST, SemaFunctionKind::FUNCTION, MangleFunction(AST)) {

}

// Function to mangle a type reference
std::string SemaFunction::MangleFunction(ASTFunction *AST) {
	llvm::SmallVector<SemaType *, 8> Params;
	for (auto Param : AST->getParams()) {
		Params.push_back(Param->getTypeRef()->getSema());
	}
	return SemaFunctionBase::MangleFunction(AST->getName(), Params);
}

SemaModule * SemaFunction::getModule() const {
	return Module;
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
