//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTTestStmt.cpp - AST test block statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTTestStmt.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTVisitor.h"
#include "Basic/Logger.h"

using namespace fly;

ASTTestStmt::ASTTestStmt(const SourceLocation &Loc) :
        ASTStmt(Loc, ASTStmtKind::STMT_TEST) {
}

void ASTTestStmt::accept(ASTVisitor &Visitor) {
    Visitor.visit(*this);
}

ASTBlockStmt *ASTTestStmt::getBody() const {
    return Body;
}

std::string ASTTestStmt::str() const {
    return Logger("ASTTestStmt")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("Body", Body)
        .End();
}
