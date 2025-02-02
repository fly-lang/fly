//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunctionBase.cpp - AST Function Base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunction.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTVar.h"

using namespace fly;

ASTFunction::ASTFunction(const SourceLocation &Loc, llvm::StringRef Name, ASTTypeRef *ReturnType,
                                 llvm::SmallVector<ASTScope *, 8> &Scopes, llvm::SmallVector<ASTVar *, 8> &Params) :
        ASTBase(Loc, ASTKind::AST_FUNCTION), ReturnType(ReturnType), Scopes(Scopes), Params(Params) {

}

llvm::StringRef ASTFunction::getName() const {
}

bool ASTFunction::isVarArg() {
}

llvm::SmallVector<ASTScope *, 8> ASTFunction::getScopes() const {
    return Scopes;
}

llvm::SmallVector<ASTVar *, 8> ASTFunction::getParams() const {
    return Params;
}

llvm::SmallVector<ASTLocalVar *, 8> ASTFunction::getLocalVars() const {
    return LocalVars;
}

ASTBlockStmt *ASTFunction::getBody() const {
    return Body;
}

ASTVar *ASTFunction::getErrorHandler() {
    return ErrorHandler;
}

ASTTypeRef *ASTFunction::getReturnType() const {
    return ReturnType;
}

std::string ASTFunction::str() const {
    return Logger("ASTFunctionBase").
           Super(ASTBase::str()).
           AttrList("Params", Params).
           Attr("ReturnType", ReturnType).
           End();
}
