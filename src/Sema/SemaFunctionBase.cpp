//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaFunctionBase.cpp - The Symbolic table of Function Base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaFunctionBase.h"
#include "Sema/SemaParam.h"

#include <AST/ASTFunction.h>
#include <AST/ASTParam.h>
#include <AST/ASTType.h>
#include <AST/ASTVar.h>
#include <CodeGen/CodeGenFunctionBase.h>
#include <Sema/Helper.h>
#include <Sema/SemaErrorHandler.h>
#include <Sema/SemaType.h>

using namespace fly;

SemaFunctionBase::SemaFunctionBase(ASTFunction &AST, SemaKind Kind, std::string MangledName) : SemaNode(Kind),
	AST(AST), MangledName(MangledName), ErrorHandler(new SemaErrorHandler(nullptr)) {

}

SemaFunctionBase::~SemaFunctionBase() {
	// Delete all parameters
	for (auto *Param : Params) {
		delete Param;
	}

	// Delete all local variables
	for (auto *LocalVar : LocalVars) {
		delete LocalVar;
	}

	// Delete ErrorHandler
	delete ErrorHandler;
}

// Function to mangle a type reference
std::string SemaFunctionBase::MangleFunction(ASTFunction &AST) {
	llvm::SmallVector<SemaType *, 8> Params;
	for (auto Param : AST.getParams()) {
		Params.push_back(Param->getType()->getSema());
	}
	return Helper::MangleFunction(AST.getName(), Params);
}

std::string SemaFunctionBase::getMangledName() const {
	return MangledName;
}

void SemaFunctionBase::setReturnType(SemaType *RetType) {
	ReturnType = RetType;
}

llvm::SmallVector<SemaParam *, 8> &SemaFunctionBase::getParams() {
    return Params;
}

void SemaFunctionBase::addParam(SemaParam *Param) {
	Params.push_back(Param);
}

SemaType *SemaFunctionBase::getReturnType() {
    return ReturnType;
}

ASTFunction &SemaFunctionBase::getAST() {
	return AST;
}

llvm::SmallVector<SemaLocalVar *, 8> SemaFunctionBase::getLocalVars() {
	return LocalVars;
}

void SemaFunctionBase::addLocalVar(SemaLocalVar *Var) {
	LocalVars.push_back(Var);
}

SemaErrorHandler * SemaFunctionBase::getErrorHandler() const {
	return ErrorHandler;
}
