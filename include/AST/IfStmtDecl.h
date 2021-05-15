//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/IfStmtDecl.cpp - If Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_IFSTMTDECL_H
#define FLY_IFSTMTDECL_H

#include "StmtDecl.h"

namespace fly {

    class IfStmtDecl;
    class ElsifStmtDecl;
    class ElseStmtDecl;

    class IfStmtDecl : public CondStmtDecl {

        friend class Parser;
        friend class ElsifStmtDecl;
        friend class ElseStmtDecl;

        enum StmtKind StmtKind = StmtKind::D_STMT_IF;
        GroupExpr *Condition;
        std::vector<ElsifStmtDecl *> Elsif;
        ElseStmtDecl *Else = NULL;

    public:
        IfStmtDecl(const SourceLocation &Loc, StmtDecl *Parent);

        static void AddBranch(StmtDecl *Parent, CondStmtDecl *Cond);

        enum StmtKind getStmtKind() const override;

        const GroupExpr *getCondition() const;

        std::vector<ElsifStmtDecl *> getElsif() const;

        const ElseStmtDecl *getElse() const;
    };

    class ElsifStmtDecl : public IfStmtDecl {

        friend class Parser;
        friend class IfStmtDecl;

        enum StmtKind StmtKind = StmtKind::D_STMT_ELSIF;
        IfStmtDecl *Head;

    public:
        ElsifStmtDecl(const SourceLocation &Loc, StmtDecl *Parent);

        enum StmtKind getStmtKind() const override;
    };

    class ElseStmtDecl : public CondStmtDecl {

        friend class Parser;
        friend class IfStmtDecl;

        enum StmtKind StmtKind = StmtKind::D_STMT_ELSE;
        IfStmtDecl *Head;

    public:
        ElseStmtDecl(const SourceLocation &Loc, StmtDecl *Parent);

        enum StmtKind getStmtKind() const override;
    };

}


#endif //FLY_IFSTMTDECL_H
