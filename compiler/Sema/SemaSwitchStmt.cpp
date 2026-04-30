//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaSwitchStmt.cpp
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaSwitchStmt.h"
#include "Sema/SemaVisitor.h"

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

