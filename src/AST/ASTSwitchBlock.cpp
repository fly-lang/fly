//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTSwitchBlock.cpp - Switch Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTSwitchBlock.h"
#include "AST/ASTExpr.h"

using namespace fly;

ASTSwitchBlock::ASTSwitchBlock(ASTBlock *Parent, const SourceLocation &Loc) :
        ASTBlock(Parent, Loc, ASTBlockKind::BLOCK_SWITCH) {

}

ASTExpr *ASTSwitchBlock::getExpr() const {
    return Expr;
}

ASTBlock *ASTSwitchBlock::getParent() const {
    return (ASTBlock *) Parent;
}

std::vector<ASTSwitchCaseBlock *> &ASTSwitchBlock::getCases() {
    return Cases;
}

ASTSwitchDefaultBlock *ASTSwitchBlock::getDefault() {
    return Default;
}

ASTSwitchCaseBlock::ASTSwitchCaseBlock(ASTSwitchBlock *SwitchBlock, const SourceLocation &Loc) :
    ASTBlock(SwitchBlock->getParent(), Loc, ASTBlockKind::BLOCK_SWITCH_CASE), SwitchBlock(SwitchBlock) {
    SwitchBlock->Cases.push_back(this);
}

ASTExpr *ASTSwitchCaseBlock::getExpr() {
    return Expr;
}

ASTSwitchDefaultBlock::ASTSwitchDefaultBlock(ASTSwitchBlock *SwitchBlock, const SourceLocation &Loc) :
    ASTBlock(SwitchBlock->getParent(), Loc, ASTBlockKind::BLOCK_SWITCH_DEFAULT), SwitchBlock(SwitchBlock) {
    SwitchBlock->Default = this;
}
