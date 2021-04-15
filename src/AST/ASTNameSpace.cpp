//
// Created by marco on 4/14/21.
//

#include "AST/ASTNameSpace.h"

const llvm::StringRef &fly::ASTNameSpace::getNameSpace() const {
    return NameSpace;
}

fly::ASTNameSpace::ASTNameSpace(const llvm::StringRef &NS) : NameSpace(NS) {}
