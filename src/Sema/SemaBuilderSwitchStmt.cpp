//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBuilderSwitchStmt.cpp - The Sema Builder Switch Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBuilderSwitchStmt.h"
#include "Sema/SemaBuilder.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTExpr.h"

using namespace fly;

SemaBuilderSwitchStmt::SemaBuilderSwitchStmt(Sema &S, ASTStmt *Parent) : S(S), Parent(Parent)  {

}

SemaBuilderSwitchStmt *SemaBuilderSwitchStmt::Create(Sema &S, ASTStmt *Parent) {
    return new SemaBuilderSwitchStmt(S, Parent);
}

SemaBuilderSwitchStmt *SemaBuilderSwitchStmt::Switch(const SourceLocation &Loc, ASTExpr *Expr) {
    SwitchStmt = new ASTSwitchStmt(Loc);
    SwitchStmt->Parent = Parent;
    SwitchStmt->Function = Parent->Function;
    if (S.Validator->CheckVarRefExpr(Expr))
        SwitchStmt->VarRef = ((ASTVarRefExpr *) Expr)->getVarRef();
    return this;
}

SemaBuilderSwitchStmt *SemaBuilderSwitchStmt::Case(const SourceLocation &Loc, ASTExpr *Expr, ASTStmt *Stmt) {
    ASTSwitchCase *Case = new ASTSwitchCase(Loc);
    Case->Value = (ASTValueExpr *) Expr;
    Case->Stmt = Stmt;
    // Inner Stmt
    Stmt->Parent = SwitchStmt;
    Stmt->Function = SwitchStmt->Function;
    // add to switch list
    SwitchStmt->Cases.push_back(Case);
    return this;
}

SemaBuilderSwitchStmt *SemaBuilderSwitchStmt::Default(const SourceLocation &Loc, ASTStmt *Stmt) {
    SwitchStmt->Default = Stmt;
    // Inner Stmt
    Stmt->Parent = SwitchStmt;
    Stmt->Function = SwitchStmt->Function;
    return this;
}
