//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTVarStmt.cpp - AST Var Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTAssignStmt.h"
#include "Basic/Logger.h"

using namespace fly;

ASTAssignStmt::ASTAssignStmt(const SourceLocation &Loc, ASTExpr *Source, ASTAssignOperatorKind AssignOperatorKind) :
        ASTStmt(Loc, ASTStmtKind::STMT_ASSIGN), Source(Source), Kind(AssignOperatorKind) {

}

ASTExpr *ASTAssignStmt::getSource() const {
    return Source;
}

ASTAssignOperatorKind ASTAssignStmt::getOpKind() const {
    return Kind;
}

ASTExpr *ASTAssignStmt::getTarget() const {
    return Target;
}

void ASTAssignStmt::setExpr(fly::ASTExpr *E) {
    Target = E;
}

std::string ASTAssignStmt::str() const {
    return Logger("ASTVarAssign").
	Attr("Location", getLocation()).
Attr("Kind", static_cast<size_t>(getKind())).
            Attr("VarRef", Source).
            Attr("Kind", (uint64_t) Kind).
            Attr("ExprStmt", ASTStmt::str()).
            End();
}
