//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaBreakStmt.cpp - break statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBreakStmt.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaBreakStmt::SemaBreakStmt(ASTStmt *AST) : SemaStmt(SemaKind::STMT_BREAK, AST) {}

void SemaBreakStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

