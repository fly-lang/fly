//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTCast.cpp - AST type cast expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTCast.h"
#include "Basic/Logger.h"

#include <AST/ASTType.h>
#include <AST/ASTVisitor.h>

using namespace fly;

ASTCast::ASTCast(ASTExpr *Expr, ASTType *ToType) : ASTExpr(ToType->getLocation(), ASTExprKind::EXPR_CAST),
	Expr(Expr), ToType(ToType) {
}

void ASTCast::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTExpr * ASTCast::getExpr() const {
	return Expr;
}

ASTType * ASTCast::getToType() const {
	return ToType;
}

std::string ASTCast::str() const {
    return Logger("ASTCast")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("Expr", Expr)
        .Attr("ToType", ToType)
        .End();
}
