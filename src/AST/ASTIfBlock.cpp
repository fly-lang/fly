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

ASTIfBlock::ASTIfBlock(const SourceLocation &Loc, ASTExpr *Condition) : Condition(Condition),
    ASTBlock(Loc) {

}

std::vector<ASTElsifBlock *> ASTIfBlock::getElsifBlocks() {
    return ElsifBlocks;
}

ASTElseBlock *ASTIfBlock::getElseBlock() {
    return ElseBlock;
}

ASTExpr *ASTIfBlock::getCondition() {
    return Condition;
}

enum ASTBlockKind ASTIfBlock::getBlockKind() const {
    return StmtKind;
}

ASTElsifBlock::ASTElsifBlock(const SourceLocation &Loc, ASTBlock *Parent, ASTExpr *Condition) :
    ASTBlock(Loc, Parent), Condition(Condition) {
    
}

enum ASTBlockKind ASTElsifBlock::getBlockKind() const {
    return StmtKind;
}

ASTExpr *ASTElsifBlock::getCondition() {
    return Condition;
}

ASTElseBlock::ASTElseBlock(const SourceLocation &Loc, ASTBlock *Parent) : ASTBlock(Loc, Parent) {
    
}

enum ASTBlockKind ASTElseBlock::getBlockKind() const {
    return StmtKind;
}
