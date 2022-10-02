//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTStmt.cpp - Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTStmt.h"
#include "AST/ASTExpr.h"
#include "AST/ASTBlock.h"

using namespace fly;

ASTStmt::ASTStmt(ASTStmt *Parent, const SourceLocation &Loc, StmtKind Kind) :
        Top(Parent ? Parent->Top : nullptr), Parent(Parent), Location(Loc), Kind(Kind) {
}

ASTStmt *ASTStmt::getParent() const {
    return Parent;
}

const SourceLocation &ASTStmt::getLocation() const {
    return Location;
}

StmtKind ASTStmt::getKind() const {
    return Kind;
}


