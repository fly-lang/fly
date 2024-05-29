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
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTStmt.h"
#include "AST/ASTType.h"
#include "Sema/SemaBuilder.h"

using namespace fly;

ASTExpr::ASTExpr(const SourceLocation &Loc, ASTExprKind Kind) :
        ASTBase(Loc), Kind(Kind) {

}

ASTExprKind ASTExpr::getExprKind() const {
    return Kind;
}

ASTType *ASTExpr::getType() const {
    return Type;
}

std::string ASTExpr::str() const {
    return Logger("ASTExpr").
           Super(ASTBase::str()).
           Attr("Kind", (uint64_t) Kind).
           Attr("Type", Type).
           End();
}

ASTEmptyExpr::ASTEmptyExpr(const SourceLocation &Loc) : ASTExpr(Loc, ASTExprKind::EXPR_EMPTY) {

}

std::string ASTEmptyExpr::str() const {
    return Logger("ASTEmptyExpr").End();
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

//ASTType *ASTVarRefExpr::getType() const {
//    return Type ? Type : VarRef->getDef() ? VarRef->getDef()->getType() : nullptr;
//}

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

//ASTType *ASTCallExpr::getType() const {
//    return Type ? Type : Call->getDef() ? Call->getDef()->getType() : nullptr;
//}

std::string ASTCallExpr::str() const {
    return Logger("ASTCallExpr").
           Super(ASTExpr::str()).
           Attr("Call", Call->str()).
           End();
}
