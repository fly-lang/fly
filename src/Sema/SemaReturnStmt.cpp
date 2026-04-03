//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaReturnStmt.cpp
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaReturnStmt.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaReturnStmt::SemaReturnStmt(ASTStmt *AST, SemaExpr *Expr) : SemaStmt(SemaKind::STMT_RETURN, AST), Expr(Expr) {}

SemaExpr *SemaReturnStmt::getExpr() const { return Expr; }

void SemaReturnStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

