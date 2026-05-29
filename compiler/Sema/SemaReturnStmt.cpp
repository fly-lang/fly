//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaReturnStmt.cpp - return statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaReturnStmt.h"
#include "Sema/SemaVisitor.h"
#include "Sema/SemaExpr.h"
#include "Basic/Logger.h"

using namespace fly;

SemaReturnStmt::SemaReturnStmt(ASTStmt *AST, SemaExpr *Expr) : SemaStmt(SemaKind::STMT_RETURN, AST), Expr(Expr) {}

SemaExpr *SemaReturnStmt::getExpr() const { return Expr; }

void SemaReturnStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

std::string SemaReturnStmt::str() const {
	return Logger("SemaReturnStmt")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Expr", Expr)
		.End();
}

