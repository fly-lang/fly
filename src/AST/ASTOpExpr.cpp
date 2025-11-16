//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTOpExpr.cpp - AST Operator Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTOpExpr.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTUnaryOpExpr::ASTUnaryOpExpr(const SourceLocation &Loc, ASTUnaryOpExprKind OpKind, ASTExpr *Expr) : ASTExpr(Loc, ASTExprKind::EXPR_UNARY) {
}

void ASTUnaryOpExpr::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTUnaryOpExprKind ASTUnaryOpExpr::getOpKind() const {
    return OpKind;
}

SourceLocation &ASTUnaryOpExpr::getOpLocation() {
    return OpLocation;
}

ASTExpr *ASTUnaryOpExpr::getExpr() const {
    return Expr;
}

std::string ASTUnaryOpExpr::str() const {
    return Logger("ASTOpExpr").
	Attr("Location", getLocation()).
Attr("Kind", static_cast<size_t>(getKind())).
           Attr("Expr", (ASTNode *) Expr).
           Attr("Op", (uint64_t) OpKind).
           Attr("OpLocation", (uint64_t) OpLocation.getRawEncoding()).
           End();
}

ASTBinaryOpTypeExprKind ASTBinaryOpExpr::setTypeKind(ASTBinaryOpExprKind OpKind) {
    switch (OpKind) {

        case ASTBinaryOpExprKind::OP_BINARY_ADD:
        case ASTBinaryOpExprKind::OP_BINARY_SUB:
        case ASTBinaryOpExprKind::OP_BINARY_MUL:
        case ASTBinaryOpExprKind::OP_BINARY_DIV:
        case ASTBinaryOpExprKind::OP_BINARY_MOD:
        case ASTBinaryOpExprKind::OP_BINARY_AND:
        case ASTBinaryOpExprKind::OP_BINARY_OR:
        case ASTBinaryOpExprKind::OP_BINARY_XOR:
        case ASTBinaryOpExprKind::OP_BINARY_SHIFT_L:
        case ASTBinaryOpExprKind::OP_BINARY_SHIFT_R:
            return ASTBinaryOpTypeExprKind::OP_BINARY_ARITH;

        case ASTBinaryOpExprKind::OP_BINARY_LOGIC_AND:
        case ASTBinaryOpExprKind::OP_BINARY_LOGIC_OR:
            return ASTBinaryOpTypeExprKind::OP_BINARY_LOGIC;

        case ASTBinaryOpExprKind::OP_BINARY_EQ:
        case ASTBinaryOpExprKind::OP_BINARY_NE:
        case ASTBinaryOpExprKind::OP_BINARY_GT:
        case ASTBinaryOpExprKind::OP_BINARY_GTE:
        case ASTBinaryOpExprKind::OP_BINARY_LT:
        case ASTBinaryOpExprKind::OP_BINARY_LTE:
            return ASTBinaryOpTypeExprKind::OP_BINARY_COMPARISON;

        case ASTBinaryOpExprKind::OP_BINARY_ASSIGN:
        case ASTBinaryOpExprKind::OP_BINARY_ASSIGN_AND:
        case ASTBinaryOpExprKind::OP_BINARY_ASSIGN_MUL:
        case ASTBinaryOpExprKind::OP_BINARY_ASSIGN_ADD:
        case ASTBinaryOpExprKind::OP_BINARY_ASSIGN_SUB:
        case ASTBinaryOpExprKind::OP_BINARY_ASSIGN_DIV:
        case ASTBinaryOpExprKind::OP_BINARY_ASSIGN_MOD:
        case ASTBinaryOpExprKind::OP_BINARY_ASSIGN_SHIFT_L:
        case ASTBinaryOpExprKind::OP_BINARY_ASSIGN_SHIFT_R:
        case ASTBinaryOpExprKind::OP_BINARY_ASSIGN_XOR:
        case ASTBinaryOpExprKind::OP_BINARY_ASSIGN_OR:
            return ASTBinaryOpTypeExprKind::OP_BINARY_ASSIGN;
    }
    return ASTBinaryOpTypeExprKind::OP_BINARY_ASSIGN;
}

ASTBinaryOpExpr::ASTBinaryOpExpr(ASTBinaryOpExprKind OpKind, const SourceLocation &OpLocation,
                                 ASTExpr *LeftExpr, ASTExpr *RightExpr) :
        ASTExpr(LeftExpr->getLocation(), ASTExprKind::EXPR_BINARY),
        TypeKind(setTypeKind(OpKind)), OpKind(OpKind), OpLocation(OpLocation),
        LeftExpr(LeftExpr), RightExpr(RightExpr) {

}

void ASTBinaryOpExpr::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTBinaryOpTypeExprKind ASTBinaryOpExpr::getTypeKind() const {
    return TypeKind;
}

ASTBinaryOpExprKind ASTBinaryOpExpr::getOpKind() const {
    return OpKind;
}


SourceLocation &ASTBinaryOpExpr::getOpLocation() {
    return OpLocation;
}

ASTExpr *ASTBinaryOpExpr::getLeftExpr() const {
    return LeftExpr;
}

ASTExpr *ASTBinaryOpExpr::getRightExpr() const {
    return RightExpr;
}

std::string ASTBinaryOpExpr::str() const {
    return Logger("ASTBinaryOpExpr").
	Attr("Location", getLocation()).
 Attr("Kind", static_cast<size_t>(getKind())).
           Attr("Op", (uint64_t) OpKind).
           Attr("OpLocation", (uint64_t) OpLocation.getRawEncoding()).
           Attr("LeftExpr", LeftExpr).
           Attr("RightExpr", RightExpr).
           End();
}

ASTTernaryOpExpr::ASTTernaryOpExpr(ASTExpr *ConditionExpr, const SourceLocation &TrueOpLocation,
                                   ASTExpr *TrueExpr, const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr) :
        ASTExpr(ConditionExpr->getLocation(), ASTExprKind::EXPR_TERNARY),
        ConditionExpr(ConditionExpr), TrueOpLocation(TrueOpLocation),
        TrueExpr(TrueExpr), FalseOpLocation(FalseOpLocation), FalseExpr(FalseExpr) {

}

void ASTTernaryOpExpr::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTExpr *ASTTernaryOpExpr::getConditionExpr() const {
    return ConditionExpr;
}

SourceLocation &ASTTernaryOpExpr::getTrueOpLocation() {
    return TrueOpLocation;
}

ASTExpr *ASTTernaryOpExpr::getTrueExpr() const {
    return TrueExpr;
}

SourceLocation &ASTTernaryOpExpr::getFalseOpLocation() {
    return FalseOpLocation;
}

ASTExpr *ASTTernaryOpExpr::getFalseExpr() const {
    return FalseExpr;
}

std::string ASTTernaryOpExpr::str() const {
    return Logger("ASTTernaryGroupExpr").
	Attr("Location", getLocation()).
 Attr("Kind", static_cast<size_t>(getKind())).
           Attr("First", ConditionExpr).
           Attr("Second", TrueExpr).
           Attr("Third", FalseExpr).
           End();
}
