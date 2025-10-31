//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBuilderLoopStmt.cpp - The Sema Builder Loop Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBuilderLoopStmt.h"
#include "AST/ASTBuilder.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTLoopStmt.h"

using namespace fly;

SemaBuilderLoopStmt::SemaBuilderLoopStmt(ASTBlockStmt *Parent) : Parent(Parent) {

}

SemaBuilderLoopStmt *SemaBuilderLoopStmt::Create(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    SemaBuilderLoopStmt *Builder = new SemaBuilderLoopStmt(Parent);
    Builder->LoopStmt = new ASTLoopStmt(Loc);

    // Inner Stmt
    Parent->Content.push_back(Builder->LoopStmt);
    Builder->LoopStmt->Parent = Parent;
    Builder->LoopStmt->Function = Parent->Function;
    return Builder;
}

SemaBuilderLoopStmt *SemaBuilderLoopStmt::Loop(ASTExpr *Expr, ASTBlockStmt *Stmt) {
    LoopStmt->Rule = Expr;
    LoopStmt->Stmt = Stmt;

    // Inner Stmt
    Stmt->Parent = Parent;
    Stmt->Function = Parent->Function;
    return this;
}

void SemaBuilderLoopStmt::Init(ASTBlockStmt *Stmt) {
    LoopStmt->Init = Stmt;

    // Inner Stmt
    Stmt->Parent = LoopStmt;
    Stmt->Function = LoopStmt->Function;
}

void SemaBuilderLoopStmt::Post(ASTBlockStmt *Stmt) {
    LoopStmt->Post = Stmt;

    // Inner Stmt
    Stmt->Parent = LoopStmt->Init;
    Stmt->Function = LoopStmt->Function;
}

void SemaBuilderLoopStmt::VerifyConditionAtEnd() {
    LoopStmt->VerifyConditionAtEnd = true;
}
