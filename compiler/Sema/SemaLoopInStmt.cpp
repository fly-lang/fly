//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaLoopInStmt.cpp
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaLoopInStmt.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaLoopInStmt::SemaLoopInStmt(ASTStmt *AST, SemaExpr *Item, SemaExpr *List, SemaStmt *Body)
    : SemaStmt(SemaKind::STMT_LOOP_IN, AST), Item(Item), List(List), Body(Body) {}

SemaExpr *SemaLoopInStmt::getItem() const { return Item; }
SemaExpr *SemaLoopInStmt::getList() const { return List; }
SemaStmt *SemaLoopInStmt::getBody() const { return Body; }

void SemaLoopInStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

