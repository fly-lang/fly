//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTLocalVar.cpp - AST Local Var Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTValue.h"
#include "AST/ASTLocalVar.h"

using namespace fly;

ASTLocalVar::ASTLocalVar(const SourceLocation &Loc, ASTBlock *Block, ASTType *Type, const std::string &Name) :
        ASTExprStmt(Loc, Block), ASTVar(Type, Name) {
    if (Type->getKind() == TYPE_ARRAY) {
        setExpr(new ASTValueExpr(new ASTArrayValue(Loc, ((ASTArrayType *)Type)->getType())));
    }
}

StmtKind ASTLocalVar::getKind() const {
    return Kind;
}

ASTExpr *ASTLocalVar::getExpr() const {
    return ASTExprStmt::getExpr();
}

void ASTLocalVar::setExpr(ASTExpr *E) {
    ASTExprStmt::setExpr(E);
}

CodeGenLocalVar *ASTLocalVar::getCodeGen() const {
    return CodeGen;
}

void ASTLocalVar::setCodeGen(CodeGenLocalVar *CG) {
    CodeGen = CG;
}

std::string ASTLocalVar::str() const {
    return "{ " +
           ASTVar::str() +
           ", Kind: " + std::to_string(Kind) +
           " }";
}

ASTLocalVarRef::ASTLocalVarRef(const SourceLocation &Loc, ASTBlock *Block, const std::string &Name,
                               const std::string &NameSpace) :
        ASTExprStmt(Loc, Block), ASTVarRef(Loc, Name, NameSpace) {

}

ASTLocalVarRef::ASTLocalVarRef(const SourceLocation &Loc, ASTBlock *Block, ASTVarRef Var) :
        ASTExprStmt(Loc, Block), ASTVarRef(Var) {

}

StmtKind ASTLocalVarRef::getKind() const {
    return STMT_VAR_ASSIGN;
}

std::string ASTLocalVarRef::str() const {
    return ASTVarRef::str();
}