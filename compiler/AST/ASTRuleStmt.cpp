//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTRuleStmt.cpp - AST conditional rule statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTRuleStmt.h"
#include "AST/ASTExpr.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTRuleStmt::ASTRuleStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_RULE) {

}

void ASTRuleStmt::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTRuleStmt::ASTRuleStmt(const SourceLocation &Loc, ASTStmtKind Kind) :
		ASTStmt(Loc, Kind) {

}

ASTExpr *ASTRuleStmt::getExpr() const {
    return Expr;
}

ASTStmt *ASTRuleStmt::getStmt() const {
    return Stmt;
}

std::string ASTRuleStmt::str() const {
    return Logger("ASTRuleStmt").
	Attr("Location", getLocation()).
Attr("Kind", static_cast<size_t>(getKind())).
		   Attr("Condition", Expr).
           End();
}
