//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaFunctionBase.cpp - function base semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaFunctionBase.h"
#include "Basic/Logger.h"

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
	for (auto *Param : Params)
		delete Param;

	for (auto *LocalVar : LocalVars)
		delete LocalVar;

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

void SemaFunctionBase::setReturnType(SemaType *T) {
	ReturnType = T;
}

const std::string &SemaFunctionBase::getNamespaceName() const {
	return NamespaceName;
}

void SemaFunctionBase::setNamespaceName(std::string NS) {
	NamespaceName = std::move(NS);
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

std::string SemaFunctionBase::str() const {
	return Logger("SemaFunctionBase")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.Attr("NamespaceName", getNamespaceName())
		.Attr("ReturnType", const_cast<SemaFunctionBase*>(this)->getReturnType())
		.Attr("Params", const_cast<SemaFunctionBase*>(this)->getParams())
		.Attr("Fallible", isFallible())
		.Attr("Body", getBody())
		.End();
}
