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

ASTForBlock::ASTForBlock(const SourceLocation &Loc) : ASTBlock(Loc) {

}

ASTForBlock::~ASTForBlock() {
    delete Condition;
    delete Post;
    delete Loop;
}

enum ASTBlockKind ASTForBlock::getBlockKind() const {
    return StmtKind;
}

ASTExpr *ASTForBlock::getCondition() {
    return Condition;
}

ASTBlock *ASTForBlock::getPost() {
    return Post;
}

ASTBlock *ASTForBlock::getLoop() {
    return Loop;
}