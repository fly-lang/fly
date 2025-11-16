//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTExpr.cpp - AST Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTExpr.h"
#include "Sema/SemaType.h"
#include "Basic/Logger.h"

using namespace fly;

ASTExpr::ASTExpr(const SourceLocation &Loc, ASTExprKind ExprKind, ASTExpr *Parent, ASTExpr *Child) :
        ASTNode(Loc, ASTKind::AST_EXPR), ExprKind(ExprKind), Parent(nullptr), Child(nullptr) {

}

ASTExprKind ASTExpr::getExprKind() const {
    return ExprKind;
}

void ASTExpr::setParent(ASTExpr *Parent) {
	this->Parent = Parent;
	Parent->Child = this;
}


void ASTExpr::setChild(ASTExpr *Identifier) {
	this->Child = Identifier;
	Child->Parent = this;
}

ASTExpr *ASTExpr::getParent() const {
	return Parent;
}

ASTExpr *ASTExpr::getChild() const {
	return Child;
}


SemaType *ASTExpr::getType() const {
    return Type;
}

void ASTExpr::setType(SemaType *Type) {
	this->Type = Type;
}

std::string ASTExpr::str() const {
    return Logger("ASTExpr").
		Attr("Location", getLocation()).
		Attr("Kind", static_cast<size_t>(getKind())).
           Attr("Type", Type).
           End();
}
