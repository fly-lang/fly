//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTLoopStmt.cpp - AST While Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTLoopStmt.h"

using namespace fly;

ASTLoopStmt::ASTLoopStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_LOOP) {

}

ASTExpr *ASTLoopStmt::getCondition() {
    return Condition;
}

bool ASTLoopStmt::isVerifyConditionOnEnd() const {
    return VerifyConditionOnEnd;
}

ASTStmt *ASTLoopStmt::getInit() const {
    return Init;
}

ASTStmt *ASTLoopStmt::getLoop() const {
    return Loop;
}

ASTStmt *ASTLoopStmt::getPost() const {
    return Post;
}

std::string ASTLoopStmt::str() const {
    return Logger("ASTLoopStmt").
            Super(ASTStmt::str()).
            End();
}
