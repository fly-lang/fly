//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTSwitchStmt.cpp - AST switch statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTSwitchStmt.h"
#include "AST/ASTExpr.h"
#include "AST/ASTStmt.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTSwitchStmt::ASTSwitchStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_SWITCH) {

}

void ASTSwitchStmt::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTExpr *ASTSwitchStmt::getExpr() const {
    return Expr;
}

llvm::SmallVector<ASTRuleStmt *, 8> &ASTSwitchStmt::getCases() {
    return Cases;
}

ASTStmt *ASTSwitchStmt::getDefault() {
    return Default;
}

std::string ASTSwitchStmt::str() const {
    return Logger("ASTSwitchStmt")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("Expr", Expr)
        .Attr("Cases", Cases)
        .Attr("Default", Default)
        .End();
}
