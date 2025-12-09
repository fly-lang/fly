//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunctionBase.cpp - AST Function Base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunction.h"
#include "AST/ASTParam.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>
#include <AST/ASTBlockStmt.h>
#include <AST/ASTModifier.h>

using namespace fly;

ASTFunction::ASTFunction(const SourceLocation &Loc, ASTType *ReturnType,
                                 llvm::SmallVector<ASTModifier *, 8> &Modifiers,
                                 llvm::StringRef Name, llvm::SmallVector<ASTParam *, 8> &Params,
                                 ASTFunctionKind FunctionKind) :
        ASTNode(Loc, ASTKind::AST_FUNCTION), ReturnType(ReturnType), Modifiers(Modifiers), Name(Name),
		Params(Params), FunctionKind(FunctionKind) {

}

ASTFunction::~ASTFunction() {
    for (auto *P : Params) delete P;
    Params.clear();
    if (Body) {
        delete Body;
        Body = nullptr;
    }
    for (auto *M : Modifiers) delete M;
    Modifiers.clear();
}

void ASTFunction::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

llvm::StringRef ASTFunction::getName() const {
	return Name;
}

bool ASTFunction::isVarArg() {
	return false;
}

llvm::SmallVector<ASTModifier *, 8> ASTFunction::getModifiers() const {
    return Modifiers;
}

llvm::SmallVector<ASTParam *, 8> ASTFunction::getParams() const {
    return Params;
}

ASTBlockStmt *ASTFunction::getBody() const {
    return Body;
}

ASTType *ASTFunction::getReturnType() const {
    return ReturnType;
}

std::string ASTFunction::str() const {
    return Logger("ASTFunctionBase").
	Attr("Location", getLocation()).
 Attr("Kind", static_cast<size_t>(getKind())).
           Attr("Params", ASTBase::str(Params)).
           Attr("ReturnType", ReturnType).
           End();
}
