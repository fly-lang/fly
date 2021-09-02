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
    return nullptr;
}

ExprKind ASTOperatorExpr::getKind() const {
    return Kind;
}

ASTUnaryExpr::ASTUnaryExpr(const SourceLocation &Loc, ASTOperatorExpr *OperatorExpr, ASTVarRef *VarRef,
                           UnaryOpKind UKind) : OperatorExpr(OperatorExpr), VarRef(VarRef), UKind(UKind),
                                                ASTOperatorExpr(Loc){

}

OpKind ASTUnaryExpr::getOpKind() {
    return OperatorExpr->getOpKind();
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

ASTBinaryExpr::ASTBinaryExpr(const SourceLocation &Loc) : ASTOperatorExpr(Loc) {

}

ASTArithExpr::ASTArithExpr(const SourceLocation &Loc, const ArithOpKind &AKind) : ASTBinaryExpr(Loc),
    ArithKind(AKind) {

}

OpKind ASTArithExpr::getOpKind() {
return OperatorKind;
}

ArithOpKind ASTArithExpr::getArithKind() const {
    return ArithKind;
}

ASTLogicExpr::ASTLogicExpr(const SourceLocation &Loc, const LogicOpKind &LKind) : ASTBinaryExpr(Loc), LogicKind(LKind) {
    assert(LKind == LogicOpKind::LOGIC_AND || LKind == LogicOpKind::LOGIC_OR && "Only && or ||");
}

OpKind ASTLogicExpr::getOpKind() {
    return OperatorKind;
}

LogicOpKind ASTLogicExpr::getLogicKind() const {
    return LogicKind;
}

ASTComparisonExpr::ASTComparisonExpr(const SourceLocation &Loc, const ComparisonOpKind &CKind) : ASTBinaryExpr(Loc),
    ComparisonKind(CKind) {}

OpKind ASTComparisonExpr::getOpKind() {
    return OperatorKind;
}

ComparisonOpKind ASTComparisonExpr::getComparisonKind() const {
    return ComparisonKind;
}

ASTTernaryExpr::ASTTernaryExpr(const SourceLocation &Loc, const CondOpKind &CondKind) : ASTOperatorExpr(Loc),
                                                                                        CondKind(CondKind) {

}

OpKind ASTTernaryExpr::getOpKind() {
    return OperatorKind;
}

CondOpKind ASTTernaryExpr::getCondKind() const {
    return CondKind;
}
