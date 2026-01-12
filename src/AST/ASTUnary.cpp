//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTUnary.cpp - AST Unary Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTUnary.h"
#include "AST/ASTVisitor.h"
#include "Basic/Logger.h"

using namespace fly;

ASTUnary::ASTUnary(const SourceLocation &Loc, ASTUnaryKind OpKind, ASTExpr *Expr) :
	ASTExpr(Loc, ASTExprKind::EXPR_UNARY), OpKind(OpKind), Expr(Expr) {
}

void ASTUnary::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTUnaryKind ASTUnary::getOpKind() const {
    return OpKind;
}

const SourceLocation &ASTUnary::getOpLocation() const {
    return ASTBase::getLocation();
}

ASTExpr *ASTUnary::getExpr() const {
    return Expr;
}

SemaUnary *ASTUnary::getSema() const {
	return static_cast<SemaUnary *>(Sema);
}

void ASTUnary::setSema(SemaUnary *Sema) {
	this->Sema = Sema;
}

std::string ASTUnary::str() const {
    return Logger("ASTOp").
	Attr("Location", getLocation()).
Attr("Kind", static_cast<size_t>(getKind())).
           Attr("Expr", (ASTNode *) Expr).
           Attr("Op", (uint64_t) OpKind).
           End();
}
