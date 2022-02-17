//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTLocalVar.cpp - AST Local Var Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTLocalVar.h"

using namespace fly;

ASTLocalVar::ASTLocalVar(const SourceLocation &Loc, ASTBlock *Block, ASTType *Type, const std::string Name) :
        ASTExprStmt(Loc, Block), ASTVar(Type, Name) {
    switch (Type->getKind()) {

        case TYPE_INT:
            setExpr(new ASTValueExpr(Loc, new ASTValue(Loc, "0", Type)));
            break;
        case TYPE_FLOAT:
            setExpr(new ASTValueExpr(Loc, new ASTValue(Loc, "0", Type)));
            break;
        case TYPE_BOOL:
            setExpr(new ASTValueExpr(Loc, new ASTValue(Loc, "false", Type)));
            break;
        case TYPE_CLASS:
            setExpr(new ASTValueExpr(Loc, new ASTValue(Loc, "null", Type)));
            break;
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