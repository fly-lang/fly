//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClassField.cpp - Class Field implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClassVar.h"
#include "CodeGen/CodeGenClass.h"

using namespace fly;

ASTClassVar::ASTClassVar(const SourceLocation &Loc, ASTClass *Class, ASTClassScopes *Scopes, ASTType *Type,
                         std::string &Name) :
        ASTVar(ASTVarKind::VAR_FIELD, Type, Name),
        Loc(Loc), Class(Class), Scopes(Scopes) {

}

const SourceLocation &ASTClassVar::getLocation() const {
    return Loc;
}

const ASTClass *ASTClassVar::getClass() const {
    return Class;
}

const std::string &ASTClassVar::getComment() const {
    return Comment;
}

ASTClassScopes *ASTClassVar::getScopes() const {
    return Scopes;
}

ASTExpr *ASTClassVar::getExpr() const {
    return Expr;
}

CodeGenVarBase *ASTClassVar::getCodeGen() const {
    return CodeGen;
}

void ASTClassVar::setCodeGen(CodeGenClassVar *CGCV) {
    CodeGen = CGCV;
}

std::string ASTClassVar::str() const {
    return ASTVar::str();
}