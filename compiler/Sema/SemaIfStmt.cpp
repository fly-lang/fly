//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaIfStmt.cpp - if statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaIfStmt.h"
#include "Sema/SemaVisitor.h"
#include "Sema/SemaExpr.h"
#include "Sema/SemaStmt.h"
#include "Basic/Logger.h"

using namespace fly;

SemaIfStmt::SemaIfStmt(ASTStmt *AST, SemaExpr *Cond, SemaStmt *Then)
    : SemaStmt(SemaKind::STMT_IF, AST), Cond(Cond), Then(Then) {}

SemaExpr *SemaIfStmt::getCond() const { return Cond; }
SemaStmt *SemaIfStmt::getThen() const { return Then; }

void SemaIfStmt::addElsif(SemaExpr *Expr, SemaStmt *Stmt) {
    Elsif.push_back({Expr, Stmt});
}

const llvm::SmallVector<SemaRuleStmt, 4> &SemaIfStmt::getElsif() const { return Elsif; }

SemaStmt *SemaIfStmt::getElse() const { return Else; }
void SemaIfStmt::setElse(SemaStmt *E) { Else = E; }

void SemaIfStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

std::string SemaIfStmt::str() const {
	return Logger("SemaIfStmt")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Cond", Cond)
		.Attr("Then", Then)
		.Attr("Else", Else)
		.End();
}

