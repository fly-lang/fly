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
#include "AST/ASTVarAssign.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTForBlock.h"
#include "Sema/Sema.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"
#include <llvm/ADT/StringMap.h>

using namespace fly;

/**
 * ASTBlock constructor
 * @param Loc
 * @param Top
 * @param Parent
 */
ASTBlock::ASTBlock(const SourceLocation &Loc, ASTFunction *Top, ASTBlock *Parent) :
    ASTStmt(Loc, Top, Parent) {
    FLY_DEBUG("ASTBlock", "ASTBlock");
}

/**
 * ASTBlock constructor
 * @param Loc
 * @param Parent
 */
ASTBlock::ASTBlock(const SourceLocation &Loc, ASTBlock *Parent) : ASTStmt(Loc, Parent->getTop(), Parent) {
        FLY_DEBUG("ASTBlock", "~ASTBlock");
}

/**
 * Get Kind
 * @return StmtKind
 */
StmtKind ASTBlock::getKind() const {
    return Kind;
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
BreakStmt::BreakStmt(const SourceLocation &Loc, ASTBlock *Parent) : ASTStmt(Loc, Parent) {

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
ContinueStmt::ContinueStmt(const SourceLocation &Loc, ASTBlock *Parent) : ASTStmt(Loc, Parent) {

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
