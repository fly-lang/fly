//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBuilderSwitchStmt.cpp - The Sema Builder Switch Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBuilderSwitchStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTBlockStmt.h"

using namespace fly;

ASTBuilderSwitchStmt::ASTBuilderSwitchStmt(ASTBlockStmt *Parent) : Parent(Parent)  {

}

ASTBuilderSwitchStmt *ASTBuilderSwitchStmt::Create(ASTBlockStmt *Parent) {
	return new ASTBuilderSwitchStmt(Parent);
}

ASTBuilderSwitchStmt *ASTBuilderSwitchStmt::Switch(const SourceLocation &Loc, ASTExpr *Expr) {
    SwitchStmt = new ASTSwitchStmt(Loc);

    // Inner stmt
    Parent->getContent().push_back(SwitchStmt);
    SwitchStmt->Parent = Parent;
    SwitchStmt->Var = Expr;
    return this;
}

ASTBuilderSwitchStmt *ASTBuilderSwitchStmt::Case(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt) {
    ASTRuleStmt *Case = new ASTRuleStmt(Loc);
    Case->Rule = Expr;
    Case->Stmt = Stmt;
	Case->Parent = SwitchStmt;
	Case->Function = SwitchStmt->Function;

    // add to switch list
    SwitchStmt->Cases.push_back(Case);
    return this;
}

ASTBuilderSwitchStmt *ASTBuilderSwitchStmt::Default(const SourceLocation &Loc, ASTBlockStmt *Stmt) {
    SwitchStmt->Default = Stmt;
    return this;
}

bool ASTBuilderSwitchStmt::hasDefault() {
	return SwitchStmt->Default != nullptr;
}
