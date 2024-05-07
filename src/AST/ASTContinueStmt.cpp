//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTContinueStmt.cpp - AST Continue Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTContinueStmt.h"

using namespace fly;

/**
 * ASTContinue constructor
 * @param Loc
 * @param Parent
 */
ASTContinueStmt::ASTContinueStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_CONTINUE) {

}

/**
 * Convert to String
 * @return string info for debugging
 */
std::string ASTContinueStmt::str() const {
    return Logger("ASTContinue").Super(ASTStmt::str()).End();
}
