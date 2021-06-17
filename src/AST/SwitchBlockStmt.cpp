//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/SwitchBlockStmt.cpp - Switch Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/SwitchBlockStmt.h"

using namespace fly;

SwitchBlockStmt::SwitchBlockStmt(const SourceLocation &Loc, BlockStmt *Parent, VarRef *Var) : BlockStmt(Loc, Parent),
                                                                                              Var(Var) {

}

CaseBlockStmt *SwitchBlockStmt::AddCase(const SourceLocation &Loc, Expr *Value) {
    CaseBlockStmt *Case = new CaseBlockStmt(Loc, this, Value);
    Cases.push_back(Case);
    return Case;
}

DefaultBlockStmt *SwitchBlockStmt::AddDefault(const SourceLocation &Loc) {
    Default = new DefaultBlockStmt(Loc, this);
    return Default;
}

enum BlockStmtKind SwitchBlockStmt::getBlockKind() const {
    return StmtKind;
}

std::vector<CaseBlockStmt *> &SwitchBlockStmt::getCases() {
    return Cases;
}

DefaultBlockStmt *SwitchBlockStmt::getDefault() {
    return Default;
}

CaseBlockStmt::CaseBlockStmt(const SourceLocation &Loc, SwitchBlockStmt *Switch, Expr *Value) : BlockStmt(Loc, Switch),
                                                                                                Exp(Value) {

}

enum BlockStmtKind CaseBlockStmt::getBlockKind() const {
    return StmtKind;
};

Expr *CaseBlockStmt::getExpr() {
    return Exp;
}

DefaultBlockStmt::DefaultBlockStmt(const SourceLocation &Loc, SwitchBlockStmt *Switch) : BlockStmt(Loc, Switch) {

}

enum BlockStmtKind DefaultBlockStmt::getBlockKind() const {
    return StmtKind;
}
