//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClassVar.cpp - Class Attribute implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClassAttribute.h"
#include "AST/ASTClass.h"
#include "AST/ASTScopes.h"
#include "CodeGen/CodeGenClassVar.h"

using namespace fly;

ASTClassAttribute::ASTClassAttribute(const SourceLocation &Loc, ASTClass &Class, ASTType *Type,
                                     llvm::StringRef Name, SmallVector<ASTScope *, 8> &Scopes) :
        ASTVar(ASTVarKind::VAR_CLASS, Loc, Type, Name, Scopes), Class(Class), Visibility(ASTVisibilityKind::V_DEFAULT) {

}

ASTClass &ASTClassAttribute::getClass() const {
    return Class;
}

ASTExpr *ASTClassAttribute::getExpr() const {
    return Expr;
}

void ASTClassAttribute::setExpr(ASTExpr *E) {
    Expr = E;
}

CodeGenVarBase *ASTClassAttribute::getCodeGen() const {
    return CodeGen;
}

void ASTClassAttribute::setCodeGen(CodeGenVarBase *CG) {
    this->CodeGen = CG;
}

std::string ASTClassAttribute::str() const {
    return Logger("ASTClassVar").
            Super(ASTVar::str()).
            End();
}
