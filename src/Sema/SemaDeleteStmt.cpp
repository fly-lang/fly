//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaDeleteStmt.cpp
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaDeleteStmt.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaDeleteStmt::SemaDeleteStmt(ASTStmt *AST, SemaExpr *Expr)
    : SemaStmt(SemaKind::STMT_DELETE, AST), Expr(Expr) {}

SemaExpr *SemaDeleteStmt::getExpr() const { return Expr; }

void SemaDeleteStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

