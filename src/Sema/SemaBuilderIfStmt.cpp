//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBuilderIfStmt.cpp - The Sema Builder If Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBuilderIfStmt.h"
#include "Sema/SemaBuilder.h"
#include "AST/ASTIfStmt.h"

using namespace fly;

SemaBuilderIfStmt::SemaBuilderIfStmt(Sema &S, ASTStmt *Parent) : S(S), Parent(Parent) {

}

SemaBuilderIfStmt *SemaBuilderIfStmt::Create(Sema &S, ASTStmt *Parent) {
    return new SemaBuilderIfStmt(S, Parent);
}

SemaBuilderIfStmt *SemaBuilderIfStmt::If(const SourceLocation &Loc, ASTExpr *Expr, ASTStmt *Stmt) {
    IfStmt = new ASTIfStmt(Loc);
    IfStmt->Parent = Parent;
    IfStmt->Function = Parent->Function;
    IfStmt->Condition = Expr;
    IfStmt->Stmt = Stmt;
    // Inner Stmt
    Stmt->Parent = IfStmt;
    Stmt->Function = IfStmt->Function;
    return this;
}


SemaBuilderIfStmt *SemaBuilderIfStmt::ElseIf(const SourceLocation &Loc, ASTExpr *Expr, ASTStmt *Stmt) {
    ASTElsif *Elsif = new ASTElsif(Loc);
    Elsif->Condition = Expr;
    Elsif->Stmt = Stmt;
    // Inner Stmt
    Stmt->Parent = IfStmt;
    Stmt->Function = IfStmt->Function;
    // add to elsif list
    IfStmt->Elsif.push_back(Elsif);
    return this;
}

SemaBuilderIfStmt *SemaBuilderIfStmt::Else(const SourceLocation &Loc, ASTStmt *Stmt) {
    // Inner Stmt
    Stmt->Parent = IfStmt;
    Stmt->Function = IfStmt->Function;
    // set else
    IfStmt->Else = Stmt;
    return this;
}
