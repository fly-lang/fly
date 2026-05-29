//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTExpr.cpp - AST expression base implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTExpr.h"
#include "Basic/Logger.h"

using namespace fly;

ASTExpr::ASTExpr(const SourceLocation &Loc, ASTExprKind ExprKind, ASTExpr *Parent, ASTExpr *Child) :
        ASTNode(Loc, ASTKind::AST_EXPR), ExprKind(ExprKind) {
	setParent(Parent);
	setChild(Child);
}

ASTExprKind ASTExpr::getExprKind() const { return ExprKind; }

void ASTExpr::setParent(ASTExpr *P) {
	this->Parent = P;
	if (P != nullptr) P->Child = this;
}

void ASTExpr::setChild(ASTExpr *C) {
	this->Child = C;
	if (C) C->Parent = this;
}

ASTExpr *ASTExpr::getParent() const { return Parent; }
ASTExpr *ASTExpr::getChild() const  { return Child; }

std::string ASTExpr::str() const {
    return Logger("ASTExpr").
		Attr("Location", getLocation()).
		Attr("Kind", static_cast<size_t>(getKind())).
           End();
}


