//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTFunction.cpp - AST function definition implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunction.h"
#include "AST/ASTParam.h"
#include "AST/ASTType.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>
#include <AST/ASTBlockStmt.h>
#include <AST/ASTModifier.h>

using namespace fly;

ASTFunction::ASTFunction(const SourceLocation &Loc,
                                 llvm::SmallVector<ASTModifier *, 8> &Modifiers,
                                 llvm::StringRef Name, llvm::SmallVector<ASTParam *, 8> &Params,
                                 ASTFunctionKind FunctionKind) :
        ASTNode(Loc, ASTKind::AST_FUNCTION), Modifiers(Modifiers), Name(Name),
		Params(Params), FunctionKind(FunctionKind) {

}

ASTFunction::~ASTFunction() {
    for (auto *P : Params) delete P;
    Params.clear();
    delete ReturnType;
    ReturnType = nullptr;
    if (Body) {
        delete Body;
        Body = nullptr;
    }
    for (auto *M : Modifiers) delete M;
    Modifiers.clear();
}

ASTType *ASTFunction::getReturnType() const {
    return ReturnType;
}

void ASTFunction::setReturnType(ASTType *RT) {
    ReturnType = RT;
}

const llvm::SmallVector<ASTType *, 4> &ASTFunction::getReturnTypes() const {
    return ReturnTypes;
}

void ASTFunction::setReturnTypes(const llvm::SmallVector<ASTType *, 4> &RTs) {
    ReturnTypes = RTs;
}

void ASTFunction::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

llvm::StringRef ASTFunction::getName() const {
	return Name;
}

llvm::SmallVector<ASTModifier *, 8> ASTFunction::getModifiers() const {
    return Modifiers;
}

llvm::SmallVector<ASTParam *, 8> ASTFunction::getParams() const {
    return Params;
}

const llvm::SmallVector<ASTTypeParam *, 4> &ASTFunction::getTypeParams() const {
    return TypeParams;
}

ASTBlockStmt *ASTFunction::getBody() const {
    return Body;
}

std::string ASTFunction::str() const {
    return Logger("ASTFunction")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("FunctionKind", static_cast<size_t>(FunctionKind))
        .Attr("Name", Name)
        .Attr("TypeParams", TypeParams)
        .Attr("Modifiers", Modifiers)
        .Attr("Params", Params)
        .Attr("ReturnType", ReturnType)
        .Attr("Body", Body)
        .End();
}
