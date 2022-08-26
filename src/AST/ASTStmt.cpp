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

ASTStmt::ASTStmt(const SourceLocation &Loc) : Location(Loc) {

}

ASTStmt::ASTStmt(ASTBlock *Parent, const SourceLocation &Loc) : Parent(Parent), Location(Loc) {

}

const SourceLocation &ASTStmt::getLocation() const {
    return Location;
}

ASTBlock *ASTStmt::getParent() const {
    return Parent;
}
