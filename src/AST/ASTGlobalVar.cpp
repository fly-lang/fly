//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTGlobalVar.cpp - AST Global Var implementation
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

ASTGlobalVar::ASTGlobalVar(const SourceLocation &Loc, ASTNode *Node, ASTType *Type, llvm::StringRef Name,
                           ASTScopes *Scopes) :
        ASTVar(ASTVarKind::VAR_GLOBAL, Loc, Type, Name, Scopes), Node(Node) {

}

ASTTopDefKind ASTGlobalVar::getTopDefKind() const {
    return TopDefKind;
}

ASTNode *ASTGlobalVar::getNode() const {
    return Node;
}

ASTNameSpace *ASTGlobalVar::getNameSpace() const {
    return Node->getNameSpace();
}

llvm::StringRef ASTGlobalVar::getName() const {
    return ASTVar::getName();
}

ASTVarStmt *ASTGlobalVar::getInit() const {
    return Init;
}

void ASTGlobalVar::setInit(ASTVarStmt *varDefine) {
    Init = varDefine;
}

CodeGenGlobalVar *ASTGlobalVar::getCodeGen() const {
    return CodeGen;
}

void ASTGlobalVar::setCodeGen(CodeGenGlobalVar *CG) {
    CodeGen = CG;
}

std::string ASTGlobalVar::print() const {
    return getNameSpace()->print() + "." + getName().data();
}

std::string ASTGlobalVar::str() const {
    return Logger("ASTGlobalVar").
            Super(ASTVar::str()).
            End();
}
