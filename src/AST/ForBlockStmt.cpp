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

}

enum BlockStmtKind ForBlockStmt::getBlockKind() const {
    return StmtKind;
}

const BlockStmt *ForBlockStmt::getInit() const {
    return Init;
}

const GroupExpr *ForBlockStmt::getCondition() const {
    return Cond;
}

const BlockStmt *ForBlockStmt::getPost() const {
    return Post;
}
