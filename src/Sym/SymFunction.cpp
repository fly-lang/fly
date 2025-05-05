//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymFunction.cpp - The Symbolic table of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymFunction.h"

#include <AST/ASTFunction.h>
#include <AST/ASTVar.h>

using namespace fly;

SymFunction::SymFunction(ASTFunction *AST) : SymFunctionBase(AST, SymFunctionKind::FUNCTION, MangleFunction(AST)) {

}

// Function to mangle a type reference
std::string SymFunction::MangleFunction(ASTFunction *AST) {
	llvm::SmallVector<SymType *, 8> Params;
	for (auto Param : AST->getParams()) {
		Params.push_back(Param->getTypeRef()->getSym());
	}
	return SymFunctionBase::MangleFunction(AST->getName(), Params);
}

SymModule * SymFunction::getModule() const {
	return Module;
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
