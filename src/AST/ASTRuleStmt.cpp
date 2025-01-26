//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTRuleStmt.cpp - AST Rule Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTRuleStmt.h"

using namespace fly;

ASTRuleStmt::ASTRuleStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_RULE) {

}

ASTRuleStmt::ASTRuleStmt(const SourceLocation &Loc, ASTStmtKind Kind) :
		ASTStmt(Loc, Kind) {

}

ASTExpr *ASTRuleStmt::getRule() {
    return Rule;
}

ASTStmt *ASTRuleStmt::getStmt() const {
    return Stmt;
}

std::string ASTRuleStmt::str() const {
    return Logger("ASTRuleStmt").
           Super(ASTStmt::str()).
		   Attr("Condition", Rule).
           End();
}
