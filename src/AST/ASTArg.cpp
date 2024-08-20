//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTArg.cpp - AST Call argument
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTArg.h"
#include "AST/ASTExpr.h"


using namespace fly;

ASTArg::ASTArg(ASTExpr *Expr, uint64_t Index) :
        ASTBase(Expr->getLocation()), Expr(Expr), Index(Index) {

}

uint64_t ASTArg::getIndex() const {
    return Index;
}

ASTParam *ASTArg::getDef() const {
    return Def;
}

ASTCall *ASTArg::getCall() const {
    return Call;
}

ASTExpr *ASTArg::getExpr() const {
    return Expr;
}

std::string ASTArg::str() const {
    return Logger("ASTArg").
            Super(ASTBase::str()).
            Attr("Expr", Expr).
            Attr("Index", Index).
            End();
}