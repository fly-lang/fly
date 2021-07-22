//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTSwitchBlock.cpp - Switch Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTSwitchBlock.h"

using namespace fly;

ASTSwitchBlock::ASTSwitchBlock(const SourceLocation &Loc, ASTBlock *Parent, ASTVarRef *Var) : ASTBlock(Loc, Parent),
                                                                                              Var(Var) {

}

CaseBlockStmt *ASTSwitchBlock::AddCase(const SourceLocation &Loc, ASTExpr *Value) {
    CaseBlockStmt *Case = new CaseBlockStmt(Loc, this, Value);
    Cases.push_back(Case);
    return Case;
}

DefaultBlockStmt *ASTSwitchBlock::AddDefault(const SourceLocation &Loc) {
    Default = new DefaultBlockStmt(Loc, this);
    return Default;
}

enum BlockStmtKind ASTSwitchBlock::getBlockKind() const {
    return StmtKind;
}

std::vector<CaseBlockStmt *> &ASTSwitchBlock::getCases() {
    return Cases;
}

DefaultBlockStmt *ASTSwitchBlock::getDefault() {
    return Default;
}

CaseBlockStmt::CaseBlockStmt(const SourceLocation &Loc, ASTSwitchBlock *Switch, ASTExpr *Value) : ASTBlock(Loc, Switch),
                                                                                                  Exp(Value) {

}

enum BlockStmtKind CaseBlockStmt::getBlockKind() const {
    return StmtKind;
};

ASTExpr *CaseBlockStmt::getExpr() {
    return Exp;
}

DefaultBlockStmt::DefaultBlockStmt(const SourceLocation &Loc, ASTSwitchBlock *Switch) : ASTBlock(Loc, Switch) {

}

enum BlockStmtKind DefaultBlockStmt::getBlockKind() const {
    return StmtKind;
}
