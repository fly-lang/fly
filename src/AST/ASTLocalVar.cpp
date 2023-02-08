//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTLocalVar.cpp - AST Local Var Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTLocalVar.h"
#include "AST/ASTType.h"
#include "AST/ASTBlock.h"

using namespace fly;

ASTLocalVar::ASTLocalVar(ASTBlock *Parent, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, bool Constant) :
        ASTExprStmt(Parent, Loc, ASTStmtKind::STMT_VAR_DEFINE), VarKind(ASTVarKind::VAR_LOCAL),
        Type(Type), Name(Name), Constant(Constant) {

}

ASTVarKind ASTLocalVar::getVarKind() {
    return VarKind;
}

ASTType *ASTLocalVar::getType() const {
    return Type;
}

llvm::StringRef ASTLocalVar::getName() const {
    return Name;
}

bool ASTLocalVar::isConstant() const {
    return Constant;
}

ASTExpr *ASTLocalVar::getExpr() const {
    return Expr;
}

CodeGenVar *ASTLocalVar::getCodeGen() const {
    return CodeGen;
}

void ASTLocalVar::setCodeGen(CodeGenVar *CG) {
    CodeGen = CG;
}

std::string ASTLocalVar::str() const {
    return Logger("ASTLocalVar").
            Super(ASTExprStmt::str()).
            Attr("Type", Type).
            Attr("Name", Name).
            Attr("VarKind", (uint64_t) VarKind).
            Attr("Constant", Constant).
            End();
}
