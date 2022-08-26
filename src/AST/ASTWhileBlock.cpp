//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTWhileBlock.cpp - While Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTWhileBlock.h"

using namespace fly;

ASTWhileBlock::ASTWhileBlock(ASTBlock *Parent, const SourceLocation &Loc, ASTExpr *Cond) : ASTBlock(Parent, Loc),
    Condition(Cond) {

}

enum ASTBlockKind ASTWhileBlock::getBlockKind() const {
    return StmtKind;
}

ASTExpr *ASTWhileBlock::getCondition() {
    return Condition;
}
