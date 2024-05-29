//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTOperatorExpr.cpp - AST Operator Expr implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTOperatorExpr.h"

using namespace fly;

ASTOperatorExpr::ASTOperatorExpr(const SourceLocation &Loc) : ASTExpr(Loc, ASTExprKind::EXPR_OPERATOR) {

}

ASTUnaryOperatorExpr::ASTUnaryOperatorExpr(const SourceLocation &Loc, ASTUnaryOperatorKind Op) : ASTOperatorExpr(Loc), OperatorKind(Op) {

}

ASTUnaryOperatorKind ASTUnaryOperatorExpr::getOperatorKind() const {
    return OperatorKind;
}

std::string ASTUnaryOperatorExpr::str() const {
    return Logger("ASTUnaryOperatorExpr").
            Super(ASTOperatorExpr::str()).
            Attr("Operator", std::to_string((int) OperatorKind)).
            End();
}

ASTBinaryOperatorExpr::ASTBinaryOperatorExpr(const SourceLocation &Loc, ASTBinaryOperatorKind Op) :
    ASTOperatorExpr(Loc),
    OperatorKind(Op),
    OptionKind((int) OperatorKind < 300 ?
                ((int) OperatorKind < 200 ? ASTBinaryOptionKind::BINARY_ARITH : ASTBinaryOptionKind::BINARY_LOGIC)
                : ASTBinaryOptionKind::BINARY_COMPARISON),
    Precedence (Op == ASTBinaryOperatorKind::BINARY_ARITH_MUL ||
                Op == ASTBinaryOperatorKind::BINARY_ARITH_DIV ||
                Op == ASTBinaryOperatorKind::BINARY_ARITH_MOD) {

}

ASTBinaryOperatorKind ASTBinaryOperatorExpr::getOperatorKind() const {
    return OperatorKind;
}

bool ASTBinaryOperatorExpr::isPrecedence() const {
    return Precedence;
}

std::string ASTBinaryOperatorExpr::str() const {
    return std::to_string((int) OperatorKind);
}

ASTBinaryOptionKind ASTBinaryOperatorExpr::getOptionKind() const {
    return ASTBinaryOptionKind::BINARY_LOGIC;
}

ASTTernaryOperatorExpr::ASTTernaryOperatorExpr(const SourceLocation &Loc, ASTTernaryOperatorKind Op) : ASTOperatorExpr(Loc), OperatorKind(Op) {

}

ASTTernaryOperatorKind ASTTernaryOperatorExpr::getOperatorKind() const {
    return OperatorKind;
}

std::string ASTTernaryOperatorExpr::str() const {
    return std::to_string((int) OperatorKind);
}