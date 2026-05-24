//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaBreakStmt.cpp - break statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBreakStmt.h"
#include "Sema/SemaVisitor.h"
#include "Basic/Logger.h"

using namespace fly;

SemaBreakStmt::SemaBreakStmt(ASTStmt *AST) : SemaStmt(SemaKind::STMT_BREAK, AST) {}

void SemaBreakStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

std::string SemaBreakStmt::str() const {
	return Logger("SemaBreakStmt")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.End();
}

