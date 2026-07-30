//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTArrayAccess.cpp - AST array subscript expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTArrayAccess.h"
#include "AST/ASTVisitor.h"
#include "Basic/Logger.h"

using namespace fly;

ASTArrayAccess::ASTArrayAccess(const SourceLocation &Loc, ASTExpr *Base, ASTExpr *Index) :
	ASTExpr(Loc, ASTExprKind::EXPR_ARRAY_ACCESS), Base(Base), Index(Index) {
}

void ASTArrayAccess::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTExpr *ASTArrayAccess::getBase() const { return Base; }

ASTExpr *ASTArrayAccess::getIndex() const { return Index; }

std::string ASTArrayAccess::str() const {
	return Logger("ASTArrayAccess").
		Attr("Location", getLocation()).
		Attr("Kind", static_cast<size_t>(getKind())).
		Attr("Base", Base).
		Attr("Index", Index).
		End();
}
