//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTSwitchBlock.cpp - Switch Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTSwitchBlock.h"
#include "AST/ASTExpr.h"
#include "Basic/Diagnostic.h"

using namespace fly;

ASTSwitchBlock::ASTSwitchBlock(const SourceLocation &Loc, ASTBlock *Parent, ASTExpr *Expr) : ASTBlock(Loc, Parent),
                                                                                              Expr(Expr) {
    if (Expr->getKind() != EXPR_REF_VAR && Expr->getKind() != EXPR_REF_FUNC) {
        Diag(Loc, diag::err_switch_expression);
        // TODO Handle Error
    }

}

ASTSwitchCaseBlock *ASTSwitchBlock::AddCase(const SourceLocation &Loc, ASTExpr *Value) {
    ASTSwitchCaseBlock *Case = new ASTSwitchCaseBlock(Loc, this, Value);
    Cases.push_back(Case);
    return Case;
}

ASTSwitchDefaultBlock *ASTSwitchBlock::setDefault(const SourceLocation &Loc) {
    Default = new ASTSwitchDefaultBlock(Loc, this);
    return Default;
}

enum ASTBlockKind ASTSwitchBlock::getBlockKind() const {
    return StmtKind;
}

std::vector<ASTSwitchCaseBlock *> &ASTSwitchBlock::getCases() {
    return Cases;
}

ASTSwitchDefaultBlock *ASTSwitchBlock::getDefault() {
    return Default;
}

ASTExpr *ASTSwitchBlock::getExpr() const {
    return Expr;
}

ASTSwitchCaseBlock::ASTSwitchCaseBlock(const SourceLocation &Loc, ASTSwitchBlock *Switch, ASTExpr *Value) : ASTBlock(Loc, Switch),
                                                                                                            Expr(Value) {

}

enum ASTBlockKind ASTSwitchCaseBlock::getBlockKind() const {
    return StmtKind;
};

ASTExpr *ASTSwitchCaseBlock::getExpr() {
    return Expr;
}

ASTSwitchDefaultBlock::ASTSwitchDefaultBlock(const SourceLocation &Loc, ASTSwitchBlock *Switch) : ASTBlock(Loc, Switch) {

}

enum ASTBlockKind ASTSwitchDefaultBlock::getBlockKind() const {
    return StmtKind;
}
