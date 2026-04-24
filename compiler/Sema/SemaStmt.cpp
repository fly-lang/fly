//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaStmt.cpp - Sema Statement base implementation
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaStmt.h"

using namespace fly;

SemaStmt::SemaStmt(SemaKind Kind, ASTStmt *AST) : SemaNode(Kind), AST(AST) {}

ASTStmt *SemaStmt::getAST() const { return AST; }

