//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTNameSpace.cpp - AST Namespace implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// This file implements the ASTNameSpace interface.
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTNameSpace.h"

using namespace fly;

ASTNameSpace::ASTNameSpace(llvm::StringRef NS) : NameSpace(NS) {}

ASTNameSpace::~ASTNameSpace() {
    Nodes.clear();
    Vars.clear();
}

const llvm::StringRef &fly::ASTNameSpace::getNameSpace() const {
    return NameSpace;
}

const llvm::StringMap<ASTNode*> &ASTNameSpace::getNodes() const {
    return Nodes;
}

const llvm::StringMap<GlobalVarDecl *> &ASTNameSpace::getVars() const {
    return Vars;
}

const llvm::StringMap<FunctionDecl *> &ASTNameSpace::getFunctions() const {
    return Functions;
}

const llvm::StringMap<ClassDecl *> &ASTNameSpace::getClasses() const {
    return Classes;
}
