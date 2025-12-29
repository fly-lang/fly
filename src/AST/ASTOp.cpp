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

const SourceLocation &ASTUnaryOp::getOpLocation() const {
    return ASTBase::getLocation();
}

ASTExpr *ASTUnaryOp::getExpr() const {
    return Expr;
}

SemaUnary *ASTUnaryOp::getSema() const {
	return static_cast<SemaUnary *>(Sema);
}

void ASTUnaryOp::setSema(SemaUnary *Sema) {
	this->Sema = Sema;
}

std::string ASTUnaryOp::str() const {
    return Logger("ASTOp").
	Attr("Location", getLocation()).
Attr("Kind", static_cast<size_t>(getKind())).
           Attr("Expr", (ASTNode *) Expr).
           Attr("Op", (uint64_t) OpKind).
           End();
}

ASTBinaryKind ASTBinaryOp::setBinaryKind(ASTBinaryOpKind OpKind) {
    // Calculate binary kind based on enum value ranges:
    // Arithmetic: 0-9 (OP_BINARY_ARITH_ADD to OP_BINARY_ARITH_SHIFT_R)
    // Logic: 10-11 (OP_BINARY_LOGIC_AND to OP_BINARY_LOGIC_OR)
    // Comparison: 12-17 (OP_BINARY_COMPARE_EQ to OP_BINARY_COMPARE_LTE)
    // Assignment: 18+ (OP_BINARY_ASSIGN to OP_BINARY_ASSIGN_OR)

    int OpValue = static_cast<int>(OpKind);

    if (OpValue < 10) {
        return ASTBinaryKind::OP_BINARY_ARITH;
    } else if (OpValue < 12) {
        return ASTBinaryKind::OP_BINARY_LOGIC;
    } else if (OpValue < 18) {
        return ASTBinaryKind::OP_BINARY_COMPARE;
    } else {
        return ASTBinaryKind::OP_BINARY_ASSIGN;
    }
}

ASTBinaryOp::ASTBinaryOp(ASTBinaryOpKind OpKind, const SourceLocation &OpLocation,
                                 ASTExpr *LeftExpr, ASTExpr *RightExpr) :
        ASTExpr(LeftExpr->getLocation(), ASTExprKind::EXPR_BINARY),
        BinaryKind(setBinaryKind(OpKind)), OpKind(OpKind), OpLocation(OpLocation),
        LeftExpr(LeftExpr), RightExpr(RightExpr) {

}

void ASTBinaryOp::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTBinaryKind ASTBinaryOp::getBinaryKind() const {
    return BinaryKind;
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

SemaBinary * ASTBinaryOp::getSema() const {
	return static_cast<SemaBinary *>(Sema);
}

void ASTBinaryOp::setSema(SemaBinary *Sema) {
	this->Sema = Sema;
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

SemaTernary * ASTTernaryOp::getSema() const {
	return static_cast<SemaTernary *>(Sema);
}

void ASTTernaryOp::setSema(SemaTernary *Sema) {
	this->Sema = Sema;
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
