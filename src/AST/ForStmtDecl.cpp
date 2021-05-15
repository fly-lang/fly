//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ForStmtDecl.cpp - For Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ForStmtDecl.h"

using namespace fly;

ForStmtDecl::ForStmtDecl(const SourceLocation &Loc, StmtDecl *Parent) : StmtDecl(Loc, Parent) {

}

enum StmtKind ForStmtDecl::getStmtKind() const {
    return StmtKind;
}

const StmtDecl *ForStmtDecl::getInit() const {
    return Init;
}

const GroupExpr *ForStmtDecl::getCondition() const {
    return Cond;
}

const StmtDecl *ForStmtDecl::getPost() const {
    return Post;
}
