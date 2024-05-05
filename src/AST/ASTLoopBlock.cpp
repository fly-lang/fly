//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTWhileBlock.cpp - AST While Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTLoopBlock.h"

using namespace fly;

ASTLoopBlock::ASTLoopBlock(const SourceLocation &Loc) :
        ASTBlock(Loc, ASTBlockKind::BLOCK_LOOP) {

}

ASTBlock *ASTLoopBlock::getParent() const {
    return (ASTBlock *) Parent;
}

ASTExpr *ASTLoopBlock::getCondition() {
    return Condition;
}

bool ASTLoopBlock::isVerifyConditionOnEnd() const {
    return VerifyConditionOnEnd;
}

ASTBlock *ASTLoopBlock::getInit() const {
    return Init;
}

ASTBlock *ASTLoopBlock::getPost() const {
    return Post;
}

std::string ASTLoopBlock::str() const {
    return Logger("ASTWhileBlock").
            Super(ASTBlock::str()).
            End();
}
