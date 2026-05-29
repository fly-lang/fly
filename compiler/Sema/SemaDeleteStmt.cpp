//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaDeleteStmt.cpp - delete statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaDeleteStmt.h"
#include "Sema/SemaVisitor.h"
#include "Sema/SemaExpr.h"
#include "Basic/Logger.h"

using namespace fly;

SemaDeleteStmt::SemaDeleteStmt(ASTStmt *AST, SemaExpr *Expr)
    : SemaStmt(SemaKind::STMT_DELETE, AST), Expr(Expr) {}

SemaExpr *SemaDeleteStmt::getExpr() const { return Expr; }

void SemaDeleteStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

std::string SemaDeleteStmt::str() const {
	return Logger("SemaDeleteStmt")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Expr", Expr)
		.End();
}

