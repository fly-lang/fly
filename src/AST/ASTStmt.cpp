//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTStmt.cpp - AST Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTStmt.h"

using namespace fly;

ASTStmt::ASTStmt(const SourceLocation &Loc, ASTStmtKind Kind) :
        ASTBase(Loc, ASTKind::AST_STMT), StmtKind(Kind) {
}

ASTStmt *ASTStmt::getParent() const {
    return Parent;
}

ASTFunction *ASTStmt::getFunction() const {
    return Function;
}

ASTStmtKind ASTStmt::getStmtKind() const {
    return StmtKind;
}

std::string ASTStmt::str() const {
    return Logger("ASTStmt").
            Super(ASTBase::str()).
            Attr("Kind", static_cast<uint64_t>(StmtKind)).
            End();
}


