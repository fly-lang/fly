//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTOp.cpp - AST Operator Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTOp.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTUnaryOp::ASTUnaryOp(const SourceLocation &Loc, ASTUnaryOpKind OpKind, ASTExpr *Expr) :
	ASTExpr(Loc, ASTExprKind::EXPR_UNARY), OpKind(OpKind), Expr(Expr) {
}

void ASTUnaryOp::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTUnaryOpKind ASTUnaryOp::getOpKind() const {
    return OpKind;
}

SourceLocation &ASTUnaryOp::getOpLocation() {
    return OpLocation;
}

ASTExpr *ASTUnaryOp::getExpr() const {
    return Expr;
}

std::string ASTUnaryOp::str() const {
    return Logger("ASTOp").
	Attr("Location", getLocation()).
Attr("Kind", static_cast<size_t>(getKind())).
           Attr("Expr", (ASTNode *) Expr).
           Attr("Op", (uint64_t) OpKind).
           Attr("OpLocation", (uint64_t) OpLocation.getRawEncoding()).
           End();
}

ASTBinaryOpTypeExprKind ASTBinaryOp::setTypeKind(ASTBinaryOpKind OpKind) {
    switch (OpKind) {

        case ASTBinaryOpKind::OP_BINARY_ADD:
        case ASTBinaryOpKind::OP_BINARY_SUB:
        case ASTBinaryOpKind::OP_BINARY_MUL:
        case ASTBinaryOpKind::OP_BINARY_DIV:
        case ASTBinaryOpKind::OP_BINARY_MOD:
        case ASTBinaryOpKind::OP_BINARY_AND:
        case ASTBinaryOpKind::OP_BINARY_OR:
        case ASTBinaryOpKind::OP_BINARY_XOR:
        case ASTBinaryOpKind::OP_BINARY_SHIFT_L:
        case ASTBinaryOpKind::OP_BINARY_SHIFT_R:
            return ASTBinaryOpTypeExprKind::OP_BINARY_ARITH;

        case ASTBinaryOpKind::OP_BINARY_LOGIC_AND:
        case ASTBinaryOpKind::OP_BINARY_LOGIC_OR:
            return ASTBinaryOpTypeExprKind::OP_BINARY_LOGIC;

        case ASTBinaryOpKind::OP_BINARY_EQ:
        case ASTBinaryOpKind::OP_BINARY_NE:
        case ASTBinaryOpKind::OP_BINARY_GT:
        case ASTBinaryOpKind::OP_BINARY_GTE:
        case ASTBinaryOpKind::OP_BINARY_LT:
        case ASTBinaryOpKind::OP_BINARY_LTE:
            return ASTBinaryOpTypeExprKind::OP_BINARY_COMPARISON;

        case ASTBinaryOpKind::OP_BINARY_ASSIGN:
        case ASTBinaryOpKind::OP_BINARY_ASSIGN_AND:
        case ASTBinaryOpKind::OP_BINARY_ASSIGN_MUL:
        case ASTBinaryOpKind::OP_BINARY_ASSIGN_ADD:
        case ASTBinaryOpKind::OP_BINARY_ASSIGN_SUB:
        case ASTBinaryOpKind::OP_BINARY_ASSIGN_DIV:
        case ASTBinaryOpKind::OP_BINARY_ASSIGN_MOD:
        case ASTBinaryOpKind::OP_BINARY_ASSIGN_SHIFT_L:
        case ASTBinaryOpKind::OP_BINARY_ASSIGN_SHIFT_R:
        case ASTBinaryOpKind::OP_BINARY_ASSIGN_XOR:
        case ASTBinaryOpKind::OP_BINARY_ASSIGN_OR:
            return ASTBinaryOpTypeExprKind::OP_BINARY_ASSIGN;
    }
    return ASTBinaryOpTypeExprKind::OP_BINARY_ASSIGN;
}

ASTBinaryOp::ASTBinaryOp(ASTBinaryOpKind OpKind, const SourceLocation &OpLocation,
                                 ASTExpr *LeftExpr, ASTExpr *RightExpr) :
        ASTExpr(LeftExpr->getLocation(), ASTExprKind::EXPR_BINARY),
        TypeKind(setTypeKind(OpKind)), OpKind(OpKind), OpLocation(OpLocation),
        LeftExpr(LeftExpr), RightExpr(RightExpr) {

}

void ASTBinaryOp::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTBinaryOpTypeExprKind ASTBinaryOp::getTypeKind() const {
    return TypeKind;
}

ASTBinaryOpKind ASTBinaryOp::getOpKind() const {
    return OpKind;
}


SourceLocation &ASTBinaryOp::getOpLocation() {
    return OpLocation;
}

ASTExpr *ASTBinaryOp::getLeftExpr() const {
    return LeftExpr;
}

ASTExpr *ASTBinaryOp::getRightExpr() const {
    return RightExpr;
}

std::string ASTBinaryOp::str() const {
    return Logger("ASTBinaryOpExpr").
	Attr("Location", getLocation()).
 Attr("Kind", static_cast<size_t>(getKind())).
           Attr("Op", (uint64_t) OpKind).
           Attr("OpLocation", (uint64_t) OpLocation.getRawEncoding()).
           Attr("LeftExpr", LeftExpr).
           Attr("RightExpr", RightExpr).
           End();
}

ASTTernaryOp::ASTTernaryOp(ASTExpr *ConditionExpr, const SourceLocation &TrueOpLocation,
                                   ASTExpr *TrueExpr, const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr) :
        ASTExpr(ConditionExpr->getLocation(), ASTExprKind::EXPR_TERNARY),
        ConditionExpr(ConditionExpr), TrueOpLocation(TrueOpLocation),
        TrueExpr(TrueExpr), FalseOpLocation(FalseOpLocation), FalseExpr(FalseExpr) {

}

void ASTTernaryOp::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTExpr *ASTTernaryOp::getConditionExpr() const {
    return ConditionExpr;
}

SourceLocation &ASTTernaryOp::getTrueOpLocation() {
    return TrueOpLocation;
}

ASTExpr *ASTTernaryOp::getTrueExpr() const {
    return TrueExpr;
}

SourceLocation &ASTTernaryOp::getFalseOpLocation() {
    return FalseOpLocation;
}

ASTExpr *ASTTernaryOp::getFalseExpr() const {
    return FalseExpr;
}

std::string ASTTernaryOp::str() const {
    return Logger("ASTTernaryGroupExpr").
	Attr("Location", getLocation()).
 Attr("Kind", static_cast<size_t>(getKind())).
           Attr("First", ConditionExpr).
           Attr("Second", TrueExpr).
           Attr("Third", FalseExpr).
           End();
}
