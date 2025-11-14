//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTExpr.cpp - AST Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTExpr.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTValue.h"
#include "AST/ASTCall.h"
#include "Basic/Logger.h"
#include "AST/ASTType.h"

#include <AST/ASTCallExpr.h>
#include <AST/ASTCastExpr.h>
#include <AST/ASTValueExpr.h>
#include <AST/ASTVarRefExpr.h>

using namespace fly;

ASTExpr::ASTExpr(const SourceLocation &Loc, ASTExprKind ExprKind) :
        ASTNode(Loc, ASTKind::AST_EXPR), ExprKind(ExprKind) {

}

ASTExprKind ASTExpr::getExprKind() const {
    return ExprKind;
}

SemaType *ASTExpr::getType() const {
    return Type;
}

void ASTExpr::setType(SemaType *Type) {
	ASTExpr::Type = Type;
}

std::string ASTExpr::str() const {
    return Logger("ASTExpr").
		Attr("Location", getLocation()).
		Attr("Kind", static_cast<size_t>(getKind())).
           Attr("Type", Type).
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
		Attr("Location", getLocation()).
		Attr("Kind", static_cast<size_t>(getKind())).
		Attr("Value", Value).
        End();
}

ASTVarRefExpr::ASTVarRefExpr(ASTIdentifier *VarRef) : ASTExpr(VarRef->getLocation(), ASTExprKind::EXPR_VAR_REF), VarRef(VarRef) {

}

ASTIdentifier *ASTVarRefExpr::getVarRef() const {
    return VarRef;
}

std::string ASTVarRefExpr::str() const {
    return Logger("ASTVarRefExpr").
	Attr("Location", getLocation()).
 Attr("Kind", static_cast<size_t>(getKind())).
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
	Attr("Location", getLocation()).
	Attr("Kind", static_cast<size_t>(getKind())).
	Attr("Call", Call->str()).
	End();
}

ASTCastExpr::ASTCastExpr(ASTExpr *Expr, ASTType *Cast) : ASTExpr(Cast->getLocation(), ASTExprKind::EXPR_CAST),
	Expr(Expr), TypeRef(Cast) {
}

ASTExpr * ASTCastExpr::getExpr() const {
	return Expr;
}

ASTType * ASTCastExpr::getTypeRef() const {
	return TypeRef;
}

std::string ASTCastExpr::str() const {
	return ASTExpr::str();
}
