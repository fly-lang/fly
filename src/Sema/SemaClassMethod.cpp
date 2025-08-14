//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymClassMethod.cpp - The Symbolic Table for Enum Entry
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaClassMethod.h"

#include <AST/ASTFunction.h>
#include <AST/ASTVar.h>

using namespace fly;

SemaClassMethod::SemaClassMethod(ASTFunction *AST, SemaClassType *Class, SemaClassInstance *This, SemaClassMethodKind MethodKind) :
	SemaFunctionBase(AST, SemaFunctionKind::CLASS_METHOD, MangleFunction(AST)), Class(Class), This(This), MethodKind(MethodKind) {

}

// Function to mangle a type reference
std::string SemaClassMethod::MangleFunction(ASTFunction *AST) {
	llvm::SmallVector<SemaType *, 8> Params;
	for (auto Param : AST->getParams()) {
		Params.push_back(Param->getTypeRef()->getSema());
	}
	return SemaFunctionBase::MangleFunction(AST->getName(), Params);
}

SemaClassType * SemaClassMethod::getClass() const {
	return Class;
}

SemaClassInstance * SemaClassMethod::getThis() const {
	return This;
}

bool SemaClassMethod::isConstructor() const {
	return MethodKind == SemaClassMethodKind::METHOD_CONSTRUCTOR;
}

bool SemaClassMethod::isAbstract() const {
	return MethodKind == SemaClassMethodKind::METHOD_ABSTRACT;
}

SemaVisibilityKind SemaClassMethod::getVisibility() const {
	return Visibility;
}

bool SemaClassMethod::isStatic() const {
	return Static;
}

CodeGenClassMethod * SemaClassMethod::getCodeGen() const {
	return CodeGen;
}

void SemaClassMethod::setCodeGen(CodeGenClassMethod *CodeGen) {
	this->CodeGen = CodeGen;
}

SemaComment * SemaClassMethod::getComment() const {
	return Comment;
}
