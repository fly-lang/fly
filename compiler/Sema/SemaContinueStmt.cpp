//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaContinueStmt.cpp
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaContinueStmt.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaContinueStmt::SemaContinueStmt(ASTStmt *AST) : SemaStmt(SemaKind::STMT_CONTINUE, AST) {}

void SemaContinueStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

