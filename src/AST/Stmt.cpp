//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/Stmt.cpp - Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/Stmt.h"
#include "AST/BlockStmt.h"

using namespace fly;

Stmt::Stmt(const SourceLocation &Loc, BlockStmt *Parent) : Location(Loc), Top(Parent->getTop()),
    Parent(Parent) {

}

Stmt::Stmt(const SourceLocation &Loc, FuncDecl *Top, BlockStmt *Parent) : Location(Loc), Top(Top), Parent(Parent) {

}

const SourceLocation &Stmt::getLocation() const {
    return Location;
}

const BlockStmt *Stmt::getParent() const {
    return Parent;
}

FuncDecl *Stmt::getTop() const {
    return Top;
}
