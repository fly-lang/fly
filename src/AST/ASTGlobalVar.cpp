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

ASTGlobalVar::ASTGlobalVar(ASTNode *Node, SourceLocation &Loc, ASTType *Type,
                             const llvm::StringRef &Name) : Kind(TopDeclKind::DECL_GLOBALVAR), ASTTopDecl(Node, Loc),
                                                            ASTVar(Type, Name, Node->getNameSpace()->getNameSpace()) {

}

TopDeclKind ASTGlobalVar::getKind() const {
    return Kind;
}

CodeGenGlobalVar *ASTGlobalVar::getCodeGen() const {
    return CodeGen;
}

void ASTGlobalVar::setCodeGen(CodeGenGlobalVar *codeGen) {
    CodeGen = codeGen;
}
