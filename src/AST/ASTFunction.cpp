//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunctionBase.cpp - AST Function Base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunction.h"
#include "AST/ASTVar.h"

using namespace fly;

ASTFunction::ASTFunction(const SourceLocation &Loc, ASTTypeRef *ReturnType,
                                 llvm::SmallVector<ASTScope *, 8> &Scopes, llvm::StringRef Name, llvm::SmallVector<ASTVar *, 8> &Params) :
        ASTBase(Loc, ASTKind::AST_FUNCTION), ReturnTypeRef(ReturnType), Scopes(Scopes), Params(Params) {

}

llvm::StringRef ASTFunction::getName() const {
	return Name;
}

bool ASTFunction::isVarArg() {
	return false;
}

SymFunctionBase *ASTFunction::getSym() {
	return Sym;
}

llvm::SmallVector<ASTScope *, 8> ASTFunction::getScopes() const {
    return Scopes;
}

llvm::SmallVector<ASTVar *, 8> ASTFunction::getParams() const {
    return Params;
}

ASTBlockStmt *ASTFunction::getBody() const {
    return Body;
}

ASTTypeRef *ASTFunction::getReturnTypeRef() const {
    return ReturnTypeRef;
}

std::string ASTFunction::str() const {
    return Logger("ASTFunctionBase").
           Super(ASTBase::str()).
           AttrList("Params", Params).
           Attr("ReturnType", ReturnTypeRef).
           End();
}
