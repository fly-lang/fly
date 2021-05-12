//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ForStmtDecl.cpp - For Statements implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SWITCHSTMTDECL_H
#define FLY_SWITCHSTMTDECL_H


#include "StmtDecl.h"

namespace fly {

    class CaseStmtDecl;
    class DefaultStmtDecl;

    class SwitchStmtDecl : public StmtDecl {

        enum StmtKind StmtKind = StmtKind::D_STMT_SWITCH;
        const VarRef *Var;
        std::vector<CaseStmtDecl *> Cases;
        DefaultStmtDecl *Default;

    public:
        SwitchStmtDecl(const SourceLocation &Loc, StmtDecl *Parent, VarRef *Var);

        CaseStmtDecl * AddCase(const SourceLocation &Loc, Expr *Value);
        DefaultStmtDecl * AddDefault(const SourceLocation &Loc);

        enum StmtKind getStmtKind() const override;

        const std::vector<CaseStmtDecl *> &getCases() const;

        const DefaultStmtDecl *getDefault() const;
    };

    class CaseStmtDecl : public StmtDecl{

        enum StmtKind StmtKind = StmtKind::D_STMT_CASE;
        Expr *Exp;

    public:
        CaseStmtDecl(const SourceLocation &Loc, SwitchStmtDecl *Switch, Expr *Value);

        Expr *getExpr() const;

        enum StmtKind getStmtKind() const override;
    };

    class DefaultStmtDecl : public StmtDecl{

        enum StmtKind StmtKind = StmtKind::D_STMT_DEFAULT;

    public:
        DefaultStmtDecl(const SourceLocation &Loc, SwitchStmtDecl *Switch);

        enum StmtKind getStmtKind() const override;
    };
}


#endif //FLY_SWITCHSTMTDECL_H
