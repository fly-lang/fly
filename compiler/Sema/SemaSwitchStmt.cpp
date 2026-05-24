//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaSwitchStmt.cpp - switch statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaSwitchStmt.h"
#include "Sema/SemaVisitor.h"
#include "Sema/SemaExpr.h"
#include "Sema/SemaStmt.h"
#include "Basic/Logger.h"

using namespace fly;

SemaSwitchStmt::SemaSwitchStmt(ASTStmt *AST, SemaExpr *Expr)
    : SemaStmt(SemaKind::STMT_SWITCH, AST), Expr(Expr) {}

SemaExpr *SemaSwitchStmt::getExpr() const { return Expr; }

void SemaSwitchStmt::addCase(SemaExpr *CaseExpr, SemaStmt *CaseStmt) {
    Cases.push_back({CaseExpr, CaseStmt});
}

const llvm::SmallVector<SemaRuleStmt, 8> &SemaSwitchStmt::getCases() const { return Cases; }

SemaStmt *SemaSwitchStmt::getDefault() const { return Default; }
void SemaSwitchStmt::setDefault(SemaStmt *D) { Default = D; }

void SemaSwitchStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

std::string SemaSwitchStmt::str() const {
	return Logger("SemaSwitchStmt")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Expr", Expr)
		.Attr("Default", Default)
		.End();
}

