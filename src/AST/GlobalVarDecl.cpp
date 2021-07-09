//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/GlobalVarDecl.cpp - Global Var declaration implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/GlobalVarDecl.h"
#include "AST/ASTNode.h"
#include "AST/ASTNameSpace.h"

using namespace fly;

GlobalVarDecl::GlobalVarDecl(ASTNode *Node, SourceLocation &Loc, TypeBase *Type,
                             const llvm::StringRef &Name) : Kind(TopDeclKind::DECL_GLOBALVAR), TopDecl(Node, Loc),
                            VarDecl(Type, Name, Node->getNameSpace()->getNameSpace()) {

}

TopDeclKind GlobalVarDecl::getKind() const {
    return Kind;
}

CodeGenGlobalVar *GlobalVarDecl::getCodeGen() const {
    return CodeGen;
}

void GlobalVarDecl::setCodeGen(CodeGenGlobalVar *codeGen) {
    CodeGen = codeGen;
}
