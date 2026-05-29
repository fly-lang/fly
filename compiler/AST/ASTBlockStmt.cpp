//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTBlockStmt.cpp - AST block statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBlockStmt.h"
#include "AST/ASTStmt.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>
#include <llvm/ADT/StringMap.h>
#include <AST/ASTLocalVar.h>

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

void ASTBlockStmt::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

/**
 * Get Content
 * @return the Block's content
 */
llvm::SmallVector<ASTStmt *, 8> &ASTBlockStmt::getContent() {
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
 * Add a statement to the block and set its parent/function
 */
void ASTBlockStmt::addContent(ASTStmt *Stmt) {
    if (!Stmt)
        return;
    Content.push_back(Stmt);
    Stmt->setParent(this);
}

/**
 * Convert to String
 * @return string info for debugging
 */
std::string ASTBlockStmt::str() const {
    return Logger("ASTBlockStmt")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("Content", Content)
        .End();
}

ASTBlockStmt::~ASTBlockStmt() {
    for (auto *S : Content) delete S;
    Content.clear();
}
