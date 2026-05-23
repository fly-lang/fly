//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaDeclStmt.cpp - declaration statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaDeclStmt.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaDeclStmt::SemaDeclStmt(ASTStmt *AST, SemaLocalVar *Var, SemaExpr *Expr)
    : SemaStmt(SemaKind::STMT_DECL, AST), Var(Var), Expr(Expr) {}

SemaLocalVar *SemaDeclStmt::getVar()  const { return Var; }
SemaExpr     *SemaDeclStmt::getExpr() const { return Expr; }

void SemaDeclStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

