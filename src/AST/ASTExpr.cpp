//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTExpr.cpp - Expression into a statement
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
#include "AST/ASTFunctionCall.h"
#include "AST/ASTStmt.h"
#include "AST/ASTType.h"
#include "Sema/SemaBuilder.h"

using namespace fly;

ASTExpr::ASTExpr(const SourceLocation &Loc, ASTExprKind Kind) : Loc(Loc), Kind(Kind) {

}

const SourceLocation &ASTExpr::getLocation() const {
    return Loc;
}

ASTExprKind ASTExpr::getExprKind() const {
    return Kind;
}

ASTType *ASTExpr::getType() const {
    return Type;
}

ASTStmt *ASTExpr::getStmt() {
    return Stmt;
}

std::string ASTExpr::str() const {
    return Logger("ASTExpr").
           Attr("Loc", Loc).
           Attr("Kind", (int) Kind).
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

ASTValue &ASTValueExpr::getValue() const {
    return *Value;
}

std::string ASTValueExpr::str() const {
    return
            Logger("ASTValueExpr").
            Super(ASTExpr::str()).
            Attr("Value", Value).
            End();
}

ASTVarRefExpr::ASTVarRefExpr(ASTVarRef *VarRef) : ASTExpr(VarRef->getLocation(), ASTExprKind::EXPR_REF_VAR), VarRef(VarRef) {

}

ASTVarRef *ASTVarRefExpr::getVarRef() const {
    return VarRef;
}

ASTType *ASTVarRefExpr::getType() const {
    return Type ? Type : VarRef->getDef() ? VarRef->getDef()->getType() : nullptr;
}

std::string ASTVarRefExpr::str() const {
    return Logger("ASTVarRefExpr").
           Super(ASTExpr::str()).
            Attr("VarRef", VarRef).
            End();
}

ASTFunctionCallExpr::ASTFunctionCallExpr(ASTFunctionCall *Call) :
    ASTExpr(Call->getLocation(), ASTExprKind::EXPR_REF_FUNC), Call(Call) {

}

ASTFunctionCall *ASTFunctionCallExpr::getCall() const {
    return Call;
}

ASTType *ASTFunctionCallExpr::getType() const {
    return Type ? Type : Call->getDef() ? Call->getDef()->getType() : nullptr;
}

std::string ASTFunctionCallExpr::str() const {
    return Logger("ASTFunctionCallExpr").
           Super(ASTExpr::str()).
           Attr("Call", Call->str()).
           End();
}

ASTGroupExpr::ASTGroupExpr(const SourceLocation &Loc, ASTExprGroupKind GroupKind) :
                           ASTExpr(Loc, ASTExprKind::EXPR_GROUP), GroupKind(GroupKind) {

}

ASTExprGroupKind ASTGroupExpr::getGroupKind() {
    return GroupKind;
}

std::string ASTGroupExpr::str() const {
    return Logger("ASTGroupExpr").
            Attr("GroupKind", (int) GroupKind).
            End();
}

ASTUnaryGroupExpr::ASTUnaryGroupExpr(const SourceLocation &Loc, ASTUnaryOperatorKind Operator,
                                     ASTUnaryOptionKind Option, ASTVarRefExpr *First) :
        ASTGroupExpr(Loc, ASTExprGroupKind::GROUP_UNARY), OperatorKind(Operator), OptionKind(Option), First(First) {

}

ASTUnaryOperatorKind ASTUnaryGroupExpr::getOperatorKind() const {
    return OperatorKind;
}

ASTUnaryOptionKind ASTUnaryGroupExpr::getOptionKind() const {
    return OptionKind;
}

const ASTVarRefExpr *ASTUnaryGroupExpr::getFirst() const {
    return First;
}

ASTType *ASTUnaryGroupExpr::getType() const {
    return Type ? Type : First->getType();
}

std::string ASTUnaryGroupExpr::str() const {
    return Logger("ASTUnaryGroupExpr").
           Super(ASTGroupExpr::str()).
           Attr("First", (Debuggable *) First).
           Attr("Operator", (int) OperatorKind).
           Attr("Option", (int) OptionKind).
           End();
}

ASTBinaryGroupExpr::ASTBinaryGroupExpr(const SourceLocation &OpLoc,
                                       ASTBinaryOperatorKind Operator, ASTExpr *First, ASTExpr *Second) :
        ASTGroupExpr(First->getLocation(), ASTExprGroupKind::GROUP_BINARY),
        OpLoc(OpLoc),
        OperatorKind(Operator),
        OptionKind((int) Operator < 300 ? ((int) Operator < 200 ? ASTBinaryOptionKind::BINARY_ARITH : ASTBinaryOptionKind::BINARY_LOGIC) : ASTBinaryOptionKind::BINARY_COMPARISON),
        First(First),
        Second(Second) {

}

ASTBinaryOperatorKind ASTBinaryGroupExpr::getOperatorKind() const {
    return OperatorKind;
}

ASTBinaryOptionKind ASTBinaryGroupExpr::getOptionKind() const {
    return OptionKind;
}

const ASTExpr *ASTBinaryGroupExpr::getFirst() const {
    return First;
}

const ASTExpr *ASTBinaryGroupExpr::getSecond() const {
    return Second;
}

ASTType *ASTBinaryGroupExpr::getType() const {
    if (Type) {
        return Type;
    }

    switch (OptionKind) {

        case ASTBinaryOptionKind::BINARY_ARITH:
            return First->getType();
        case ASTBinaryOptionKind::BINARY_LOGIC:
        case ASTBinaryOptionKind::BINARY_COMPARISON:
            return SemaBuilder::CreateBoolType(SourceLocation());
    }
}

std::string ASTBinaryGroupExpr::str() const {
    return Logger("ASTBinaryGroupExpr").
           Super(ASTGroupExpr::str()).
           Attr("First", First).
           Attr("Operator", (int) OperatorKind).
           Attr("Second=", Second).
           End();
}

ASTTernaryGroupExpr::ASTTernaryGroupExpr(ASTExpr *First, const SourceLocation &IfLoc, ASTExpr *Second,
                                         const SourceLocation &ElseLoc, ASTExpr *Third) :
                                         ASTGroupExpr(First->getLocation(), ASTExprGroupKind::GROUP_TERNARY),
                                         First(First), IfLoc(IfLoc), Second(Second), ElseLoc(ElseLoc), Third(Third) {

}

ASTTernaryOperatorKind ASTTernaryGroupExpr::getOperatorKind() const {
    return OperatorKind;
}

ASTType *ASTTernaryGroupExpr::getType() const {
    return Type ? Type : Second->getType();
}

const ASTExpr *ASTTernaryGroupExpr::getFirst() const {
    return First;
}

const ASTExpr *ASTTernaryGroupExpr::getSecond() const {
    return Second;
}

const ASTExpr *ASTTernaryGroupExpr::getThird() const {
    return Third;
}

std::string ASTTernaryGroupExpr::str() const {
    return Logger("ASTTernaryGroupExpr").
           Super(ASTGroupExpr::str()).
           Attr("First", First).
           Attr("Second", Second).
           Attr("Third", Third).
           End();
}