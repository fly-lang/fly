//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBuilderIfStmt.cpp - The Sema Builder If Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBuilderIfStmt.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTBlockStmt.h"

using namespace fly;

ASTBuilderIfStmt::ASTBuilderIfStmt(ASTBlockStmt *Parent) : Parent(Parent) {

}

ASTBuilderIfStmt *ASTBuilderIfStmt::Create(ASTBlockStmt *Parent) {
    return new ASTBuilderIfStmt(Parent);
}

ASTBuilderIfStmt *ASTBuilderIfStmt::If(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt) {
    IfStmt = new ASTIfStmt(Loc);
    IfStmt->Rule = Expr;
    IfStmt->Stmt = Stmt;
    // Inner Stmt
    Parent->getContent().push_back(IfStmt);
    IfStmt->Parent = Parent;
    return this;
}


ASTBuilderIfStmt *ASTBuilderIfStmt::ElseIf(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt) {
    ASTRuleStmt *Elsif = new ASTRuleStmt(Loc);
    Elsif->Rule = Expr;
    Elsif->Stmt = Stmt;
    // add to elsif list
    IfStmt->Elsif.push_back(Elsif);
    return this;
}

ASTBuilderIfStmt *ASTBuilderIfStmt::Else(const SourceLocation &Loc, ASTBlockStmt *Stmt) {
    // set else
    IfStmt->Else = Stmt;
    return this;
}
