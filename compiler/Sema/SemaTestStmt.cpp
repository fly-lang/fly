//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaTestStmt.cpp - test block statement semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaTestStmt.h"
#include "Sema/SemaBlockStmt.h"
#include "Sema/SemaVisitor.h"
#include "AST/ASTTestStmt.h"
#include "Basic/Logger.h"

using namespace fly;

SemaTestStmt::SemaTestStmt(ASTTestStmt *AST)
    : SemaStmt(SemaKind::STMT_TEST, static_cast<ASTStmt *>(AST)) {}

SemaBlockStmt *SemaTestStmt::getBody() const { return Body; }

void SemaTestStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

std::string SemaTestStmt::str() const {
    return Logger("SemaTestStmt")
        .Attr("Kind", static_cast<uint64_t>(getKind()))
        .End();
}
