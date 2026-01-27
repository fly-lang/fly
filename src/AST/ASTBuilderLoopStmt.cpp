//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBuilderLoopStmt.cpp - The Sema Builder Loop Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBuilderLoopStmt.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTLoopInStmt.h"

using namespace fly;

ASTBuilderLoopStmt::ASTBuilderLoopStmt() {

}

ASTBuilderLoopStmt *ASTBuilderLoopStmt::Create(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    ASTBuilderLoopStmt *Builder = new ASTBuilderLoopStmt();
    Builder->LoopStmt = new ASTLoopStmt(Loc);

    // Inner Stmt
    Parent->getContent().push_back(Builder->LoopStmt);
    Builder->LoopStmt->Parent = Parent;
    return Builder;
}

ASTBuilderLoopStmt *ASTBuilderLoopStmt::setCycle(ASTExpr *Expr, ASTBlockStmt *Stmt) {
    LoopStmt->Expr = Expr;
    LoopStmt->Loop = Stmt;
    return this;
}

ASTBuilderLoopStmt *ASTBuilderLoopStmt::setInit(ASTBlockStmt *BlockStmt) {
	for (auto &Stmt : BlockStmt->getContent()) {
		LoopStmt->Init.push_back(Stmt);
	}
	return this;
}

ASTBuilderLoopStmt *ASTBuilderLoopStmt::setPost(ASTBlockStmt *BlockStmt) {
	for (auto &Stmt : BlockStmt->getContent()) {
		LoopStmt->Post.push_back(Stmt);
	}
	return this;
}

void ASTBuilderLoopStmt::VerifyConditionAtEnd() {
    LoopStmt->VerifyConditionAtEnd = true;
}
