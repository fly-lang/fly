//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTCallExpr.cpp - AST Call Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTCallExpr.h"
#include "AST/ASTCall.h"

using namespace fly;

ASTCallExpr::ASTCallExpr(ASTCall *Call) :
        ASTExpr(Call->getLocation(), ASTExprKind::EXPR_CALL), Call(Call) {

}

ASTCall *ASTCallExpr::getCall() const {
    return Call;
}

std::string ASTCallExpr::str() const {
    return Logger("ASTCallExpr").
	Attr("Location", getLocation()).
	Attr("Kind", static_cast<size_t>(getKind())).
	Attr("Call", Call->str()).
	End();
}
