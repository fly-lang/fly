//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ForStmtDecl.cpp - Switch Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/SwitchStmtDecl.h"

using namespace fly;

SwitchStmtDecl::SwitchStmtDecl(const SourceLocation &Loc, StmtDecl *Parent, VarRef *Var) : StmtDecl(Loc, Parent),
        Var(Var) {

}

CaseStmtDecl *SwitchStmtDecl::AddCase(const SourceLocation &Loc, Expr *Value) {
    CaseStmtDecl *Case = new CaseStmtDecl(Loc, this, Value);
    Cases.push_back(Case);
    return Case;
}

DefaultStmtDecl *SwitchStmtDecl::AddDefault(const SourceLocation &Loc) {
    Default = new DefaultStmtDecl(Loc, this);
    return Default;
}

enum StmtKind SwitchStmtDecl::getStmtKind() const {
    return StmtKind;
}

const std::vector<CaseStmtDecl *> &SwitchStmtDecl::getCases() const {
    return Cases;
}

const DefaultStmtDecl *SwitchStmtDecl::getDefault() const {
    return Default;
}

CaseStmtDecl::CaseStmtDecl(const SourceLocation &Loc, SwitchStmtDecl *Switch, Expr *Value) : StmtDecl(Loc, Switch),
        Exp(Value) {

}

enum StmtKind CaseStmtDecl::getStmtKind() const {
    return StmtKind;
};

Expr *CaseStmtDecl::getExpr() const {
    return Exp;
}

DefaultStmtDecl::DefaultStmtDecl(const SourceLocation &Loc, SwitchStmtDecl *Switch) : StmtDecl(Loc, Switch) {

}

enum StmtKind DefaultStmtDecl::getStmtKind() const {
    return StmtKind;
}
