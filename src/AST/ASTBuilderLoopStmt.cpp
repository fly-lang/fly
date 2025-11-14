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

using namespace fly;

ASTBuilderLoopStmt::ASTBuilderLoopStmt(ASTBlockStmt *Parent) : Parent(Parent) {

}

ASTBuilderLoopStmt *ASTBuilderLoopStmt::Create(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    ASTBuilderLoopStmt *Builder = new ASTBuilderLoopStmt(Parent);
    Builder->LoopStmt = new ASTLoopStmt(Loc);

    // Inner Stmt
    Parent->getContent().push_back(Builder->LoopStmt);
    Builder->LoopStmt->Parent = Parent;
    Builder->LoopStmt->Function = Parent->getFunction();
    return Builder;
}

ASTBuilderLoopStmt *ASTBuilderLoopStmt::Loop(ASTExpr *Expr, ASTBlockStmt *Stmt) {
    LoopStmt->Rule = Expr;
    LoopStmt->Stmt = Stmt;
    return this;
}

void ASTBuilderLoopStmt::Init(ASTBlockStmt *Stmt) {
    LoopStmt->Init = Stmt;
}

void ASTBuilderLoopStmt::Post(ASTBlockStmt *Stmt) {
    LoopStmt->Post = Stmt;
}

void ASTBuilderLoopStmt::VerifyConditionAtEnd() {
    LoopStmt->VerifyConditionAtEnd = true;
}
