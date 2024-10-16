//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFailStmt.cpp - AST Fail Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFailStmt.h"

using namespace fly;

ASTFailStmt::ASTFailStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_FAIL) {

}

ASTVar *ASTFailStmt::getErrorHandler() const {
    return ErrorHandler;
}

ASTExpr *ASTFailStmt::getExpr() const {
    return Expr;
}

void ASTFailStmt::setExpr(fly::ASTExpr *E) {
    Expr = E;
}


bool ASTFailStmt::hasHandle() {
    return HandleStmt != nullptr;
}

ASTHandleStmt *ASTFailStmt::getHandle() {
    return HandleStmt;
}

std::string ASTFailStmt::str() const {
    return Logger("ASTFailStmt").
            Super(ASTStmt::str()).
            Attr("Expr", Expr).
            End();
}
