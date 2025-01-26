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
        ASTRuleStmt(Loc, ASTStmtKind::STMT_IF) {

}

llvm::SmallVector<ASTRuleStmt *, 8> ASTIfStmt::getElsif() {
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
