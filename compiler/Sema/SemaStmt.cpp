//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaStmt.cpp - statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaStmt.h"

using namespace fly;

SemaStmt::SemaStmt(SemaKind Kind, ASTStmt *AST) : SemaNode(Kind), AST(AST) {}

ASTStmt *SemaStmt::getAST() const { return AST; }

