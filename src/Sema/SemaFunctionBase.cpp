//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaFunctionBase.cpp - The Symbolic table of Function Base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaFunctionBase.h"
#include "Sema/SemaVisitor.h"

#include "AST/ASTFunction.h"
#include "Sema/SemaLocalVar.h"
#include "Sema/SemaParam.h"

#include <AST/ASTVar.h>
#include <CodeGen/CodeGenFunctionBase.h>
#include <Sema/SemaErrorHandler.h>
#include <Sema/SemaType.h>

using namespace fly;

SemaFunctionBase::SemaFunctionBase(ASTFunction &AST, SemaKind Kind) : SemaNode(Kind),
	AST(AST), ErrorHandler(new SemaErrorHandler(nullptr)) {

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

llvm::StringRef SemaFunctionBase::getName() const {
	return AST.getName();
}

SemaType *SemaFunctionBase::getReturnType() {
	return ReturnType;
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

void SemaFunctionBase::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

