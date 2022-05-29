//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTBlockStmt.cpp - Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTBlock.h"
#include "AST/ASTContext.h"
#include "AST/ASTNode.h"
#include "AST/ASTFunction.h"
#include "AST/ASTFunctionCall.h"
#include "AST/ASTStmt.h"
#include "AST/ASTExpr.h"
#include "AST/ASTLocalVar.h"
#include "Sema/Sema.h"
#include <llvm/ADT/StringMap.h>

using namespace fly;

/**
 * ASTBlock constructor
 * @param Loc
 * @param Top
 * @param Parent
 */
ASTBlock::ASTBlock(const SourceLocation &Loc) : ASTStmt(Loc) {

}

/**
 * Get Kind
 * @return StmtKind
 */
StmtKind ASTBlock::getKind() const {
    return Kind;
}

ASTBlock *ASTBlock::getParent() const {
    return (ASTBlock *) ASTStmt::getParent();
}

/**
 * Get Content
 * @return the Block's content
 */
const std::vector<ASTStmt *> &ASTBlock::getContent() const {
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
    return "{ Kind=" + std::to_string(Kind) +
            ", BlockKind=" + std::to_string(BlockKind) +
            + " }";
}

/**
 * BreakStmt constructor
 * @param Loc
 * @param Parent
 */
BreakStmt::BreakStmt(const SourceLocation &Loc) : ASTStmt(Loc) {

}
/**
 * Convert to String
 * @return string info for debugging
 */
std::string BreakStmt::str() const {
    return "{ Kind=" + std::to_string(Kind) + " }";
}


/**
 * Get the Kind of Stmt
 * @return the StmtKind
 */
StmtKind BreakStmt::getKind() const {
    return Kind;
}

/**
 * ContinueStmt constructor
 * @param Loc
 * @param Parent
 */
ContinueStmt::ContinueStmt(const SourceLocation &Loc) : ASTStmt(Loc) {

}

/**
 * Convert to String
 * @return string info for debugging
 */
std::string ContinueStmt::str() const {
    return "{ Kind=" + std::to_string(Kind) + " }";
}

/**
 * Get the Kind of Stmt
 * @return the StmtKind
 */
StmtKind ContinueStmt::getKind() const {
    return Kind;
}
