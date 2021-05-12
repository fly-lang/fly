//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ForStmtDecl.cpp - If Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/IfStmtDecl.h"

using namespace fly;

void IfStmtDecl::AddBranch(StmtDecl *Parent, CondStmtDecl *Cond) {
    Decl *&PrevIf = Parent->Content.at(Parent->Content.size() - 1);
    if (PrevIf->getKind() == DeclKind::D_STMT) {
        enum StmtKind PrevIfKind = static_cast<StmtDecl *>(PrevIf)->getStmtKind();
        if (Cond->getStmtKind() == StmtKind::D_STMT_IF) {

            assert("Do not add If branch with AddBranch()");

        } else if (Cond->getStmtKind() == StmtKind::D_STMT_ELSIF) {

            ElsifStmtDecl *Elsif = static_cast<ElsifStmtDecl *>(Cond);
            switch (PrevIfKind) {
                case StmtKind::D_STMT_IF:
                    // Add current elsif
                    Elsif->Head = static_cast<IfStmtDecl *>(PrevIf);
                    static_cast<IfStmtDecl *>(PrevIf)->Elsif.push_back(Elsif);
                    break;
                case StmtKind::D_STMT_ELSIF:
                    Elsif->Head = static_cast<ElsifStmtDecl *>(PrevIf)->Head;
                    static_cast<ElsifStmtDecl *>(PrevIf)->Elsif.push_back(Elsif);
                    break;
                case StmtKind::D_STMT_ELSE:
                    // TODO Error cannot add elseif after an else
                    break;
                default:
                    assert("Not CondStmtDecl class");
            }

        } else if (Cond->getStmtKind() == StmtKind::D_STMT_ELSE) {

            ElseStmtDecl *Else = static_cast<ElseStmtDecl *>(Cond);
            switch (PrevIfKind) {
                case StmtKind::D_STMT_IF:
                    // Add current else
                    Else->Head = static_cast<IfStmtDecl *>(PrevIf);
                    static_cast<IfStmtDecl *>(PrevIf)->Else = Else;
                    break;
                case StmtKind::D_STMT_ELSIF:
                    Else->Head = static_cast<ElsifStmtDecl *>(PrevIf)->Head;
                    static_cast<ElsifStmtDecl *>(PrevIf)->Head->Else = Else;
                    break;
                case StmtKind::D_STMT_ELSE:
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

IfStmtDecl::IfStmtDecl(const SourceLocation &Loc, StmtDecl *Parent) : CondStmtDecl(Loc, Parent) {

}

std::vector<ElsifStmtDecl *> IfStmtDecl::getElsif() const {
    return Elsif;
}

const ElseStmtDecl *IfStmtDecl::getElse() const {
    return Else;
}

const GroupExpr *IfStmtDecl::getCondition() const {
    return Condition;
}

enum StmtKind IfStmtDecl::getStmtKind() const {
    return StmtKind;
}

ElsifStmtDecl::ElsifStmtDecl(const SourceLocation &Loc, StmtDecl *Parent) : IfStmtDecl(Loc, Parent) {
    
}

enum StmtKind ElsifStmtDecl::getStmtKind() const {
    return StmtKind;
}

ElseStmtDecl::ElseStmtDecl(const SourceLocation &Loc, StmtDecl *Parent) : CondStmtDecl(Loc, Parent) {
    
}

enum StmtKind ElseStmtDecl::getStmtKind() const {
    return StmtKind;
}
