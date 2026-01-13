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

SemaClassMethod::SemaClassMethod(ASTMethod &AST, SemaClassType *Class, SemaClassInstance *This, SemaClassMethodKind MethodKind) :
	SemaFunctionBase(AST, SemaKind::METHOD), Class(Class), This(This), MethodKind(MethodKind) {

}

SemaClassMethod::~SemaClassMethod() {
	// Delete Comment if present
	delete Comment;

	// Note: Params and LocalVars are deleted by SemaFunctionBase destructor
}

SemaClassType *SemaClassMethod::getClass() const {
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

