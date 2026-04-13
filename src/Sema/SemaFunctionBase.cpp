//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaFunctionBase.cpp - The Symbolic table of Function Base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaFunctionBase.h"

#include "AST/ASTFunction.h"
#include "Sema/SemaBuilder.h"
#include "Sema/SemaBuiltin.h"
#include "Sema/SemaLocalVar.h"
#include "Sema/SemaParam.h"
#include "Sema/SemaBlockStmt.h"

#include <AST/ASTVar.h>
#include <CodeGen/CodeGenFunctionBase.h>
#include <Sema/SemaError.h>
#include <Sema/SemaType.h>

using namespace fly;

SemaFunctionBase::SemaFunctionBase(ASTFunction &AST, SemaKind Kind, SymbolTable *Symbols) : SemaNode(Kind),
	AST(AST), Scope(Symbols), ReturnType(SemaBuiltin::getVoidType()), ErrorHandler(SemaBuilder::CreateErrorHandler()), Fallible(false) {

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

SymbolTable* SemaFunctionBase::getSymbols() const {
	return Scope;
}

llvm::StringRef SemaFunctionBase::getName() const {
	return AST.getName();
}

SemaType *SemaFunctionBase::getReturnType() {
	return ReturnType;
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

const ASTFunction &SemaFunctionBase::getAST() const {
	return AST;
}

llvm::SmallVector<SemaLocalVar *, 8> SemaFunctionBase::getLocalVars() {
	return LocalVars;
}

void SemaFunctionBase::addLocalVar(SemaLocalVar *Var) {
	LocalVars.push_back(Var);
}

SemaError * SemaFunctionBase::getErrorHandler() const {
	return ErrorHandler;
}

bool SemaFunctionBase::isFallible() const { return Fallible; }

void SemaFunctionBase::setFallible(bool F) { Fallible = F; }

SemaBlockStmt *SemaFunctionBase::getBody() const { return Body; }

void SemaFunctionBase::setBody(SemaBlockStmt *B) { Body = B; }
