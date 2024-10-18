//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVar.cpp - AST Var implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTVar.h"
#include "AST/ASTScopes.h"

using namespace fly;

ASTVar::ASTVar(ASTVarKind VarKind, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
               SmallVector<ASTScope *, 8> &Scopes) :
        ASTBase(Loc), VarKind(VarKind), Type(Type), Name(Name), Scopes(Scopes) {

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

bool ASTVar::isConstant() const {
    for (auto Scope : Scopes) {
        if (Scope->getKind() == ASTScopeKind::SCOPE_CONSTANT) {
            return Scope->isConstant();
        }
    }
    return false;
}

bool ASTVar::isInitialized() {
    return Initialization != nullptr;
}

ASTAssignmentStmt *ASTVar::getInitialization() {
    return Initialization;
}

void ASTVar::setInitialization(ASTAssignmentStmt *VarDefine) {
    Initialization = VarDefine;
}

const SmallVector<ASTScope *, 8> &ASTVar::getScopes() const {
    return Scopes;
}

std::string ASTVar::str() const {
    return Logger("ASTVar").
            Super(ASTBase::str()).
            Attr("Type", Type).
            Attr("Name", Name).
            Attr("VarKind", (uint64_t) VarKind).
            AttrList("Scopes", Scopes).
            End();
}