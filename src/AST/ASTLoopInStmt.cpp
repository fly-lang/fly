//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTLoopInStmt.cpp - AST Loop In Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTLoopInStmt.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTLoopInStmt::ASTLoopInStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_LOOP_IN) {

}

void ASTLoopInStmt::accept(ASTVisitor& Visitor) {
	Visitor.visit(*this);
}

ASTIdentifier *ASTLoopInStmt::getVarRef() const {
    return VarRef;
}

ASTBlockStmt *ASTLoopInStmt::getBlock() const {
    return Block;
}

std::string ASTLoopInStmt::str() const {
    return Logger("ASTLoopInBlock").
	Attr("Location", getLocation()).
	Attr("Kind", static_cast<size_t>(getKind())).
            End();
}
