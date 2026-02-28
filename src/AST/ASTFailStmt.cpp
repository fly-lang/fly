//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFailStmt.cpp - AST Fail Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFailStmt.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTFailStmt::ASTFailStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_FAIL) {

}

void ASTFailStmt::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTExpr *ASTFailStmt::getFirstExpr() const {
    return FirstExpr;
}

void ASTFailStmt::setFirstExpr(ASTExpr *E) {
    FirstExpr = E;
}

ASTExpr *ASTFailStmt::getSecondExpr() const {
	return SecondExpr;
}

void ASTFailStmt::setSecondExpr(ASTExpr *E) {
	SecondExpr = E;
}

ASTExpr *ASTFailStmt::getThirdExpr() const {
	return ThirdExpr;
}

void ASTFailStmt::setThirdExpr(ASTExpr *E) {
	ThirdExpr = E;
}

std::string ASTFailStmt::str() const {
    return Logger("ASTFailStmt").
	Attr("Location", getLocation()).
Attr("Kind", static_cast<size_t>(getKind())).
            Attr("FirstExpr", FirstExpr).
	Attr("SecondExpr", SecondExpr).
	Attr("ThirdExpr", ThirdExpr).
            End();
}
