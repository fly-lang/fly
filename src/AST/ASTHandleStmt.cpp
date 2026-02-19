//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTHandleStmt.cpp - AST Handle Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTHandleStmt.h"
#include "AST/ASTIdentifier.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTHandleStmt::ASTHandleStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_HANDLE) {

}

void ASTHandleStmt::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTBlockStmt* ASTHandleStmt::getHandle() const {
    return Handle;
}

std::string ASTHandleStmt::str() const {
    return Logger("ASTHandleBlock").
	Attr("Location", getLocation()).
Attr("Kind", static_cast<size_t>(getKind())).
            End();
}
