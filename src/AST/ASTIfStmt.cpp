//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTIfBlock.cpp - AST If Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTIfStmt.h"

using namespace fly;

ASTIfStmt::ASTIfStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_IF) {

}

ASTExpr *ASTIfStmt::getCondition() {
    return Condition;
}

ASTStmt *ASTIfStmt::getStmt() const {
    return Stmt;
}

llvm::SmallVector<ASTElsif *, 8> ASTIfStmt::getElsif() {
    return Elsif;
}

ASTStmt *ASTIfStmt::getElse() {
    return Else;
}

std::string ASTIfStmt::str() const {
    return Logger("ASTIfStmt").
           Super(ASTStmt::str()).
           End();
}

ASTElsif::ASTElsif(const SourceLocation &Loc) {
}

ASTExpr *ASTElsif::getCondition() {
    return Condition;
}

ASTStmt *ASTElsif::getStmt() const {
    return Stmt;
}

std::string ASTElsif::str() const {
    return Logger("ASTElsif").
            Attr("Condition", Condition).
            End();
}
