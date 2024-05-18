//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTSwitchBlock.cpp - AST Switch Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTSwitchStmt.h"

using namespace fly;

ASTSwitchStmt::ASTSwitchStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_SWITCH) {

}

ASTVarRef *ASTSwitchStmt::getVarRef() const {
    return VarRef;
}

llvm::SmallVector<ASTSwitchCase *, 8> &ASTSwitchStmt::getCases() {
    return Cases;
}

ASTBlockStmt *ASTSwitchStmt::getDefault() {
    return Default;
}

std::string ASTSwitchStmt::str() const {
    return Logger("ASTSwitchStmt").
            Super(ASTStmt::str()).
            Attr("VarRef", VarRef).
            End();
}

ASTSwitchCase::ASTSwitchCase(const SourceLocation &Loc) {
}

ASTValueExpr *ASTSwitchCase::getValueExpr() {
    return Value;
}

ASTType *ASTSwitchCase::getType() const {
    return Type;
}

ASTBlockStmt *ASTSwitchCase::getBlock() const {
    return Block;
}

std::string ASTSwitchCase::str() const {
    return Logger("ASTSwitchCase").
            Attr("Value", Value).
            Attr("Type", Type).
            End();
}
