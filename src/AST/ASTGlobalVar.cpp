//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTGlobalVar.cpp - Global Var declaration implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTGlobalVar.h"
#include "AST/ASTNode.h"
#include "AST/ASTNameSpace.h"

using namespace fly;

ASTGlobalVar::ASTGlobalVar(const SourceLocation &Loc, ASTNode *Node, ASTType *Type, const std::string Name,
                           ASTTopScopes *Scopes) :
        ASTTopDef(Loc, Node, ASTTopDefKind::DEF_GLOBALVAR, Scopes),
        VarKind(ASTVarKind::VAR_GLOBAL), Type(Type), Name(Name) {

}

std::string ASTGlobalVar::getName() const {
    return Name;
}

ASTVarKind ASTGlobalVar::getVarKind() {
    return VarKind;
}

ASTType *ASTGlobalVar::getType() const {
    return Type;
}

ASTExpr *ASTGlobalVar::getExpr() const {
    return Expr;
}

CodeGenGlobalVar *ASTGlobalVar::getCodeGen() const {
    return CodeGen;
}

void ASTGlobalVar::setCodeGen(CodeGenGlobalVar *CG) {
    CodeGen = CG;
}

std::string ASTGlobalVar::str() const {
    return Logger("ASTGlobalVar").
            Super(ASTTopDef::str()).
            Attr("Type", Type).
            Attr("Name", Name).
            Attr("VarKind", (uint64_t) VarKind).
            Attr("Expr", Expr).
            End();
}
