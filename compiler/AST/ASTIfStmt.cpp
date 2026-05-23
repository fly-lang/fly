//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTIfStmt.cpp - AST if statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTIfStmt.h"
#include "AST/ASTExpr.h"
#include "AST/ASTStmt.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTIfStmt::ASTIfStmt(const SourceLocation &Loc) :
        ASTRuleStmt(Loc, ASTStmtKind::STMT_IF) {

}

void ASTIfStmt::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

llvm::SmallVector<ASTRuleStmt *, 8> ASTIfStmt::getElsif() {
    return Elsif;
}

ASTStmt *ASTIfStmt::getElse() {
    return Else;
}

std::string ASTIfStmt::str() const {
    return Logger("ASTIfStmt")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("Condition", getExpr())
        .Attr("Elsif", Elsif)
        .Attr("Else", Else)
        .End();
}
