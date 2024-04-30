//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTBlock.cpp - Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBlock.h"
#include "AST/ASTStmt.h"

#include <llvm/ADT/StringMap.h>

using namespace fly;

/**
 * ASTBlock constructor
 * @param Loc
 * @param Top
 * @param Parent
 */
ASTBlock::ASTBlock(const SourceLocation &Loc) :
        ASTBlock(Loc, ASTBlockKind::BLOCK) {

}

ASTBlock::ASTBlock(const SourceLocation &Loc, ASTBlockKind Kind) :
        ASTStmt(Loc, ASTStmtKind::STMT_BLOCK), BlockKind(Kind) {
}

ASTBlockKind ASTBlock::getBlockKind() const {
    return BlockKind;
}

/**
 * Get Content
 * @return the Block's content
 */
const llvm::SmallVector<ASTStmt *, 8>  &ASTBlock::getContent() const {
    return Content;
}

/**
 * Check if Content is empty
 * @return true if empty, or false
 */
bool ASTBlock::isEmpty() const {
    return Content.empty();
}

void ASTBlock::Clear() {
    return Content.clear();
}

/**
 * Get LocalVars
 * @return the Block's declared vars
 */
const llvm::StringMap<ASTLocalVar *> &ASTBlock::getLocalVars() const {
    return LocalVars;
}

/**
 * Convert to String
 * @return string info for debugging
 */
std::string ASTBlock::str() const {
    return Logger("ASTBlock").Super(ASTStmt::str()).Attr("Kind", (uint64_t) BlockKind).End();
}

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
