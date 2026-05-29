//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaFunction.cpp - function semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaFunction.h"
#include "Sema/SemaVisitor.h"
#include "Sema/Helper.h"
#include "Sema/SemaParam.h"
#include "Sema/SemaBlockStmt.h"
#include "Sema/SemaType.h"
#include "Basic/Logger.h"

using namespace fly;

SemaFunction::SemaFunction(ASTFunction &AST, SymbolTable *Scope) :
	SemaFunctionBase(AST, SemaKind::FUNCTION, Scope) {

}

SemaFunction::~SemaFunction() {
	// Delete Comment if present
	delete Comment;
	// Delete CodeGen if present
	delete CodeGen;
}

SemaModule *SemaFunction::getModule() const {
	return Module;
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

llvm::StringRef SemaFunction::getName() const {
	if (!MangledName.empty())
		return MangledName;
	return SemaFunctionBase::getName();
}

CodeGenFunction *SemaFunction::getCodeGen() const {
	return CodeGen;
}

void SemaFunction::setCodeGen(CodeGenFunction *CGF) {
	CodeGen = CGF;
}

void SemaFunction::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaFunction::str() const {
	return Logger("SemaFunction")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.Attr("NamespaceName", getNamespaceName())
		.Attr("Visibility", static_cast<uint64_t>(getVisibility()))
		.Attr("ReturnType", const_cast<SemaFunction*>(this)->getReturnType())
		.Attr("Params", const_cast<SemaFunction*>(this)->getParams())
		.Attr("Fallible", isFallible())
		.Attr("Body", getBody())
		.End();
}

