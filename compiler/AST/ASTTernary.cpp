//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTTernary.cpp - AST ternary expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTTernary.h"
#include "AST/ASTVisitor.h"
#include "Basic/Logger.h"

using namespace fly;

ASTTernary::ASTTernary(ASTExpr *ConditionExpr, const SourceLocation &TrueOpLocation,
                                   ASTExpr *TrueExpr, const SourceLocation &FalseOpLocation, ASTExpr *FalseExpr) :
        ASTExpr(ConditionExpr->getLocation(), ASTExprKind::EXPR_TERNARY),
        ConditionExpr(ConditionExpr), TrueOpLocation(TrueOpLocation),
        TrueExpr(TrueExpr), FalseOpLocation(FalseOpLocation), FalseExpr(FalseExpr) {

}

void ASTTernary::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTExpr *ASTTernary::getConditionExpr() const {
    return ConditionExpr;
}

SourceLocation &ASTTernary::getTrueOpLocation() {
    return TrueOpLocation;
}

ASTExpr *ASTTernary::getTrueExpr() const {
    return TrueExpr;
}

SourceLocation &ASTTernary::getFalseOpLocation() {
    return FalseOpLocation;
}

ASTExpr *ASTTernary::getFalseExpr() const { return FalseExpr; }

std::string ASTTernary::str() const {
    return Logger("ASTTernary")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("ConditionExpr", ConditionExpr)
        .Attr("TrueOpLocation", TrueOpLocation)
        .Attr("TrueExpr", TrueExpr)
        .Attr("FalseOpLocation", FalseOpLocation)
        .Attr("FalseExpr", FalseExpr)
        .End();
}

