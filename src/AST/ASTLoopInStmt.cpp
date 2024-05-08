//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTLoopInStmt.cpp - AST Loop In Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTLoopInStmt.h"

using namespace fly;

ASTLoopInStmt::ASTLoopInStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_LOOP_IN) {

}

ASTVarRef *ASTLoopInStmt::getVarRef() const {
    return VarRef;
}

ASTBlockStmt *ASTLoopInStmt::getBlock() const {
    return Block;
}

std::string ASTLoopInStmt::str() const {
    return Logger("ASTLoopInBlock").
            Super(ASTStmt::str()).
            End();
}
