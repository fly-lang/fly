//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTIfBlock.cpp - If Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTIfBlock.h"
#include "AST/ASTFunction.h"
#include "AST/ASTNode.h"
#include "AST/ASTContext.h"

using namespace fly;

ASTIfBlock::ASTIfBlock(ASTBlock *Parent, const SourceLocation &Loc) : ASTBlock(Parent, Loc, BLOCK_IF) {

}

ASTBlock *ASTIfBlock::getParent() const {
    return (ASTBlock *) Parent;
}

ASTExpr *ASTIfBlock::getCondition() {
    return Condition;
}

std::vector<ASTElsifBlock *> ASTIfBlock::getElsifBlocks() {
    return ElsifBlocks;
}

ASTElseBlock *ASTIfBlock::getElseBlock() {
    return ElseBlock;
}

ASTElsifBlock::ASTElsifBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc) :
    ASTBlock(IfBlock->getParent(), Loc, BLOCK_ELSIF), IfBlock(IfBlock) {
    IfBlock->ElsifBlocks.push_back(this);
}

ASTExpr *ASTElsifBlock::getCondition() {
    return Condition;
}

ASTElseBlock::ASTElseBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc) :
    ASTBlock(IfBlock->getParent(), Loc, BLOCK_ELSE), IfBlock(IfBlock) {
    IfBlock->ElseBlock = this;
}
