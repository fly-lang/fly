//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunctionBase.cpp - AST Function Base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunctionBase.h"
#include "AST/ASTBlock.h"
#include "AST/ASTParam.h"

using namespace fly;

ASTFunctionBase::ASTFunctionBase(const SourceLocation &Loc, ASTFunctionKind Kind, ASTType *ReturnType,
                                 ASTScopes * Scopes) :
        ASTBase(Loc), Kind(Kind), ReturnType(ReturnType), Scopes(Scopes) {

}

ASTScopes *ASTFunctionBase::getScopes() const {
    return Scopes;
}

llvm::SmallVector<ASTParam *, 8> ASTFunctionBase::getParams() const {
    return Params;
}

void ASTFunctionBase::setEllipsis(ASTParam *Param) {
    Ellipsis = Param;
}

const ASTBlock *ASTFunctionBase::getBody() const {
    return Body;
}

void ASTFunctionBase::setErrorHandler(ASTParam *EH) {
    ErrorHandler = EH;
}

ASTParam *ASTFunctionBase::getErrorHandler() {
    return ErrorHandler;
}

ASTFunctionKind ASTFunctionBase::getKind() {
    return Kind;
}

ASTType *ASTFunctionBase::getType() const {
    return ReturnType;
}

bool ASTFunctionBase::isVarArg() {
    return Ellipsis != nullptr;
}

std::string ASTFunctionBase::str() const {
    return Logger("ASTFunctionBase").
           Super(ASTBase::str()).
           AttrList("Params", Params).
           Attr("ReturnType", ReturnType).
           End();
}

ASTParam *ASTFunctionBase::getEllipsis() const {
    return Ellipsis;
}

ASTReturnStmt::ASTReturnStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_RETURN) {

}

ASTExpr *ASTReturnStmt::getExpr() const {
    return Expr;
}

ASTBlock *ASTReturnStmt::getBlock() const {
    return Block;
}

std::string ASTReturnStmt::str() const {
    return Logger("ASTReturn").
            Attr("Kind", (uint64_t) Kind).
            End();
}
