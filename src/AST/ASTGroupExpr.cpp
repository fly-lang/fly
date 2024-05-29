//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTGroupExpr.cpp - AST Group Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTGroupExpr.h"

using namespace fly;

ASTGroupExpr::ASTGroupExpr(const SourceLocation &Loc, ASTExprGroupKind GroupKind) :
                           ASTExpr(Loc, ASTExprKind::EXPR_GROUP), GroupKind(GroupKind) {

}

ASTExprGroupKind ASTGroupExpr::getGroupKind() {
    return GroupKind;
}

ASTUnaryGroupExpr::ASTUnaryGroupExpr(const SourceLocation &Loc, ASTUnaryOperatorExpr *Operator, ASTVarRefExpr *First) :
        ASTGroupExpr(Loc, ASTExprGroupKind::GROUP_UNARY), Operator(Operator), First(First) {

}

ASTUnaryOperatorExpr *ASTUnaryGroupExpr::getOperator() const {
    return Operator;
}

const ASTVarRefExpr *ASTUnaryGroupExpr::getFirst() const {
    return First;
}

std::string ASTUnaryGroupExpr::str() const {
    return Logger("ASTUnaryGroupExpr").
           Super(ASTGroupExpr::str()).
           Attr("First", (ASTBase *) First).
           Attr("Operator", (uint64_t) Operator).
           End();
}

ASTBinaryGroupExpr::ASTBinaryGroupExpr(const SourceLocation &Loc, ASTBinaryOperatorExpr *Operator,
                                       ASTExpr *First, ASTExpr *Second) :
        ASTGroupExpr(Loc, ASTExprGroupKind::GROUP_BINARY),
        Operator(Operator),
        First(First),
        Second(Second) {

}

ASTBinaryOperatorExpr *ASTBinaryGroupExpr::getOperator() const {
    return Operator;
}

const ASTExpr *ASTBinaryGroupExpr::getFirst() const {
    return First;
}

const ASTExpr *ASTBinaryGroupExpr::getSecond() const {
    return Second;
}

std::string ASTBinaryGroupExpr::str() const {
    return Logger("ASTBinaryGroupExpr").
           Super(ASTGroupExpr::str()).
           Attr("First", First).
           Attr("Operator", (uint64_t) Operator).
           Attr("Second=", Second).
           End();
}

ASTTernaryGroupExpr::ASTTernaryGroupExpr(const SourceLocation &Loc, ASTExpr *First,
                                         ASTTernaryOperatorExpr *FirstOperator, ASTExpr *Second,
                                         ASTTernaryOperatorExpr *SecondOperator, ASTExpr *Third) :
                                         ASTGroupExpr(Loc, ASTExprGroupKind::GROUP_TERNARY),
                                         First(First), Second(Second), Third(Third) {

}

ASTTernaryOperatorExpr *ASTTernaryGroupExpr::getFirstOperator() const {
    return FirstOperator;
}

ASTTernaryOperatorExpr *ASTTernaryGroupExpr::getSecondOperator() const {
    return SecondOperator;
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