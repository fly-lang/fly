//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaClassAttribute.cpp - The Symbolic Table for Class Attribute
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaClassAttribute.h"

#include "AST/ASTAttribute.h"
#include "Sema/SemaExpr.h"
#include "Sema/SemaVisibilityKind.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaClassAttribute::SemaClassAttribute(ASTAttribute &AST, SemaClassType &Class, SemaType *Type) :
	SemaVar(&AST, SemaKind::ATTRIBUTE, Type), Class(Class), Visibility(SemaVisibilityKind::DEFAULT) {

}

SemaClassAttribute::~SemaClassAttribute() {
	// Delete Comment if present
	delete Comment;
}

SemaClassType &SemaClassAttribute::getClass() const {
	return Class;
}

SemaComment * SemaClassAttribute::getComment() const {
	return Comment;
}

SemaVisibilityKind SemaClassAttribute::getVisibility() const {
    return Visibility;
}

bool SemaClassAttribute::isStatic() const {
	return Static;
}

SemaClassType * SemaClassAttribute::getInherited() const {
	return Inherited;
}

SemaExpr * SemaClassAttribute::getInitExpr() const {
	return InitExpr;
}

void SemaClassAttribute::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

