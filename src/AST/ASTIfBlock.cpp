//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTIfBlock.cpp - If Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTIfBlock.h"
#include "AST/ASTFunc.h"
#include "AST/ASTNode.h"
#include "AST/ASTContext.h"

using namespace fly;

void ASTIfBlock::AddBranch(ASTBlock *Parent, ConditionBlockStmt *Cond) {
    ASTStmt *&PrevIf = Parent->Content.at(Parent->Content.size() - 1);
    if (PrevIf->getKind() == StmtKind::STMT_BLOCK) {
        enum BlockStmtKind PrevIfKind = static_cast<ASTBlock *>(PrevIf)->getBlockKind();
        if (Cond->getBlockKind() == BlockStmtKind::BLOCK_STMT_IF) {

            assert(0 && "Do not add If branch with AddBranch()");
        } else if (Cond->getBlockKind() == BlockStmtKind::BLOCK_STMT_ELSIF) {

            ASTElsifBlock *Elsif = static_cast<ASTElsifBlock *>(Cond);
            switch (PrevIfKind) {
                case BlockStmtKind::BLOCK_STMT_IF:
                    // Add current elsif
                    Elsif->Head = static_cast<ASTIfBlock *>(PrevIf);
                    static_cast<ASTIfBlock *>(PrevIf)->Elsif.push_back(Elsif);
                    break;
                case BlockStmtKind::BLOCK_STMT_ELSIF:
                    Elsif->Head = static_cast<ASTElsifBlock *>(PrevIf)->Head;
                    static_cast<ASTElsifBlock *>(PrevIf)->Elsif.push_back(Elsif);
                    break;
                case BlockStmtKind::BLOCK_STMT_ELSE:
                    Parent->Top->getNode()->getContext().Diag(Cond->getLocation(), diag::err_elseif_after_else);
                    break;
                default:
                    assert(0 && "Not CondStmtDecl class");
            }

        } else if (Cond->getBlockKind() == BlockStmtKind::BLOCK_STMT_ELSE) {

            ASTElseBlock *Else = static_cast<ASTElseBlock *>(Cond);
            switch (PrevIfKind) {
                case BlockStmtKind::BLOCK_STMT_IF:
                    // Add current else
                    Else->Head = static_cast<ASTIfBlock *>(PrevIf);
                    static_cast<ASTIfBlock *>(PrevIf)->Else = Else;
                    break;
                case BlockStmtKind::BLOCK_STMT_ELSIF:
                    Else->Head = static_cast<ASTElsifBlock *>(PrevIf)->Head;
                    static_cast<ASTElsifBlock *>(PrevIf)->Head->Else = Else;
                    break;
                case BlockStmtKind::BLOCK_STMT_ELSE:
                    // TODO Error cannot add else after another else
                    break;
                default:
                    assert(0 && "Not CondStmtDecl class");
            }

        }


    } else {
        // TODO Manage Error
    }
}

ASTIfBlock::ASTIfBlock(const SourceLocation &Loc, ASTBlock *Parent) : ConditionBlockStmt(Loc, Parent) {

}

ASTIfBlock::ASTIfBlock(const SourceLocation &Loc, ASTBlock *Parent, ASTExpr *Condition) : Condition(Condition),
    ConditionBlockStmt(Loc, Parent) {

}

std::vector<ASTElsifBlock *> ASTIfBlock::getElsif() {
    return Elsif;
}

ASTElseBlock *ASTIfBlock::getElse() {
    return Else;
}

ASTExpr *ASTIfBlock::getCondition() {
    return Condition;
}

enum BlockStmtKind ASTIfBlock::getBlockKind() const {
    return StmtKind;
}

ASTElsifBlock::ASTElsifBlock(const SourceLocation &Loc, ASTBlock *Parent, ASTExpr *Condition) :
    ASTIfBlock(Loc, Parent, Condition) {
    
}

enum BlockStmtKind ASTElsifBlock::getBlockKind() const {
    return StmtKind;
}

ASTElseBlock::ASTElseBlock(const SourceLocation &Loc, ASTBlock *Parent) : ConditionBlockStmt(Loc, Parent) {
    
}

enum BlockStmtKind ASTElseBlock::getBlockKind() const {
    return StmtKind;
}
