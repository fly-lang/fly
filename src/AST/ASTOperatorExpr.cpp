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

ASTLogicExpr::ASTLogicExpr(const SourceLocation &Loc, const LogicOpKind &BKind) : ASTOperatorExpr(Loc), BoolKind(BKind) {

}

OpKind ASTLogicExpr::getOpKind() {
    return OperatorKind;
}

LogicOpKind ASTLogicExpr::getBoolKind() const {
    return BoolKind;
}

ASTComparisonExpr::ASTComparisonExpr(const SourceLocation &Loc, const ComparisonOpKind &LKind) : ASTOperatorExpr(Loc), ComparisonKind(LKind) {}

OpKind ASTComparisonExpr::getOpKind() {
    return OperatorKind;
}

ComparisonOpKind ASTComparisonExpr::getComparisonKind() const {
    return ComparisonKind;
}

ASTIncDecExpr::ASTIncDecExpr(const SourceLocation &Loc, const IncDecOpKind &Kind) : ASTOperatorExpr(Loc),
    IncDecKind(Kind) {

}

OpKind ASTIncDecExpr::getOpKind() {
    return OperatorKind;
}

IncDecOpKind ASTIncDecExpr::getIncDecKind() const {
    return IncDecKind;
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