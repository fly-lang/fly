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
#include "Basic/Logger.h"

using namespace fly;

ASTFunction::ASTFunction(const SourceLocation &Loc, ASTTypeRef *ReturnType,
                                 llvm::SmallVector<ASTModifier *, 8> &Modifiers, llvm::StringRef Name, llvm::SmallVector<ASTVar *, 8> &Params) :
        ASTNode(Loc, ASTKind::AST_FUNCTION), ReturnTypeRef(ReturnType), Modifiers(Modifiers), Name(Name), Params(Params) {

}

llvm::StringRef ASTFunction::getName() const {
	return Name;
}

bool ASTFunction::isVarArg() {
	return false;
}

SemaFunctionBase *ASTFunction::getSema() {
	return Sema;
}

llvm::SmallVector<ASTModifier *, 8> ASTFunction::getModifiers() const {
    return Modifiers;
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
	Attr("Location", getLocation()).
 Attr("Kind", static_cast<size_t>(getKind())).
           Attr("Params", ASTNode::str(Params)).
           Attr("ReturnType", ReturnTypeRef).
           End();
}
