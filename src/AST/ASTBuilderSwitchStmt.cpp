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

ASTBuilderSwitchStmt::ASTBuilderSwitchStmt() {

}

ASTBuilderSwitchStmt *ASTBuilderSwitchStmt::Create(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTExpr *Expr) {
	ASTBuilderSwitchStmt *Builder = new ASTBuilderSwitchStmt();

	Builder->SwitchStmt = new ASTSwitchStmt(Loc);
	Builder->SwitchStmt->setParent(Parent);

	// Inner stmt
	Parent->getContent().push_back(Builder->SwitchStmt);
	Builder->SwitchStmt->Expr = Expr;

	return Builder;
}

ASTBuilderSwitchStmt *ASTBuilderSwitchStmt::addCase(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt) {
    ASTRuleStmt *Case = new ASTRuleStmt(Loc);
    Case->Expr = Expr;
    Case->Stmt = Stmt;
	Case->Parent = Stmt;

    // add to switch list
    SwitchStmt->Cases.push_back(Case);
    return this;
}

ASTBuilderSwitchStmt *ASTBuilderSwitchStmt::setDefault(const SourceLocation &Loc, ASTBlockStmt *Stmt) {
    SwitchStmt->Default = Stmt;
    return this;
}

bool ASTBuilderSwitchStmt::hasDefault() {
	return SwitchStmt->Default != nullptr;
}
