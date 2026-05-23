//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTBreakStmt.cpp - AST break statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBreakStmt.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>

using namespace fly;

/**
 * ASTBreak constructor
 * @param Loc
 * @param Parent
 */
ASTBreakStmt::ASTBreakStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_BREAK) {

}

void ASTBreakStmt::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

/**
 * Convert to String
 * @return string info for debugging
 */
std::string ASTBreakStmt::str() const {
    return Logger("ASTBreak").
	Attr("Location", getLocation()).
			Attr("Kind", static_cast<size_t>(getKind())).
	End();
}
