//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTSwitchBlock.cpp - AST Switch Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTSwitchStmt.h"
#include "Basic/Logger.h"

using namespace fly;

ASTSwitchStmt::ASTSwitchStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_SWITCH) {

}

ASTIdentifier *ASTSwitchStmt::getVar() const {
    return Var;
}

llvm::SmallVector<ASTRuleStmt *, 8> &ASTSwitchStmt::getCases() {
    return Cases;
}

ASTStmt *ASTSwitchStmt::getDefault() {
    return Default;
}

std::string ASTSwitchStmt::str() const {
    return Logger("ASTSwitchStmt").
	Attr("Location", getLocation()).
Attr("Kind", static_cast<size_t>(getKind())).
            Attr("VarRef", Var).
            End();
}
