//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTVarRefExpr.cpp - AST VarRef Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTVarRefExpr.h"

using namespace fly;

ASTVarRefExpr::ASTVarRefExpr(ASTRef *VarRef) : ASTExpr(VarRef->getLocation(), ASTExprKind::EXPR_VAR_REF), VarRef(VarRef) {

}

ASTRef *ASTVarRefExpr::getVarRef() const {
    return VarRef;
}

std::string ASTVarRefExpr::str() const {
    return Logger("ASTVarRefExpr").
	Attr("Location", getLocation()).
 Attr("Kind", static_cast<size_t>(getKind())).
            Attr("VarRef", VarRef).
            End();
}
