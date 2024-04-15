//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVar.cpp - Var declaration implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTVar.h"
#include "AST/ASTVarDefine.h"
#include "AST/ASTScopes.h"

using namespace fly;

ASTVar::ASTVar(ASTVarKind VarKind, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes) :
    VarKind(VarKind), Loc(Loc), Type(Type), Name(Name), Scopes(Scopes) {

}

ASTVarKind ASTVar::getVarKind() {
    return VarKind;
}

ASTType *ASTVar::getType() const {
    return Type;
}

llvm::StringRef ASTVar::getName() const {
    return Name;
}

const SourceLocation &ASTVar::getLocation() const {
    return Loc;
}

llvm::StringRef ASTVar::getComment() const {
    return Comment;
}

bool ASTVar::isConstant() const {
    return Scopes->isConstant();
}

bool ASTVar::isInitialized() {
    return Initialization != nullptr;
}

ASTVarDefine *ASTVar::getInitialization() {
    return Initialization;
}

void ASTVar::setInitialization(ASTVarDefine *VarDefine) {
    Initialization = VarDefine;
}

ASTScopes *ASTVar::getScopes() const {
    return Scopes;
}

std::string ASTVar::str() const {
    return Logger("ASTVar").
            Attr("Type", Type).
            Attr("Name", Name).
            Attr("VarKind", (uint64_t) VarKind).
            Attr("Scopes", Scopes).
            End();
}