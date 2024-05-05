//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTSwitchBlock.cpp - AST Switch Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTSwitchBlock.h"

using namespace fly;

ASTSwitchBlock::ASTSwitchBlock(const SourceLocation &Loc) :
        ASTBlock(Loc, ASTBlockKind::BLOCK_SWITCH) {

}

ASTVarRef *ASTSwitchBlock::getVarRef() const {
    return VarRef;
}

llvm::SmallVector<ASTSwitchCaseBlock *, 8> &ASTSwitchBlock::getCases() {
    return Cases;
}

ASTBlock *ASTSwitchBlock::getDefault() {
    return Default;
}

std::string ASTSwitchBlock::str() const {
    return Logger("ASTSwitchBlock").
           Super(ASTBlock::str()).
           End();
}

ASTSwitchCaseBlock::ASTSwitchCaseBlock(const SourceLocation &Loc) :
    ASTBlock(Loc, ASTBlockKind::BLOCK) {
}

ASTValue *ASTSwitchCaseBlock::getValue() {
    return Value;
}

ASTType *ASTSwitchCaseBlock::getType() const {
    return Type;
}

std::string ASTSwitchCaseBlock::str() const {
    return Logger("ASTSwitchCaseBlock").
           Super(ASTBlock::str()).
           End();
}
