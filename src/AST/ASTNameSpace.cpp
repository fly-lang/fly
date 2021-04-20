//
// Created by marco on 4/14/21.
//

#include "AST/ASTNameSpace.h"

using namespace fly;

ASTNameSpace::ASTNameSpace(const llvm::StringRef &NS) : NameSpace(NS) {}

const llvm::StringRef &fly::ASTNameSpace::getNameSpace() const {
    return NameSpace;
}

const llvm::StringMap<ASTNode*> &ASTNameSpace::getNodes() const {
    return Nodes;
}