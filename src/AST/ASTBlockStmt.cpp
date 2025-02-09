//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTBlockStmt.cpp - AST Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBlockStmt.h"
#include "AST/ASTStmt.h"

#include <llvm/ADT/StringMap.h>

using namespace fly;

/**
 * ASTBlock constructor
 * @param Loc
 * @param Top
 * @param Parent
 */
ASTBlockStmt::ASTBlockStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_BLOCK) {

}

/**
 * Get Content
 * @return the Block's content
 */
const llvm::SmallVector<ASTStmt *, 8>  &ASTBlockStmt::getContent() const {
    return Content;
}

/**
 * Check if Content is empty
 * @return true if empty, or false
 */
bool ASTBlockStmt::isEmpty() const {
    return Content.empty();
}

void ASTBlockStmt::Clear() {
    return Content.clear();
}

/**
 * Get LocalVars
 * @return the Block's declared vars
 */
const llvm::StringMap<ASTVar *> &ASTBlockStmt::getLocalVars() const {
    return LocalVars;
}

/**
 * Convert to String
 * @return string info for debugging
 */
std::string ASTBlockStmt::str() const {
    return Logger("ASTBlock").Super(ASTStmt::str()).End();
}
