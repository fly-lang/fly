//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTExpr.cpp - Expression into a statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTExpr.h"
#include "AST/ASTVar.h"
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

ASTEmptyExpr::ASTEmptyExpr(const SourceLocation &Loc) : ASTExpr(Loc, ASTExprKind::EXPR_EMPTY) {

}

std::string ASTEmptyExpr::str() const {
    return "";
}

ASTValueExpr::ASTValueExpr(ASTValue *Val) : ASTExpr(Val->getLocation(), ASTExprKind::EXPR_VALUE), Value(Val) {

}

ASTValue &ASTValueExpr::getValue() const {
    return *Value;
}

std::string ASTValueExpr::str() const {
    return "{ Type=" + (getType() ? getType()->str() : "") +
           ", Kind=" + std::to_string((int) getExprKind()) +
           ", Value=" + Value->str() +
           " }";
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
    return "{ Type=" + (getType() ? getType()->str() : "") +
           ", Kind=" + std::to_string((int) getExprKind()) +
           ", VarRef=" + VarRef->str() +
           " }";
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
    return "{ Type=" + (getType() ? getType()->str() : "") +
           ", Kind=" + std::to_string((int) getExprKind()) +
           ", Call=" + Call->str() +
           " }";
}

ASTGroupExpr::ASTGroupExpr(const SourceLocation &Loc, ASTExprGroupKind GroupKind) :
                           ASTExpr(Loc, ASTExprKind::EXPR_GROUP), GroupKind(GroupKind) {

}

ASTExprGroupKind ASTGroupExpr::getGroupKind() {
    return GroupKind;
}

ASTUnaryGroupExpr::ASTUnaryGroupExpr(const SourceLocation &Loc, UnaryOpKind Operator,
                                     UnaryOptionKind Option, ASTVarRefExpr *First) :
        ASTGroupExpr(Loc, ASTExprGroupKind::GROUP_UNARY), OperatorKind(Operator), OptionKind(Option), First(First) {

}

UnaryOpKind ASTUnaryGroupExpr::getOperatorKind() const {
    return OperatorKind;
}

UnaryOptionKind ASTUnaryGroupExpr::getOptionKind() const {
    return OptionKind;
}

const ASTVarRefExpr *ASTUnaryGroupExpr::getFirst() const {
    return First;
}

ASTType *ASTUnaryGroupExpr::getType() const {
    return Type ? Type : First->getType();
}

std::string ASTUnaryGroupExpr::str() const {
    return "{ First=" + First->str() +
           ", Operator=" + std::to_string((int) OperatorKind) +
           ", Option=" + std::to_string((int) OptionKind) +
           ", Type=" + (getType() ? getType()->str() : "") +
           ", Kind=" + std::to_string((int) getExprKind());
}

ASTBinaryGroupExpr::ASTBinaryGroupExpr(const SourceLocation &OpLoc,
                                       BinaryOpKind Operator, ASTExpr *First, ASTExpr *Second) :
        ASTGroupExpr(First->getLocation(), ASTExprGroupKind::GROUP_BINARY),
        OpLoc(OpLoc),
        OperatorKind(Operator),
        OptionKind((int) Operator < 300 ? ((int) Operator < 200 ?  BinaryOptionKind::BINARY_ARITH : BinaryOptionKind::BINARY_LOGIC) : BinaryOptionKind::BINARY_COMPARISON),
        First(First),
        Second(Second) {

}

BinaryOpKind ASTBinaryGroupExpr::getOperatorKind() const {
    return OperatorKind;
}

BinaryOptionKind ASTBinaryGroupExpr::getOptionKind() const {
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

        case BinaryOptionKind::BINARY_ARITH:
            return First->getType();
        case BinaryOptionKind::BINARY_LOGIC:
        case BinaryOptionKind::BINARY_COMPARISON:
            return SemaBuilder::CreateBoolType(SourceLocation());
    }
}

std::string ASTBinaryGroupExpr::str() const {
    return "{ First=" + First->str() +
           ", Operator=" + std::to_string((int) OperatorKind) +
           ", Second=" + Second->str() +
           ", Type=" + (getType() ? getType()->str() : "") +
           ", Kind=" + std::to_string((int) getExprKind());
}

ASTTernaryGroupExpr::ASTTernaryGroupExpr(ASTExpr *First, const SourceLocation &IfLoc, ASTExpr *Second,
                                         const SourceLocation &ElseLoc, ASTExpr *Third) :
                                         ASTGroupExpr(First->getLocation(), ASTExprGroupKind::GROUP_TERNARY),
                                         First(First), IfLoc(IfLoc), Second(Second), ElseLoc(ElseLoc), Third(Third) {

}

TernaryOpKind ASTTernaryGroupExpr::getOperatorKind() const {
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
    return "{ First=" + First->str() +
           ", Second=" + Second->str() +
           ", Third=" + Third->str() +
           ", Type=" + (getType() ? getType()->str() : "") +
           ", Kind=" + std::to_string((int) getExprKind());
}