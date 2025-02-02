//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTExpr.cpp - AST Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTExpr.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTValue.h"
#include "AST/ASTCall.h"
#include "AST/ASTTypeRef.h"

using namespace fly;

ASTExpr::ASTExpr(const SourceLocation &Loc, ASTExprKind Kind) :
        ASTBase(Loc, ASTKind::AST_EXPR), Kind(Kind) {

}

ASTExprKind ASTExpr::getExprKind() const {
    return Kind;
}

ASTTypeRef *ASTExpr::getTypeRef() const {
    return TypeRef;
}

std::string ASTExpr::str() const {
    return Logger("ASTExpr").
           Super(ASTBase::str()).
           Attr("Kind", (uint64_t) Kind).
           Attr("TypeRef", TypeRef).
           End();
}

ASTValueExpr::ASTValueExpr(ASTValue *Val) : ASTExpr(Val->getLocation(), ASTExprKind::EXPR_VALUE), Value(Val) {

}

ASTValue *ASTValueExpr::getValue() const {
    return Value;
}

std::string ASTValueExpr::str() const {
    return
            Logger("ASTValueExpr").
            Super(ASTExpr::str()).
            Attr("Value", Value).
            End();
}

ASTVarRefExpr::ASTVarRefExpr(ASTVarRef *VarRef) : ASTExpr(VarRef->getLocation(), ASTExprKind::EXPR_VAR_REF), VarRef(VarRef) {

}

ASTVarRef *ASTVarRefExpr::getVarRef() const {
    return VarRef;
}

std::string ASTVarRefExpr::str() const {
    return Logger("ASTVarRefExpr").
           Super(ASTExpr::str()).
            Attr("VarRef", VarRef).
            End();
}

ASTCallExpr::ASTCallExpr(ASTCall *Call) :
        ASTExpr(Call->getLocation(), ASTExprKind::EXPR_CALL), Call(Call) {

}

ASTCall *ASTCallExpr::getCall() const {
    return Call;
}

std::string ASTCallExpr::str() const {
    return Logger("ASTCallExpr").
           Super(ASTExpr::str()).
           Attr("Call", Call->str()).
           End();
}
