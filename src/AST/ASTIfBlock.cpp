//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTIfBlock.cpp - If Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTIfBlock.h"
#include "AST/ASTFunc.h"
#include "AST/ASTNode.h"
#include "AST/ASTContext.h"
#include "Basic/Diagnostic.h"

using namespace fly;

ASTIfBlock::ASTIfBlock(const SourceLocation &Loc, ASTBlock *Parent) : ASTBlock(Loc, Parent) {
    // The UndefVars are copied from Parent to this if block
    UndefVars = Parent->UndefVars;
}

ASTIfBlock::ASTIfBlock(const SourceLocation &Loc, ASTBlock *Parent, ASTExpr *Condition) : Condition(Condition),
    ASTBlock(Loc, Parent) {
    // The UndefVars are copied from Parent to this if block
    UndefVars = Parent->UndefVars;
}

ASTElsifBlock *ASTIfBlock::AddElsifBlock(const SourceLocation &Loc, ASTExpr *Expr) {
    if (ElseBlock) {
        Parent->Top->getNode()->getContext().Diag(Loc, diag::err_elseif_after_else);
    }
    ASTElsifBlock *Elsif = new ASTElsifBlock(Loc, this, Expr); // TODO continue here!
    ElsifBlocks.push_back(Elsif);
    return Elsif;
}

std::vector<ASTElsifBlock *> ASTIfBlock::getElsifBlocks() {
    return ElsifBlocks;
}

ASTElseBlock *ASTIfBlock::AddElseBlock(const SourceLocation &Loc) {
    if (ElseBlock) {
        Parent->Top->getNode()->getContext().Diag(Loc, diag::err_else_after_else);
    }
    ElseBlock = new ASTElseBlock(Loc, this);
    return ElseBlock;
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
    ASTIfBlock(Loc, Parent, Condition) {
    
}

enum ASTBlockKind ASTElsifBlock::getBlockKind() const {
    return StmtKind;
}

ASTElseBlock::ASTElseBlock(const SourceLocation &Loc, ASTBlock *Parent) : ASTIfBlock(Loc, Parent) {
    
}

enum ASTBlockKind ASTElseBlock::getBlockKind() const {
    return StmtKind;
}
