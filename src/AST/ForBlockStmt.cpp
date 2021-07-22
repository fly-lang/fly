//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ForBlockStmt.cpp - For Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ForBlockStmt.h"

using namespace fly;

ForBlockStmt::ForBlockStmt(const SourceLocation &Loc, BlockStmt *Parent) : BlockStmt(Loc, Parent) {
    Init = new BlockStmt(Loc, this);
    Cond = new GroupExpr();
    Post = new BlockStmt(Loc, Init);
    Loop = new BlockStmt(Loc, Init);
}

enum BlockStmtKind ForBlockStmt::getBlockKind() const {
    return StmtKind;
}

BlockStmt *ForBlockStmt::getInit() {
    return Init;
}

GroupExpr *ForBlockStmt::getCondition() {
    return Cond;
}

BlockStmt *ForBlockStmt::getPost() {
    return Post;
}

BlockStmt *ForBlockStmt::getLoop() {
    return Loop;
}