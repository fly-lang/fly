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

bool ASTIfBlock::AddBranch(ASTBlock *Parent, ASTIfBlock *Cond) {
    ASTStmt *&PrevIf = Parent->Content.at(Parent->Content.size() - 1);
    if (PrevIf->getKind() == StmtKind::STMT_BLOCK) {
        enum ASTBlockKind PrevIfKind = static_cast<ASTBlock *>(PrevIf)->getBlockKind();
        if (Cond->getBlockKind() == ASTBlockKind::BLOCK_STMT_IF) {

            assert(0 && "Do not add If branch with AddBranch()");
        } else if (Cond->getBlockKind() == ASTBlockKind::BLOCK_STMT_ELSIF) {

            ASTElsifBlock *Elsif = static_cast<ASTElsifBlock *>(Cond);
            switch (PrevIfKind) {
                case ASTBlockKind::BLOCK_STMT_IF:
                    // Add current elsif
                    Elsif->Head = static_cast<ASTIfBlock *>(PrevIf);
                    static_cast<ASTIfBlock *>(PrevIf)->Elsif.push_back(Elsif);
                    return true;
                case ASTBlockKind::BLOCK_STMT_ELSIF:
                    Elsif->Head = static_cast<ASTElsifBlock *>(PrevIf)->Head;
                    static_cast<ASTElsifBlock *>(PrevIf)->Elsif.push_back(Elsif);
                    return true;
                case ASTBlockKind::BLOCK_STMT_ELSE:
                    Parent->Top->getNode()->getContext().Diag(Cond->getLocation(), diag::err_elseif_after_else);
                    return false;
            }

        } else if (Cond->getBlockKind() == ASTBlockKind::BLOCK_STMT_ELSE) {

            ASTElseBlock *Else = static_cast<ASTElseBlock *>(Cond);
            switch (PrevIfKind) {
                case ASTBlockKind::BLOCK_STMT_IF:
                    // Add current else
                    Else->Head = static_cast<ASTIfBlock *>(PrevIf);
                    static_cast<ASTIfBlock *>(PrevIf)->Else = Else;
                    return true;
                case ASTBlockKind::BLOCK_STMT_ELSIF:
                    Else->Head = static_cast<ASTElsifBlock *>(PrevIf)->Head;
                    static_cast<ASTElsifBlock *>(PrevIf)->Head->Else = Else;
                    return true;
                case ASTBlockKind::BLOCK_STMT_ELSE:
                    Parent->Top->getNode()->getContext().Diag(Cond->getLocation(), diag::err_else_after_else);
                    return false;
            }
        }
    }
    assert(0 && "Invalid ASTBlockKind");
}

ASTIfBlock::ASTIfBlock(const SourceLocation &Loc, ASTBlock *Parent) : ASTBlock(Loc, Parent) {

}

ASTIfBlock::ASTIfBlock(const SourceLocation &Loc, ASTBlock *Parent, ASTExpr *Condition) : Condition(Condition),
    ASTBlock(Loc, Parent) {

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

enum ASTBlockKind ASTIfBlock::getBlockKind() const {
    return StmtKind;
}

ASTElsifBlock::ASTElsifBlock(const SourceLocation &Loc, ASTBlock *Parent, ASTExpr *Condition) :
    ASTIfBlock(Loc, Parent, Condition) {
    
}

enum ASTBlockKind ASTElsifBlock::getBlockKind() const {
    return StmtKind;
}

ASTElseBlock::ASTElseBlock(const SourceLocation &Loc, ASTBlock *Parent) : ASTIfBlock(Loc, Parent) {
    
}

enum ASTBlockKind ASTElseBlock::getBlockKind() const {
    return StmtKind;
}
