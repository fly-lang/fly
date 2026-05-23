//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaExprStmt.cpp - expression statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaExprStmt.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaExprStmt::SemaExprStmt(ASTStmt *AST, SemaExpr *Expr)
    : SemaStmt(SemaKind::STMT_EXPR, AST), Expr(Expr) {}

SemaExpr *SemaExprStmt::getExpr() const { return Expr; }

void SemaExprStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

