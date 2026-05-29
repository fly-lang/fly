//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaDeclStmt.cpp - declaration statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaDeclStmt.h"
#include "Sema/SemaVisitor.h"
#include "Sema/SemaLocalVar.h"
#include "Sema/SemaExpr.h"
#include "Basic/Logger.h"

using namespace fly;

SemaDeclStmt::SemaDeclStmt(ASTStmt *AST, SemaLocalVar *Var, SemaExpr *Expr)
    : SemaStmt(SemaKind::STMT_DECL, AST), Var(Var), Expr(Expr) {}

SemaLocalVar *SemaDeclStmt::getVar()  const { return Var; }
SemaExpr     *SemaDeclStmt::getExpr() const { return Expr; }

void SemaDeclStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

std::string SemaDeclStmt::str() const {
	return Logger("SemaDeclStmt")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Var", Var)
		.Attr("Expr", Expr)
		.End();
}

