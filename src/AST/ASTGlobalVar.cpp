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

ASTGlobalVar::ASTGlobalVar(SourceLocation &Loc, ASTNode *Node, ASTType *Type, const std::string Name) :
    ASTTopDecl(Loc, Node, TopDeclKind::DECL_GLOBALVAR),
    ASTVar(Type, Name, Node->getNameSpace()->getName(), true) {

}

const std::string &ASTGlobalVar::getName() const {
    return ASTVar::getName();
}

ASTExpr *ASTGlobalVar::getExpr() const {
    return Expr;
}

void ASTGlobalVar::setExpr(ASTExpr *E) {
    Expr = (ASTValueExpr *)E;
}

CodeGenGlobalVar *ASTGlobalVar::getCodeGen() const {
    return CodeGen;
}

void ASTGlobalVar::setCodeGen(CodeGenGlobalVar *codeGen) {
    CodeGen = codeGen;
}

std::string ASTGlobalVar::str() const {
    return "{ " +
        ASTTopDecl::str() +
        ", " + ASTVar::str() +
        ", Expr= " + (Expr ? Expr->str() : "{}") +
        " }";
}
