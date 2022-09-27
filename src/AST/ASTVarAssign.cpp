//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTLocalVar.cpp - AST Local Var Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTVarAssign.h"
#include "AST/ASTBlock.h"
#include "AST/ASTVar.h"

using namespace fly;

ASTVarAssign::ASTVarAssign(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *VarRef) :
        ASTExprStmt(Parent, Loc, StmtKind::STMT_VAR_ASSIGN), VarRef(VarRef) {

}

std::string ASTVarAssign::str() const {
    return "VarRef=" + VarRef->str() +
        ", ExprStmt=" + ASTExprStmt::str();
}

ASTVarRef *ASTVarAssign::getVarRef() const {
    return VarRef;
}
