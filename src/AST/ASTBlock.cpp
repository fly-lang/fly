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
ASTBlock::ASTBlock(ASTBlock *Parent, const SourceLocation &Loc) : ASTBlock(Parent, Loc, ASTBlockKind::BLOCK) {

}

ASTBlock::ASTBlock(ASTBlock *Parent, const SourceLocation &Loc, ASTBlockKind Kind) :
    ASTStmt(Parent, Loc, StmtKind::STMT_BLOCK), BlockKind(Kind) {
    if (Parent) {
        Top = Parent->Top;
    }
}

/**
 * Get the ASTTopDef
 * @return
 */
ASTFunction *ASTBlock::getTop() const {
    return Top;
}

ASTBlockKind ASTBlock::getBlockKind() const {
    return BlockKind;
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

const llvm::StringMap<ASTLocalVar *> &ASTBlock::getUndefVars() const {
    return UndefVars;
}

/**
 * Convert to String
 * @return string info for debugging
 */
std::string ASTBlock::str() const {
    return "{ Kind=" + std::to_string((int) Kind) +
            + " }";
}

/**
 * ASTBreak constructor
 * @param Loc
 * @param Parent
 */
ASTBreak::ASTBreak(ASTBlock *Parent, const SourceLocation &Loc) : ASTStmt(Parent, Loc, StmtKind::STMT_BREAK) {

}
/**
 * Convert to String
 * @return string info for debugging
 */
std::string ASTBreak::str() const {
    return "{ Kind=" + std::to_string((int) Kind) + " }";
}

/**
 * ASTContinue constructor
 * @param Loc
 * @param Parent
 */
ASTContinue::ASTContinue(ASTBlock *Parent, const SourceLocation &Loc) : ASTStmt(Parent, Loc, StmtKind::STMT_CONTINUE) {

}

/**
 * Convert to String
 * @return string info for debugging
 */
std::string ASTContinue::str() const {
    return "{ Kind=" + std::to_string((int) Kind) + " }";
}
