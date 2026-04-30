//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTBuilderStmt.cpp - The AST Builder Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBuilderStmt.h"
#include "AST/ASTBuilder.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTReturnStmt.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTExprStmt.h"

#include <AST/ASTDeclStmt.h>

using namespace fly;

ASTBuilderStmt::ASTBuilderStmt() {

}


ASTBuilderStmt *ASTBuilderStmt::CreateReturn(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    ASTBuilderStmt *BuilderStmt = new ASTBuilderStmt();
    BuilderStmt->Stmt = new ASTReturnStmt(Loc);
    // Inner Stmt
    Parent->getContent().push_back(BuilderStmt->Stmt);
    return BuilderStmt;
}

ASTBuilderStmt *ASTBuilderStmt::CreateFail(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    ASTBuilderStmt *BuilderStmt = new ASTBuilderStmt();
    BuilderStmt->Stmt = new ASTFailStmt(Loc);
    // Inner Stmt
    Parent->getContent().push_back(BuilderStmt->Stmt);
    return BuilderStmt;
}

ASTBuilderStmt *ASTBuilderStmt::CreateExpr(ASTBlockStmt *Parent, const SourceLocation &Loc) {
    ASTBuilderStmt *BuilderStmt = new ASTBuilderStmt();
    BuilderStmt->Stmt = new ASTExprStmt(Loc);
    // Inner Stmt
    Parent->getContent().push_back(BuilderStmt->Stmt);
    return BuilderStmt;
}

void ASTBuilderStmt::setExpr(ASTExpr *Expr) {
    // TODO use a super class with expr
    switch (Stmt->getStmtKind()) {
        case ASTStmtKind::STMT_FAIL:
            ((ASTFailStmt *) Stmt)->setFirstExpr(Expr);
            return;
        case ASTStmtKind::STMT_EXPR:
            ((ASTExprStmt *) Stmt)->setExpr(Expr);
            return;
    	case ASTStmtKind::STMT_DECL:
    		((ASTDeclStmt *) Stmt)->setExpr(Expr);
    		return;
    }

    assert(false && "Invalid Stmt Kind");
}
