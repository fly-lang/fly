//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTForBlock.cpp - For Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTForBlock.h"

using namespace fly;

ASTForBlock::ASTForBlock(const SourceLocation &Loc, ASTBlock *Parent) : ASTBlock(Loc, Parent) {
    Init = new ASTBlock(Loc, this);
    Cond = new ASTBlock(Loc, Init);
    Post = new ASTBlock(Loc, Init);
    Loop = new ASTBlock(Loc, Init);
}

enum BlockStmtKind ASTForBlock::getBlockKind() const {
    return StmtKind;
}

ASTBlock *ASTForBlock::getInit() {
    return Init;
}

ASTExpr *ASTForBlock::getCondition() {
    return CondExpr;
}

void ASTForBlock::setCond(ASTExpr *Expr) {
    CondExpr = Expr;
    ASTExprStmt *ExprStmt = new ASTExprStmt(Expr->getLocation(), Cond);
    ExprStmt->setExpr(CondExpr);
    Cond->addExprStmt(ExprStmt);
}

ASTBlock *ASTForBlock::getPost() {
    return Post;
}

ASTBlock *ASTForBlock::getLoop() {
    return Loop;
}