//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaLoopInStmt.cpp - loop-in statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaLoopInStmt.h"
#include "Sema/SemaVisitor.h"
#include "Sema/SemaExpr.h"
#include "Sema/SemaStmt.h"
#include "Basic/Logger.h"

using namespace fly;

SemaLoopInStmt::SemaLoopInStmt(ASTStmt *AST, SemaExpr *Item, SemaExpr *List, SemaStmt *Body)
    : SemaStmt(SemaKind::STMT_LOOP_IN, AST), Item(Item), List(List), Body(Body) {}

SemaExpr *SemaLoopInStmt::getItem() const { return Item; }
SemaExpr *SemaLoopInStmt::getList() const { return List; }
SemaStmt *SemaLoopInStmt::getBody() const { return Body; }

void SemaLoopInStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

std::string SemaLoopInStmt::str() const {
	return Logger("SemaLoopInStmt")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Item", Item)
		.Attr("List", List)
		.Attr("Body", Body)
		.End();
}

