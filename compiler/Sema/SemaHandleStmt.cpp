//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaHandleStmt.cpp - handle statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaHandleStmt.h"
#include "Sema/SemaVisitor.h"
#include "Sema/SemaBlockStmt.h"
#include "Basic/Logger.h"

using namespace fly;

SemaHandleStmt::SemaHandleStmt(ASTStmt *AST) : SemaStmt(SemaKind::STMT_HANDLE, AST) {}

SemaError      *SemaHandleStmt::getErrorHandler() const { return ErrorHandler; }
void            SemaHandleStmt::setErrorHandler(SemaError *E) { ErrorHandler = E; }

SemaBlockStmt  *SemaHandleStmt::getHandle() const { return Handle; }
void            SemaHandleStmt::setHandle(SemaBlockStmt *H) { Handle = H; }

void SemaHandleStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

std::string SemaHandleStmt::str() const {
	return Logger("SemaHandleStmt")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Handle", Handle)
		.End();
}

