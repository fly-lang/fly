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
#include "AST/ASTBlockStmt.h"

using namespace fly;

SemaBuilderIfStmt::SemaBuilderIfStmt(Sema &S, ASTBlockStmt *Parent) : S(S), Parent(Parent) {

}

SemaBuilderIfStmt *SemaBuilderIfStmt::Create(Sema &S, ASTBlockStmt *Parent) {
    return new SemaBuilderIfStmt(S, Parent);
}

SemaBuilderIfStmt *SemaBuilderIfStmt::If(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt) {
    IfStmt = new ASTIfStmt(Loc);
    IfStmt->Rule = Expr;
    IfStmt->Stmt = Stmt;
    // Inner Stmt
    Parent->Content.push_back(IfStmt);
    IfStmt->Parent = Parent;
    IfStmt->Function = Parent->Function;
    // Inner Stmt
    Stmt->Parent = IfStmt;
    Stmt->Function = IfStmt->Function;
    return this;
}


SemaBuilderIfStmt *SemaBuilderIfStmt::ElseIf(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt) {
    ASTRuleStmt *Elsif = new ASTRuleStmt(Loc);
    Elsif->Rule = Expr;
    Elsif->Stmt = Stmt;
    // Inner Stmt
    Stmt->Parent = IfStmt;
    Stmt->Function = IfStmt->Function;
    // add to elsif list
    IfStmt->Elsif.push_back(Elsif);
    return this;
}

SemaBuilderIfStmt *SemaBuilderIfStmt::Else(const SourceLocation &Loc, ASTBlockStmt *Stmt) {
    // Inner Stmt
    Stmt->Parent = IfStmt;
    Stmt->Function = IfStmt->Function;
    // set else
    IfStmt->Else = Stmt;
    return this;
}
