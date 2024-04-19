//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTHandleBlock.cpp - If Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTHandleStmt.h"

using namespace fly;

ASTHandleStmt::ASTHandleStmt(ASTStmt *Parent, const SourceLocation &Loc) :
        ASTStmt(Parent, Loc, ASTStmtKind::STMT_HANDLE) {

}

void ASTHandleStmt::setErrorRef(ASTVarRef *errorRef) {
    ErrorRef = errorRef;
}

ASTVarRef *ASTHandleStmt::getErrorRef() const {
    return ErrorRef;
}

ASTStmt *ASTHandleStmt::getHandle() const {
    return Handle;
}

void ASTHandleStmt::setHandle(ASTStmt *H) {
    Handle = H;
}

std::string ASTHandleStmt::str() const {
    return Logger("ASTHandleBlock").
            Super(ASTStmt::str()).
            Attr("ErrorRef", ErrorRef).
            Attr("Handle", Handle).
            End();
}
