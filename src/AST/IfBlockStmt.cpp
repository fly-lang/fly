//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/IfBlockStmt.cpp - If Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/IfBlockStmt.h"

using namespace fly;

void IfBlockStmt::AddBranch(BlockStmt *Parent, ConditionBlockStmt *Cond) {
    Stmt *&PrevIf = Parent->Content.at(Parent->Content.size() - 1);
    if (PrevIf->getKind() == StmtKind::STMT_BLOCK) {
        enum BlockStmtKind PrevIfKind = static_cast<BlockStmt *>(PrevIf)->getBlockKind();
        if (Cond->getBlockKind() == BlockStmtKind::BLOCK_STMT_IF) {

            assert("Do not add If branch with AddBranch()");

        } else if (Cond->getBlockKind() == BlockStmtKind::BLOCK_STMT_ELSIF) {

            ElsifBlockStmt *Elsif = static_cast<ElsifBlockStmt *>(Cond);
            switch (PrevIfKind) {
                case BlockStmtKind::BLOCK_STMT_IF:
                    // Add current elsif
                    Elsif->Head = static_cast<IfBlockStmt *>(PrevIf);
                    static_cast<IfBlockStmt *>(PrevIf)->Elsif.push_back(Elsif);
                    break;
                case BlockStmtKind::BLOCK_STMT_ELSIF:
                    Elsif->Head = static_cast<ElsifBlockStmt *>(PrevIf)->Head;
                    static_cast<ElsifBlockStmt *>(PrevIf)->Elsif.push_back(Elsif);
                    break;
                case BlockStmtKind::BLOCK_STMT_ELSE:
                    // TODO Error cannot add elseif after an else
                    break;
                default:
                    assert("Not CondStmtDecl class");
            }

        } else if (Cond->getBlockKind() == BlockStmtKind::BLOCK_STMT_ELSE) {

            ElseBlockStmt *Else = static_cast<ElseBlockStmt *>(Cond);
            switch (PrevIfKind) {
                case BlockStmtKind::BLOCK_STMT_IF:
                    // Add current else
                    Else->Head = static_cast<IfBlockStmt *>(PrevIf);
                    static_cast<IfBlockStmt *>(PrevIf)->Else = Else;
                    break;
                case BlockStmtKind::BLOCK_STMT_ELSIF:
                    Else->Head = static_cast<ElsifBlockStmt *>(PrevIf)->Head;
                    static_cast<ElsifBlockStmt *>(PrevIf)->Head->Else = Else;
                    break;
                case BlockStmtKind::BLOCK_STMT_ELSE:
                    // TODO Error cannot add else after another else
                    break;
                default:
                    assert("Not CondStmtDecl class");
            }

        }


    } else {
        // TODO Manage Error
    }
}

IfBlockStmt::IfBlockStmt(const SourceLocation &Loc, BlockStmt *Parent) : ConditionBlockStmt(Loc, Parent) {

}

std::vector<ElsifBlockStmt *> IfBlockStmt::getElsif() const {
    return Elsif;
}

const ElseBlockStmt *IfBlockStmt::getElse() const {
    return Else;
}

const GroupExpr *IfBlockStmt::getCondition() const {
    return Condition;
}

enum BlockStmtKind IfBlockStmt::getBlockKind() const {
    return StmtKind;
}

ElsifBlockStmt::ElsifBlockStmt(const SourceLocation &Loc, BlockStmt *Parent) : IfBlockStmt(Loc, Parent) {
    
}

enum BlockStmtKind ElsifBlockStmt::getBlockKind() const {
    return StmtKind;
}

ElseBlockStmt::ElseBlockStmt(const SourceLocation &Loc, BlockStmt *Parent) : ConditionBlockStmt(Loc, Parent) {
    
}

enum BlockStmtKind ElseBlockStmt::getBlockKind() const {
    return StmtKind;
}
