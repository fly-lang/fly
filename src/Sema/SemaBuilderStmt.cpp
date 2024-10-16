//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBuilderStmt.cpp - The Sema Builder Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBuilderStmt.h"
#include "Sema/SemaBuilder.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTVarStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTReturnStmt.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTExprStmt.h"

using namespace fly;

SemaBuilderStmt::SemaBuilderStmt(SemaBuilder *Builder) : Builder(Builder) {

}

SemaBuilderStmt *SemaBuilderStmt::CreateVar(SemaBuilder *Builder, ASTBlockStmt *Parent, ASTVarRef *VarRef) {
    SemaBuilderStmt *BuilderStmt = new SemaBuilderStmt(Builder);
    BuilderStmt->Stmt = new ASTVarStmt(VarRef->getLocation(), VarRef);

    // Inner Stmt
    Parent->Content.push_back(BuilderStmt->Stmt);
    BuilderStmt->Stmt->Parent = Parent;
    BuilderStmt->Stmt->Function = Parent->Function;
    return BuilderStmt;
}

SemaBuilderStmt *SemaBuilderStmt::CreateVar(SemaBuilder *Builder, ASTBlockStmt *Parent, ASTVar *Var) {
    SemaBuilderStmt *BuilderStmt = new SemaBuilderStmt(Builder);
    ASTVarRef *VarRef = Builder->CreateVarRef(Var);
    BuilderStmt->Stmt = new ASTVarStmt(Var->getLocation(), VarRef);

    // Inner Stmt
    Parent->Content.push_back(BuilderStmt->Stmt);
    BuilderStmt->Stmt->Parent = Parent;
    BuilderStmt->Stmt->Function = Parent->Function;
    return BuilderStmt;
}

SemaBuilderStmt *SemaBuilderStmt::CreateReturn(SemaBuilder *Builder, ASTBlockStmt *Parent, const SourceLocation &Loc) {
    SemaBuilderStmt *BuilderStmt = new SemaBuilderStmt(Builder);
    BuilderStmt->Stmt = new ASTReturnStmt(Loc);
    // Inner Stmt
    Parent->Content.push_back(BuilderStmt->Stmt);
    BuilderStmt->Stmt->Parent = Parent;
    BuilderStmt->Stmt->Function = Parent->Function;
    return BuilderStmt;
}

SemaBuilderStmt *SemaBuilderStmt::CreateFail(SemaBuilder *Builder, ASTBlockStmt *Parent, const SourceLocation &Loc) {
    SemaBuilderStmt *BuilderStmt = new SemaBuilderStmt(Builder);
    BuilderStmt->Stmt = new ASTFailStmt(Loc);
    // Inner Stmt
    Parent->Content.push_back(BuilderStmt->Stmt);
    BuilderStmt->Stmt->Parent = Parent;
    BuilderStmt->Stmt->Function = Parent->Function;
    return BuilderStmt;
}

SemaBuilderStmt *SemaBuilderStmt::CreateExpr(SemaBuilder *Builder, ASTBlockStmt *Parent, const SourceLocation &Loc) {
    SemaBuilderStmt *BuilderStmt = new SemaBuilderStmt(Builder);
    BuilderStmt->Stmt = new ASTExprStmt(Loc);
    // Inner Stmt
    Parent->Content.push_back(BuilderStmt->Stmt);
    BuilderStmt->Stmt->Parent = Parent;
    BuilderStmt->Stmt->Function = Parent->Function;
    return BuilderStmt;
}

void SemaBuilderStmt::setExpr(ASTExpr *Expr) {
    // TODO use a super class with expr
    switch (Stmt->getKind()) {
        case ASTStmtKind::STMT_VAR:
            ((ASTVarStmt *) Stmt)->Expr = Expr;
            return;
        case ASTStmtKind::STMT_RETURN:
            ((ASTReturnStmt *) Stmt)->Expr = Expr;
            return;
        case ASTStmtKind::STMT_FAIL:
            ((ASTFailStmt *) Stmt)->Expr = Expr;
            return;
        case ASTStmtKind::STMT_EXPR:
            ((ASTExprStmt *) Stmt)->Expr = Expr;
            return;
    }

    assert(false && "Invalid Stmt Kind");
}
