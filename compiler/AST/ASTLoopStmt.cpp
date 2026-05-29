//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTLoopStmt.cpp - AST loop statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTLoopStmt.h"
#include "AST/ASTExpr.h"
#include "AST/ASTStmt.h"

#include "AST/ASTVisitor.h"
#include "Basic/Logger.h"

using namespace fly;

ASTLoopStmt::ASTLoopStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_LOOP) {

}

void ASTLoopStmt::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

bool ASTLoopStmt::hasVerifyConditionAtEnd() const {
    return VerifyConditionAtEnd;
}

ASTExpr *ASTLoopStmt::getExpr() const {
	return Expr;
}

llvm::SmallVector<ASTStmt *, 4> &ASTLoopStmt::getInit() {
    return Init;
}

ASTStmt *ASTLoopStmt::getLoop() const {
    return Loop;
}

llvm::SmallVector<ASTStmt *, 4> &ASTLoopStmt::getPost() {
    return Post;
}

std::string ASTLoopStmt::str() const {
    return Logger("ASTLoopStmt")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("VerifyConditionAtEnd", VerifyConditionAtEnd)
        .Attr("Expr", Expr)
        .Attr("Init", Init)
        .Attr("Post", Post)
        .Attr("Loop", Loop)
        .End();
}
