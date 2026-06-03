//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaCaseStmt.cpp - suite case statement semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaCaseStmt.h"
#include "Sema/SemaBlockStmt.h"
#include "Sema/SemaVisitor.h"
#include "AST/ASTCaseStmt.h"
#include "Basic/Logger.h"

using namespace fly;

SemaCaseStmt::SemaCaseStmt(ASTCaseStmt *AST, std::string Label, SemaBlockStmt *Body)
    : SemaStmt(SemaKind::STMT_CASE, static_cast<ASTStmt *>(AST)),
      Label(std::move(Label)), Body(Body) {}

const std::string &SemaCaseStmt::getLabel() const { return Label; }

SemaBlockStmt *SemaCaseStmt::getBody() const { return Body; }

void SemaCaseStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

std::string SemaCaseStmt::str() const {
    return Logger("SemaCaseStmt")
        .Attr("Kind", static_cast<uint64_t>(getKind()))
        .Attr("Label", Label)
        .End();
}
