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

ASTVarDefine::ASTVarDefine(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *VarRef, bool Init) :
        ASTStmt(Parent, Loc, ASTStmtKind::STMT_VAR_DEFINE), VarRef(VarRef), FirstDefined(Init) {

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

ASTBlock *ASTVarDefine::getBlock() const {
    return Block;
}

bool ASTVarDefine::isFirstDefined() const {
    return FirstDefined;
}
