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

ASTIfBlock::ASTIfBlock(ASTBlock *Parent, const SourceLocation &Loc) :
        ASTBlock(Parent, Loc, ASTBlockKind::BLOCK_IF) {

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

std::string ASTIfBlock::str() const {
    return Logger("ASTIfBlock").
           Super(ASTBlock::str()).
           End();
}

ASTElsifBlock::ASTElsifBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc) :
    ASTBlock(IfBlock->getParent(), Loc, ASTBlockKind::BLOCK_ELSIF), IfBlock(IfBlock) {
    IfBlock->ElsifBlocks.push_back(this);
}

ASTExpr *ASTElsifBlock::getCondition() {
    return Condition;
}

std::string ASTElsifBlock::str() const {
    return Logger("ASTElsifBlock").
           Super(ASTBlock::str()).
           End();
}

ASTElseBlock::ASTElseBlock(ASTIfBlock *IfBlock, const SourceLocation &Loc) :
    ASTBlock(IfBlock->getParent(), Loc, ASTBlockKind::BLOCK_ELSE), IfBlock(IfBlock) {
    IfBlock->ElseBlock = this;
}

std::string ASTElseBlock::str() const {
    return Logger("ASTElseBlock").
           Super(ASTBlock::str()).
           End();
}
