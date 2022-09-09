//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTForBlock.cpp - For Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTForBlock.h"
#include "AST/ASTExpr.h"

using namespace fly;

ASTForBlock::ASTForBlock(ASTBlock *Parent, const SourceLocation &Loc) : ASTBlock(Parent, Loc, BLOCK_FOR) {

}

ASTForBlock::~ASTForBlock() {
    delete Condition;
    delete Post;
    delete Loop;
}

ASTBlock *ASTForBlock::getParent() const {
    return (ASTBlock *) Parent;
}

ASTExpr *ASTForBlock::getCondition() {
    return Condition;
}

ASTForPostBlock *ASTForBlock::getPost() {
    return Post;
}

ASTForLoopBlock *ASTForBlock::getLoop() {
    return Loop;
}

ASTForLoopBlock::ASTForLoopBlock(ASTForBlock *ForBlock, const SourceLocation &Loc) :
    ASTBlock(ForBlock, Loc, BLOCK_FOR_LOOP) {
    ForBlock->Loop = this;
}

ASTForPostBlock::ASTForPostBlock(ASTForBlock *ForBlock, const SourceLocation &Loc) :
    ASTBlock(ForBlock, Loc, BLOCK_FOR_POST) {
    ForBlock->Post = this;
}