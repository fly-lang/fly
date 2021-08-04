//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTOperatorExpr.cpp - AST Operator Expression
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTOperatorExpr.h"

using namespace fly;

ASTOperatorExpr::ASTOperatorExpr(const SourceLocation &Loc) : ASTExpr(Loc) {

}

ASTType *ASTOperatorExpr::getType() const {
    assert(0 && "Unknown Type from Operator Expr");
}

ExprKind ASTOperatorExpr::getKind() const {
    return Kind;
}

ASTArithExpr::ASTArithExpr(const SourceLocation &Loc, const ArithOpKind &AKind) : ASTOperatorExpr(Loc),
    ArithKind(AKind) {

}

OpKind ASTArithExpr::getOpKind() {
return OperatorKind;
}

ArithOpKind ASTArithExpr::getArithKind() const {
    return ArithKind;
}

ASTLogicExpr::ASTLogicExpr(const SourceLocation &Loc, const LogicOpKind &LKind) : ASTOperatorExpr(Loc), LogicKind(LKind) {
    assert(LKind == LogicOpKind::LOGIC_AND || LKind == LogicOpKind::LOGIC_OR && "Only && or ||");
}

OpKind ASTLogicExpr::getOpKind() {
    return OperatorKind;
}

LogicOpKind ASTLogicExpr::getLogicKind() const {
    return LogicKind;
}

ASTComparisonExpr::ASTComparisonExpr(const SourceLocation &Loc, const ComparisonOpKind &CKind) : ASTOperatorExpr(Loc), ComparisonKind(CKind) {}

OpKind ASTComparisonExpr::getOpKind() {
    return OperatorKind;
}

ComparisonOpKind ASTComparisonExpr::getComparisonKind() const {
    return ComparisonKind;
}

ASTCondExpr::ASTCondExpr(const SourceLocation &Loc, const CondOpKind &CondKind) : ASTOperatorExpr(Loc),
    CondKind(CondKind) {

}

OpKind ASTCondExpr::getOpKind() {
    return OperatorKind;
}

CondOpKind ASTCondExpr::getCondKind() const {
    return CondKind;
}

ASTUnaryExpr::ASTUnaryExpr(const SourceLocation &Loc, ASTOperatorExpr *OperatorExpr, ASTVarRef *VarRef,
                           UnaryOpKind UKind) : OperatorExpr(OperatorExpr), VarRef(VarRef), UKind(UKind),
                           ASTOperatorExpr(Loc){

}

OpKind ASTUnaryExpr::getOpKind() {
    return OperatorKind;
}

UnaryOpKind ASTUnaryExpr::getUnaryKind() const {
    return UKind;
}

ASTOperatorExpr *ASTUnaryExpr::getOperatorExpr() const {
    return OperatorExpr;
}

ASTVarRef *ASTUnaryExpr::getVarRef() const {
    return VarRef;
}
