//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBuilderSwitchStmt.cpp - The Sema Builder Switch Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBuilderSwitchStmt.h"
#include "Sema/ASTBuilder.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTExpr.h"

#include <Sema/Sema.h>
#include <Sema/SemaValidator.h>

using namespace fly;

SemaBuilderSwitchStmt::SemaBuilderSwitchStmt(Sema &S, ASTBlockStmt *Parent) : S(S), Parent(Parent)  {

}

SemaBuilderSwitchStmt *SemaBuilderSwitchStmt::Create(Sema &S, ASTBlockStmt *Parent) {
    return new SemaBuilderSwitchStmt(S, Parent);
}

SemaBuilderSwitchStmt *SemaBuilderSwitchStmt::Switch(const SourceLocation &Loc, ASTExpr *Expr) {
    SwitchStmt = new ASTSwitchStmt(Loc);

    // Inner stmt
    Parent->Content.push_back(SwitchStmt);
    SwitchStmt->Parent = Parent;
    SwitchStmt->Function = Parent->Function;
    if (S.getValidator().CheckVarRefExpr(Expr))
        SwitchStmt->VarRef = ((ASTVarRefExpr *) Expr)->getVarRef();
    return this;
}

SemaBuilderSwitchStmt *SemaBuilderSwitchStmt::Case(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt) {
    ASTRuleStmt *Case = new ASTRuleStmt(Loc);
    Case->Rule = Expr;
    Case->Stmt = Stmt;
	Case->Parent = SwitchStmt;
	Case->Function = SwitchStmt->Function;

    // Inner Stmt
    Stmt->Parent = Case;
    Stmt->Function = SwitchStmt->Function;

    // add to switch list
    SwitchStmt->Cases.push_back(Case);
    return this;
}

SemaBuilderSwitchStmt *SemaBuilderSwitchStmt::Default(const SourceLocation &Loc, ASTBlockStmt *Stmt) {
    SwitchStmt->Default = Stmt;

    // Inner Stmt
    Stmt->Parent = SwitchStmt;
    Stmt->Function = SwitchStmt->Function;
    return this;
}

bool SemaBuilderSwitchStmt::hasDefault() {
	return SwitchStmt->Default != nullptr;
}
