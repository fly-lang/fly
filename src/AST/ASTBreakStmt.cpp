//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTBreakStmt.cpp - AST Break Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBreakStmt.h"

using namespace fly;

/**
 * ASTBreak constructor
 * @param Loc
 * @param Parent
 */
ASTBreakStmt::ASTBreakStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_BREAK) {

}
/**
 * Convert to String
 * @return string info for debugging
 */
std::string ASTBreakStmt::str() const {
    return Logger("ASTBreak").Super(ASTStmt::str()).End();
}
