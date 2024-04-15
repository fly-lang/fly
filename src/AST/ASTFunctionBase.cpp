//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunctionBase.cpp - Function Base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunctionBase.h"
#include "AST/ASTBlock.h"
#include "AST/ASTParams.h"
#include "AST/ASTType.h"

#include <string>

using namespace fly;

ASTFunctionBase::ASTFunctionBase(const SourceLocation &Loc, ASTFunctionKind Kind, ASTType *ReturnType,
                                 llvm::StringRef Name, ASTScopes * Scopes)
        : Kind(Kind), ReturnType(ReturnType), Params(new ASTParams()), Name(Name), Location(Loc), Scopes(Scopes) {

}

llvm::StringRef ASTFunctionBase::getName() const {
    return Name;
}

ASTScopes *ASTFunctionBase::getScopes() const {
    return Scopes;
}

llvm::StringRef ASTFunctionBase::getComment() const {
    return Comment;
}

const SourceLocation &ASTFunctionBase::getLocation() const {
    return Location;
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
           Attr("Name", Name).
           Attr("Params", Params).
           Attr("ReturnType", ReturnType).
           End();
}

ASTReturn::ASTReturn(ASTBlock *Parent, const SourceLocation &Loc) :
        ASTStmt(Parent, Loc, ASTStmtKind::STMT_RETURN) {

}

ASTExpr *ASTReturn::getExpr() const {
    return Expr;
}

ASTBlock *ASTReturn::getBlock() const {
    return Block;
}

std::string ASTReturn::str() const {
    return Logger("ASTReturn").
            Attr("Kind", (uint64_t) Kind).
            End();
}
