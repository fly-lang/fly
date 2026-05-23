//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTBuilderLoopInStmt.cpp - AST builder for loop-in statements
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBuilderLoopInStmt.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTLoopInStmt.h"

using namespace fly;

ASTBuilderLoopInStmt::ASTBuilderLoopInStmt() {

}

ASTBuilderLoopInStmt *ASTBuilderLoopInStmt::Create(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTExpr *Item, ASTExpr *List, ASTBlockStmt *Stmt) {
	ASTBuilderLoopInStmt *Builder = new ASTBuilderLoopInStmt();
    Builder->LoopStmt = new ASTLoopInStmt(Loc, Item, List, Stmt);

    // Add to Parent block content
    Parent->getContent().push_back(Builder->LoopStmt);
    Builder->LoopStmt->setParent(Parent);

    // Set Stmt parent and function
    if (Stmt) {
        Stmt->setParent(Builder->LoopStmt);
    }

    return Builder;
}

ASTBuilderLoopInStmt *ASTBuilderLoopInStmt::setCycle(ASTBlockStmt *Stmt) {
    LoopStmt->Stmt = Stmt;
    return this;
}
