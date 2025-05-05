//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymClassMethod.cpp - The Symbolic Table for Enum Entry
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymClassMethod.h"

#include <AST/ASTFunction.h>
#include <AST/ASTVar.h>

using namespace fly;

SymClassMethod::SymClassMethod(ASTFunction *AST, SymClass *Class) :
	SymFunctionBase(AST, SymFunctionKind::METHOD, MangleFunction(AST)), Class(Class) {

}

// Function to mangle a type reference
std::string SymClassMethod::MangleFunction(ASTFunction *AST) {
	llvm::SmallVector<SymType *, 8> Params;
	for (auto Param : AST->getParams()) {
		Params.push_back(Param->getTypeRef()->getSym());
	}
	return SymFunctionBase::MangleFunction(AST->getName(), Params);
}

SymClass * SymClassMethod::getClass() const {
	return Class;
}

SymClass *SymClassMethod::getDerivedClass() const {
	return DerivedClass;
}

bool SymClassMethod::isConstructor() const {
	return MethodKind == SymClassMethodKind::METHOD_CONSTRUCTOR;
}

bool SymClassMethod::isStatic() const {
	return Static;
}

CodeGenClassFunction * SymClassMethod::getCodeGen() const {
	return CodeGen;
}

void SymClassMethod::setCodeGen(CodeGenClassFunction *CodeGen) {
	this->CodeGen = CodeGen;
}

SymComment * SymClassMethod::getComment() const {
	return Comment;
}
