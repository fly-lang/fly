//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTDelete.cpp - AST Delete Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTDeleteStmt.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTIdentifier.h"
#include "Basic/Logger.h"

using namespace fly;

/**
 * ASTBreak constructor
 * @param Loc
 * @param Parent
 */
ASTDeleteStmt::ASTDeleteStmt(const SourceLocation &Loc, ASTExpr *Expr) :
        ASTStmt(Loc, ASTStmtKind::STMT_DELETE), Expr(Expr) {

}

ASTExpr *ASTDeleteStmt::getExpr() {
    return Expr;
}

/**
 * Convert to String
 * @return string info for debugging
 */
std::string ASTDeleteStmt::str() const {
    return Logger("ASTDelete").
	Attr("Location", getLocation()).
		Attr("Kind", static_cast<size_t>(getKind()))
	.Attr("Expr", Expr->str()).End();
}
