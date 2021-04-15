//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTContext.cpp - AST Context implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// This file implements the ASTContext interface.
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTContext.h"

using namespace fly;

bool ASTContext::AddNode(ASTNode &AST) {
    assert(AST.getFileID().isValid() && "ASTUnit FileID is not valid!");
    Nodes.insert(std::make_pair(AST.getFileID(), &AST));
    return true;
}

bool ASTContext::DelNode(ASTNode &AST) {
    assert(AST.getFileID().isValid() && "ASTUnit FileID is not valid!");
    Nodes.erase(AST.getFileID());
    return true;
}

const DenseMap<FileID, ASTNode*> &ASTContext::getNodes() const {
    return Nodes;
}
