//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTLocalVar.cpp - AST Local Var Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTVarDefine.h"
#include "AST/ASTBlock.h"
#include "AST/ASTVar.h"
#include "AST/ASTVarRef.h"

using namespace fly;

ASTVarDefine::ASTVarDefine(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *VarRef) :
        ASTStmt(Parent, Loc, ASTStmtKind::STMT_VAR_DEFINE), VarRef(VarRef) {

}

std::string ASTVarDefine::str() const {
    return Logger("ASTVarAssign").
            Super(ASTStmt::str()).
            Attr("VarRef", VarRef).
            Attr("ExprStmt", ASTStmt::str()).
            End();
}

ASTVarRef *ASTVarDefine::getVarRef() const {
    return VarRef;
}

ASTExpr *ASTVarDefine::getExpr() const {
    return Expr;
}

void ASTVarDefine::setExpr(fly::ASTExpr *E) {
    Expr = E;
}

ASTBlock *ASTVarDefine::getBlock() const {
    return Block;
}
