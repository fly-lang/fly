//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTBinary.cpp - AST Binary Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBinary.h"
#include "AST/ASTVisitor.h"
#include "Basic/Logger.h"

using namespace fly;

ASTBinary::ASTBinary(ASTBinaryKind OpKind, const SourceLocation &OpLocation,
                                 ASTExpr *LeftExpr, ASTExpr *RightExpr) :
        ASTExpr(LeftExpr->getLocation(), ASTExprKind::EXPR_BINARY),
        OpKind(OpKind), OpLocation(OpLocation),
        LeftExpr(LeftExpr), RightExpr(RightExpr) {

}

void ASTBinary::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

bool ASTBinary::isArith() const {
    int OpValue = static_cast<int>(OpKind);
    return OpValue < 10; // OP_BINARY_ARITH_ADD to OP_BINARY_ARITH_SHIFT_R (0-9)
}

bool ASTBinary::isLogic() const {
    int OpValue = static_cast<int>(OpKind);
    return OpValue >= 10 && OpValue < 12; // OP_BINARY_LOGIC_AND to OP_BINARY_LOGIC_OR (10-11)
}

bool ASTBinary::isCompare() const {
    int OpValue = static_cast<int>(OpKind);
    return OpValue >= 12 && OpValue < 18; // OP_BINARY_COMPARE_EQ to OP_BINARY_COMPARE_LTE (12-17)
}

bool ASTBinary::isAssign() const {
    int OpValue = static_cast<int>(OpKind);
    return OpValue >= 18; // OP_BINARY_ASSIGN to OP_BINARY_ASSIGN_OR (18+)
}

ASTBinaryKind ASTBinary::getOpKind() const {
    return OpKind;
}


SourceLocation &ASTBinary::getOpLocation() {
    return OpLocation;
}

ASTExpr *ASTBinary::getLeftExpr() const {
    return LeftExpr;
}

ASTExpr *ASTBinary::getRightExpr() const {
    return RightExpr;
}

SemaBinary * ASTBinary::getSema() const {
	return static_cast<SemaBinary *>(Sema);
}

void ASTBinary::setSema(SemaBinary *Sema) {
	this->Sema = Sema;
}

std::string ASTBinary::str() const {
    return Logger("ASTBinaryOpExpr").
	Attr("Location", getLocation()).
 Attr("Kind", static_cast<size_t>(getKind())).
           Attr("Op", (uint64_t) OpKind).
           Attr("OpLocation", (uint64_t) OpLocation.getRawEncoding()).
           Attr("LeftExpr", LeftExpr).
           Attr("RightExpr", RightExpr).
           End();
}

