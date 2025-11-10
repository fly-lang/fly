//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTValueExpr.cpp - AST Value Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTValueExpr.h"

#include <AST/ASTValue.h>

using namespace fly;

ASTValueExpr::ASTValueExpr(ASTValue *Val) : ASTExpr(Val->getLocation(), ASTExprKind::EXPR_VALUE), Value(Val) {

}

ASTValue *ASTValueExpr::getValue() const {
    return Value;
}

std::string ASTValueExpr::str() const {
    return
		Logger("ASTValueExpr").
		Attr("Location", getLocation()).
		Attr("Kind", static_cast<size_t>(getKind())).
		Attr("Value", Value).
        End();
}
