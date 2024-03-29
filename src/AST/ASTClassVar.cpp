//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClassField.cpp - Class Field implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClassVar.h"
#include "AST/ASTClass.h"
#include "AST/ASTType.h"
#include "AST/ASTExpr.h"
#include "CodeGen/CodeGenClass.h"

using namespace fly;

ASTClassVar::ASTClassVar(const SourceLocation &Loc, ASTClass *Class, ASTClassScopes *Scopes, ASTType *Type,
                         llvm::StringRef Name) :
        VarKind(ASTVarKind::VAR_CLASS), Type(Type), Name(Name),
        Loc(Loc), Class(Class), Scopes(Scopes) {

}

const SourceLocation &ASTClassVar::getLocation() const {
    return Loc;
}

llvm::StringRef ASTClassVar::getName() const {
    return Name;
}

ASTVarKind ASTClassVar::getVarKind() {
    return VarKind;
}

ASTType *ASTClassVar::getType() const {
    return Type;
}

const ASTClass *ASTClassVar::getClass() const {
    return Class;
}

llvm::StringRef ASTClassVar::getComment() const {
    return Comment;
}

ASTClassScopes *ASTClassVar::getScopes() const {
    return Scopes;
}

ASTExpr *ASTClassVar::getExpr() const {
    return Expr;
}

void ASTClassVar::setExpr(ASTExpr *Expr) {
    this->Expr = Expr;
}

CodeGenClassVar *ASTClassVar::getCodeGen() const {
    return CodeGen;
}

void ASTClassVar::setCodeGen(CodeGenClassVar *CGCV) {
    CodeGen = CGCV;
}

std::string ASTClassVar::str() const {
    return Logger("ASTClassVar").
            Attr("Type", Type).
            Attr("Name", Name).
            Attr("VarKind", (uint64_t) VarKind).
            Attr("Comment", Comment).
            Attr("Scopes", Scopes).
            Attr("Expr", Expr).
            End();
}
