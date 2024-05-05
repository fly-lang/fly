//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTIfBlock.cpp - AST If Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTIfBlock.h"

using namespace fly;

ASTIfBlock::ASTIfBlock(const SourceLocation &Loc) :
        ASTBlock(Loc, ASTBlockKind::BLOCK_IF) {

}

ASTBlock *ASTIfBlock::getParent() const {
    return (ASTBlock *) Parent;
}

ASTExpr *ASTIfBlock::getCondition() {
    return Condition;
}

llvm::SmallVector<ASTElsifBlock *, 8> ASTIfBlock::getElsifBlocks() {
    return ElsifBlocks;
}

ASTBlock *ASTIfBlock::getElseBlock() {
    return ElseBlock;
}

std::string ASTIfBlock::str() const {
    return Logger("ASTIfBlock").
           Super(ASTBlock::str()).
           End();
}

ASTElsifBlock::ASTElsifBlock(const SourceLocation &Loc) :
    ASTBlock(Loc, ASTBlockKind::BLOCK) {
}

ASTExpr *ASTElsifBlock::getCondition() {
    return Condition;
}

std::string ASTElsifBlock::str() const {
    return Logger("ASTElsifBlock").
           Super(ASTBlock::str()).
           End();
}
