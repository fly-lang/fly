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

ASTStmt::ASTStmt(const SourceLocation &Loc, ASTBlock *Parent) : Location(Loc), Top(Parent->getTop()),
    Parent(Parent) {

}

ASTStmt::ASTStmt(const SourceLocation &Loc, ASTFunc *Top, ASTBlock *Parent) : Location(Loc), Top(Top), Parent(Parent) {

}

const SourceLocation &ASTStmt::getLocation() const {
    return Location;
}

ASTBlock *ASTStmt::getParent() const {
    return Parent;
}

ASTFunc *ASTStmt::getTop() const {
    return Top;
}

ASTExprStmt::ASTExprStmt(const SourceLocation &Loc, ASTBlock *Block) :
        ASTStmt(Loc, Block) {

}

StmtKind ASTExprStmt::getKind() const {
    return STMT_EXPR;
}

ASTExpr *ASTExprStmt::getExpr() const {
    return Expr;
}

void ASTExprStmt::setExpr(ASTExpr *E) {
    Expr = E;
}

std::string ASTExprStmt::str() const {
    return "{ Expr=" + Expr->str() + " }";
}