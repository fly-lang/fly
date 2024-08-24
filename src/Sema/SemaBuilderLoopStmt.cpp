//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBuilderLoopStmt.cpp - The Sema Builder Loop Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBuilderLoopStmt.h"
#include "Sema/SemaBuilder.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTLoopStmt.h"

using namespace fly;

SemaBuilderLoopStmt::SemaBuilderLoopStmt(Sema &S, ASTStmt *Parent) : S(S), Parent(Parent) {

}

SemaBuilderLoopStmt *SemaBuilderLoopStmt::Create(Sema &S, ASTStmt *Parent) {
    return new SemaBuilderLoopStmt(S, Parent);
}

SemaBuilderLoopStmt *SemaBuilderLoopStmt::Loop(const SourceLocation &Loc, ASTExpr *Expr, ASTStmt *Stmt) {
    LoopStmt = new ASTLoopStmt(Loc);
    LoopStmt->Condition = Expr;
    // Inner Stmt
    LoopStmt->Parent = Parent;
    LoopStmt->Function = Parent->Function;
    // Inner Stmt
    Stmt->Parent = Parent;
    Stmt->Function = Parent->Function;
    return this;
}

void SemaBuilderLoopStmt::Init(ASTStmt *Stmt) {
    LoopStmt->Init = Stmt;
    // Inner Stmt
    Stmt->Parent = LoopStmt;
    Stmt->Function = LoopStmt->Function;
}

void SemaBuilderLoopStmt::Post(ASTStmt *Stmt) {
    LoopStmt->Post = Stmt;
    // Inner Stmt
    Stmt->Parent = LoopStmt->Init;
    Stmt->Function = LoopStmt->Function;
}

void SemaBuilderLoopStmt::VerifyConditionAtEnd() {
    LoopStmt->VerifyConditionAtEnd = true;
}
