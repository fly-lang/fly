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

ASTLocalVar::ASTLocalVar(const SourceLocation &Loc, ASTBlock *Block, ASTType *Type, const StringRef &Name) :
        ASTStmt(Loc, Block), ASTVar(Type, Name) {}

StmtKind ASTLocalVar::getKind() const {
    return Kind;
}

unsigned long ASTLocalVar::getOrder() const {
    return Order;
}

void ASTLocalVar::setOrder(unsigned long order) {
    Order = order;
}

CodeGenVar *ASTLocalVar::getCodeGen() const {
    return CodeGen;
}

void ASTLocalVar::setCodeGen(CodeGenVar *CG) {
    CodeGen = CG;
}

ASTLocalVarStmt::ASTLocalVarStmt(const SourceLocation &Loc, ASTBlock *Block, const llvm::StringRef &Name,
                                 const StringRef &NameSpace) :
        ASTStmt(Loc, Block), ASTVarRef(Loc, Name, NameSpace), Expr(new ASTGroupExpr) {

}

StmtKind ASTLocalVarStmt::getKind() const {
    return STMT_VAR_ASSIGN;
}

ASTGroupExpr *ASTLocalVarStmt::getExpr() const {
    return Expr;
}

void ASTLocalVarStmt::setExpr(ASTGroupExpr *E) {
    Expr = E;
}