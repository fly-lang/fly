//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaFailStmt.cpp
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaFailStmt.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaFailStmt::SemaFailStmt(ASTStmt *AST) : SemaStmt(SemaKind::STMT_FAIL, AST) {}

SemaExpr *SemaFailStmt::getFirst()  const { return First; }
SemaExpr *SemaFailStmt::getSecond() const { return Second; }
SemaExpr *SemaFailStmt::getThird()  const { return Third; }

void SemaFailStmt::setFirst(SemaExpr *E)  { First  = E; }
void SemaFailStmt::setSecond(SemaExpr *E) { Second = E; }
void SemaFailStmt::setThird(SemaExpr *E)  { Third  = E; }

void SemaFailStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

