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

ASTSwitchBlock::ASTSwitchBlock(const SourceLocation &Loc, ASTExpr *Expr) : ASTBlock(Loc), Expr(Expr) {

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

ASTSwitchCaseBlock::ASTSwitchCaseBlock(const SourceLocation &Loc, ASTExpr *Value) : ASTBlock(Loc), Expr(Value) {

}

enum ASTBlockKind ASTSwitchCaseBlock::getBlockKind() const {
    return StmtKind;
};

ASTExpr *ASTSwitchCaseBlock::getExpr() {
    return Expr;
}

ASTSwitchDefaultBlock::ASTSwitchDefaultBlock(const SourceLocation &Loc) : ASTBlock(Loc) {

}

enum ASTBlockKind ASTSwitchDefaultBlock::getBlockKind() const {
    return StmtKind;
}
