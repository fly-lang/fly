//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunctionBase.cpp - AST Function Base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunctionBase.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTParam.h"

using namespace fly;

ASTFunctionBase::ASTFunctionBase(const SourceLocation &Loc, ASTFunctionKind Kind, ASTType *ReturnType,
                                 llvm::SmallVector<ASTScope *, 8> &Scopes, llvm::SmallVector<ASTParam *, 8> &Params) :
        ASTBase(Loc), Kind(Kind), ReturnType(ReturnType), Scopes(Scopes), Params(Params) {

}

llvm::SmallVector<ASTScope *, 8> ASTFunctionBase::getScopes() const {
    return Scopes;
}

llvm::SmallVector<ASTParam *, 8> ASTFunctionBase::getParams() const {
    return Params;
}

void ASTFunctionBase::setEllipsis(ASTParam *Param) {
    Ellipsis = Param;
}

llvm::SmallVector<ASTLocalVar *, 8> ASTFunctionBase::getLocalVars() const {
    return LocalVars;
}

ASTBlockStmt *ASTFunctionBase::getBody() const {
    return Body;
}

ASTParam *ASTFunctionBase::getErrorHandler() {
    return ErrorHandler;
}

ASTFunctionKind ASTFunctionBase::getKind() {
    return Kind;
}

ASTType *ASTFunctionBase::getReturnType() const {
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
