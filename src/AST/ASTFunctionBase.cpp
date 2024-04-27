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
#include "AST/ASTParams.h"

using namespace fly;

ASTFunctionBase::ASTFunctionBase(const SourceLocation &Loc, ASTFunctionKind Kind, ASTType *ReturnType,
                                 llvm::StringRef Name, ASTScopes * Scopes) :
        ASTBase(Loc), Kind(Kind), ReturnType(ReturnType),
        Params(new ASTParams()), Name(Name), Scopes(Scopes) {

}

llvm::StringRef ASTFunctionBase::getName() const {
    return Name;
}

ASTScopes *ASTFunctionBase::getScopes() const {
    return Scopes;
}

void ASTFunctionBase::addParam(ASTParam *Param) {
    Params->List.push_back(Param);
}

void ASTFunctionBase::setEllipsis(ASTParam *Param) {
    Params->Ellipsis = Param;
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

const ASTParams *ASTFunctionBase::getParams() const {
    return Params;
}

ASTFunctionKind ASTFunctionBase::getKind() {
    return Kind;
}

ASTType *ASTFunctionBase::getType() const {
    return ReturnType;
}

bool ASTFunctionBase::isVarArg() {
    return Params->getEllipsis();
}

std::string ASTFunctionBase::str() const {
    return Logger("ASTFunctionBase").
           Super(ASTBase::str()).
           Attr("Name", Name).
           Attr("Params", Params).
           Attr("ReturnType", ReturnType).
           End();
}

ASTReturnStmt::ASTReturnStmt(ASTBlock *Parent, const SourceLocation &Loc) :
        ASTStmt(Parent, Loc, ASTStmtKind::STMT_RETURN) {

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
