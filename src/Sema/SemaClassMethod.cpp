//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymClassMethod.cpp - The Symbolic Table for Enum Entry
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaClassMethod.h"
#include "Sema/SemaVisitor.h"

#include "Sema/Helper.h"

#include <AST/ASTMethod.h>

using namespace fly;

SemaClassMethod::SemaClassMethod(ASTMethod &AST, SemaClassType *Class, SemaClassInstance *This,
	bool Constructor, SymbolTable *Scope) :
	SemaFunctionBase(AST, SemaKind::METHOD, Scope), Class(Class), This(This),
	Constructor(Constructor), Abstract(!Constructor && AST.getBody() == nullptr) {

}

SemaClassMethod::~SemaClassMethod() {
	// Delete Comment if present
	delete Comment;
	// Delete CodeGen if present
	delete CodeGen;

	// Note: Params and LocalVars are deleted by SemaFunctionBase destructor
}

SemaClassType *SemaClassMethod::getClass() const {
	return Class;
}

SemaClassInstance * SemaClassMethod::getThis() const {
	return This;
}

bool SemaClassMethod::isConstructor() const {
	return Constructor;
}

bool SemaClassMethod::isAbstract() const {
	return Abstract;
}

SemaVisibilityKind SemaClassMethod::getVisibility() const {
	return Visibility;
}

bool SemaClassMethod::isStatic() const {
	return Static;
}

bool SemaClassMethod::isFinal() const {
	return Final;
}

SemaClassMethod * SemaClassMethod::getOverridden() const {
    return Overridden;
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

void SemaClassMethod::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

