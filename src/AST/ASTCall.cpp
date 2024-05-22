//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTCall.cpp - AST Function Call implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTCall.h"
#include "AST/ASTFunctionBase.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTExpr.h"

using namespace fly;

ASTArg::ASTArg(ASTExpr *Expr) :
        ASTBase(Expr->getLocation()), Expr(Expr), Index(0) {

}

uint64_t ASTArg::getIndex() const {
    return Index;
}

ASTParam *ASTArg::getDef() const {
    return Def;
}

std::string ASTArg::str() const {
    return Logger("ASTArg").
            Super(ASTBase::str()).
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

ASTCall::ASTCall(const SourceLocation &Loc, llvm::StringRef Name) : ASTIdentifier(Loc, Name, ASTIdentifierKind::REF_CALL) {

}

const ASTVar *ASTCall::getErrorHandler() const {
    return ErrorHandler;
}

llvm::SmallVector<ASTArg *, 8> ASTCall::getArgs() const {
    return Args;
}

ASTFunctionBase *ASTCall::getDef() const {
    return Def;
}

ASTCallKind ASTCall::getCallKind() const {
    return CallKind;
}

ASTMemoryKind ASTCall::getMemoryKind() const {
    return MemoryKind;
}

std::string ASTCall::str() const {
    return Logger("ASTFunctionCall").
            AttrList("Args", Args).
            Attr("Def", Def).
            End();
}
