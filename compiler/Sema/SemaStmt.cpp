//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaStmt.cpp - statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaStmt.h"
#include "AST/ASTStmt.h"
#include "Basic/Logger.h"

using namespace fly;

SemaStmt::SemaStmt(SemaKind Kind, ASTStmt *AST) : SemaNode(Kind), AST(AST) {}

ASTStmt *SemaStmt::getAST() const { return AST; }

std::string SemaStmt::str() const {
	return Logger("SemaStmt")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("AST", AST)
		.End();
}

