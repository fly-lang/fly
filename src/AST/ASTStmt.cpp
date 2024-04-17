//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTStmt.cpp - Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTStmt.h"

using namespace fly;

ASTStmt::ASTStmt(ASTStmt *Parent, const SourceLocation &Loc, ASTStmtKind Kind) :
        ASTBase(Loc), Top(Parent ? Parent->Top : nullptr), Parent(Parent),  Kind(Kind) {
}

ASTStmt *ASTStmt::getParent() const {
    return Parent;
}

ASTStmtKind ASTStmt::getKind() const {
    return Kind;
}

void ASTStmt::setHandleError(bool HE) {
    HandleError = HE;
}

bool ASTStmt::isHandlerError() {
    return HandleError;
}

std::string ASTStmt::str() const {
    return Logger("ASTStmt").
            Super(ASTBase::str()).
            Attr("Kind", (uint64_t) Kind).
            End();
}


