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

ASTGlobalVar::ASTGlobalVar(SourceLocation &Loc, ASTNode *Node, ASTType *Type, const std::string Name,
                           VisibilityKind Visibility, bool Constant) :
    ASTTopDecl(Loc, Node, TopDeclKind::DECL_GLOBALVAR, Visibility),
    ASTVar(VAR_GLOBAL, Type, Name, Constant) {

}

const std::string &ASTGlobalVar::getName() const {
    return ASTVar::getName();
}

ASTVarRef *ASTGlobalVar::CreateVarRef() {
    ASTVarRef *VarRef = new ASTVarRef(Location, Name, NameSpace->getName());
    VarRef->setDecl(this);
    return VarRef;
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
