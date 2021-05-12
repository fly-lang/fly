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

const std::vector<VarDecl *> &ForStmtDecl::getInit() const {
    return Init;
}

const CondExpr *ForStmtDecl::getCondition() const {
    return Condition;
}

const std::vector<IncDecExpr *> &ForStmtDecl::getCount() const {
    return Count;
}
