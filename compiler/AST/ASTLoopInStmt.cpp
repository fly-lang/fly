//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTLoopInStmt.cpp - AST Loop In Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTLoopInStmt.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTLoopInStmt::ASTLoopInStmt(const SourceLocation &Loc, ASTExpr *Item, ASTExpr *List, ASTStmt *Stmt) :
        ASTStmt(Loc, ASTStmtKind::STMT_LOOP_IN), Item(Item), List(List), Stmt(Stmt) {

}

void ASTLoopInStmt::accept(ASTVisitor &Visitor) {
    Visitor.visit(*this);
}

ASTExpr *ASTLoopInStmt::getItem() const {
    return Item;
}

ASTExpr *ASTLoopInStmt::getList() const {
    return List;
}

ASTStmt *ASTLoopInStmt::getStmt() const {
    return Stmt;
}

std::string ASTLoopInStmt::str() const {
    return Logger("ASTLoopInStmt").
            Attr("Location", getLocation()).
            Attr("Item", Item).
            Attr("List", List).
            Attr("Stmt", Stmt).
            End();
}
