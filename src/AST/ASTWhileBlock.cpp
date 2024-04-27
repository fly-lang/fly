//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTWhileBlock.cpp - AST While Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTWhileBlock.h"

using namespace fly;

ASTWhileBlock::ASTWhileBlock(ASTBlock *Parent, const SourceLocation &Loc) :
        ASTBlock(Parent, Loc, ASTBlockKind::BLOCK_WHILE) {

}

ASTBlock *ASTWhileBlock::getParent() const {
    return (ASTBlock *) Parent;
}

ASTExpr *ASTWhileBlock::getCondition() {
    return Condition;
}

std::string ASTWhileBlock::str() const {
    return Logger("ASTWhileBlock").
            Super(ASTBlock::str()).
            End();
}
