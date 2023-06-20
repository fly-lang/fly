//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunc.cpp - AST Function Call implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTCall.h"
#include "AST/ASTFunction.h"
#include "AST/ASTParams.h"
#include "AST/ASTIdentifier.h"

using namespace fly;

ASTArg::ASTArg(ASTCall *Call, ASTExpr *Expr) : Expr(Expr), Call(Call) {

}

uint64_t ASTArg::getIndex() const {
    return Index;
}

ASTParam *ASTArg::getDef() const {
    return Def;
}

std::string ASTArg::str() const {
    return Logger("ASTArg").
            Attr("Expr", Expr).
            Attr("Index", Index).
            End();
}

ASTCall *ASTArg::getCall() const {
    return Call;
}

ASTExpr *ASTArg::getExpr() const {
    return Expr;
}

ASTCall::ASTCall(ASTIdentifier *Identifier) : ASTReference(Identifier, true) {

}

ASTCall::ASTCall(ASTFunctionBase *Function) : Def(Function), ASTReference(nullptr, true) {
}

const std::vector<ASTArg*> ASTCall::getArgs() const {
    return Args;
}

ASTFunctionBase *ASTCall::getDef() const {
    return Def;
}

bool ASTCall::isNew() const {
    return New;
}

std::string ASTCall::str() const {
    return Logger("ASTFunctionCall").
            Attr("Identifier", getIdentifier()).
            Attr("Instance", getInstance()).
            AttrList("Args", Args).
            Attr("Def", Def).
            End();
}
