//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaContinueStmt.cpp - continue statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaContinueStmt.h"
#include "Sema/SemaVisitor.h"
#include "Basic/Logger.h"

using namespace fly;

SemaContinueStmt::SemaContinueStmt(ASTStmt *AST) : SemaStmt(SemaKind::STMT_CONTINUE, AST) {}

void SemaContinueStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

std::string SemaContinueStmt::str() const {
	return Logger("SemaContinueStmt")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.End();
}

