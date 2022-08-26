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

using namespace fly;

ASTSwitchBlock::ASTSwitchBlock(ASTBlock *Parent, const SourceLocation &Loc, ASTExpr *Expr) : ASTBlock(Parent, Loc),
    Expr(Expr) {

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

ASTSwitchCaseBlock::ASTSwitchCaseBlock(ASTSwitchBlock *SwitchBlock, const SourceLocation &Loc, ASTExpr *Value) :
    ASTBlock(SwitchBlock, Loc), Expr(Value) {

}

enum ASTBlockKind ASTSwitchCaseBlock::getBlockKind() const {
    return StmtKind;
};

ASTExpr *ASTSwitchCaseBlock::getExpr() {
    return Expr;
}

ASTSwitchDefaultBlock::ASTSwitchDefaultBlock(ASTSwitchBlock *SwitchBlock, const SourceLocation &Loc) :
    ASTBlock(SwitchBlock, Loc) {

}

enum ASTBlockKind ASTSwitchDefaultBlock::getBlockKind() const {
    return StmtKind;
}
