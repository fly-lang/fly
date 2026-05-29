//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaClassAttribute.cpp - class attribute semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaClassAttribute.h"

#include "AST/ASTAttribute.h"
#include "Basic/Logger.h"
#include "Sema/SemaExpr.h"
#include "Sema/SemaType.h"
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

std::string SemaClassAttribute::str() const {
	return Logger("SemaClassAttribute")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.Attr("Visibility", static_cast<uint64_t>(getVisibility()))
		.Attr("Static", isStatic())
		.Attr("Constant", isConstant())
		.Attr("Type", Type)
		.End();
}

