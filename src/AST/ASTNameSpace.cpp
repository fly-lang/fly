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

ASTNameSpace::ASTNameSpace(const llvm::StringRef &NS) : NameSpace(NS) {}

ASTNameSpace::~ASTNameSpace() {
    for (auto &N : Nodes)
        Nodes.erase(N.getKey());
    for (auto &V : Vars)
        Vars.erase(V.getKey());
}

const llvm::StringRef &fly::ASTNameSpace::getNameSpace() const {
    return NameSpace;
}

const llvm::StringMap<ASTNode*> &ASTNameSpace::getNodes() const {
    return Nodes;
}
