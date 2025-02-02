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

ASTVar::ASTVar(const SourceLocation &Loc, ASTTypeRef *TypeRef, llvm::StringRef Name,
               SmallVector<ASTScope *, 8> &Scopes) :
        ASTBase(Loc, ASTKind::AST_VAR), TypeRef(TypeRef), Name(Name), Scopes(Scopes) {

}

ASTTypeRef *ASTVar::getTypeRef() const {
    return TypeRef;
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

ASTExpr * ASTVar::getExpr() const {
	return Expr;
}

std::string ASTVar::str() const {
    return Logger("ASTVar").
            Super(ASTBase::str()).
            Attr("TypeRef", TypeRef).
            Attr("Name", Name).
            Attr("Expr", Expr).
            AttrList("Scopes", Scopes).
            End();
}