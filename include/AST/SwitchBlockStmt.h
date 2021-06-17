//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/SwitchBlockStmt.cpp - For Block Statements
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SWITCHBLOCKSTMT_H
#define FLY_SWITCHBLOCKSTMT_H


#include "BlockStmt.h"

namespace fly {

    class CaseBlockStmt;
    class DefaultBlockStmt;

    class SwitchBlockStmt : public BlockStmt {

        enum BlockStmtKind StmtKind = BlockStmtKind::BLOCK_STMT_SWITCH;
        const VarRef *Var;
        std::vector<CaseBlockStmt *> Cases;
        DefaultBlockStmt *Default;

    public:
        SwitchBlockStmt(const SourceLocation &Loc, BlockStmt *Parent, VarRef *Var);

        CaseBlockStmt * AddCase(const SourceLocation &Loc, Expr *Value);
        DefaultBlockStmt * AddDefault(const SourceLocation &Loc);

        enum BlockStmtKind getBlockKind() const override;

        std::vector<CaseBlockStmt *> &getCases();

        DefaultBlockStmt *getDefault();
    };

    class CaseBlockStmt : public BlockStmt{

        enum BlockStmtKind StmtKind = BlockStmtKind::BLOCK_STMT_CASE;
        Expr *Exp;

    public:
        CaseBlockStmt(const SourceLocation &Loc, SwitchBlockStmt *Switch, Expr *Value);

        Expr *getExpr();

        enum BlockStmtKind getBlockKind() const override;
    };

    class DefaultBlockStmt : public BlockStmt{

        enum BlockStmtKind StmtKind = BlockStmtKind::BLOCK_STMT_DEFAULT;

    public:
        DefaultBlockStmt(const SourceLocation &Loc, SwitchBlockStmt *Switch);

        enum BlockStmtKind getBlockKind() const override;
    };
}


#endif //FLY_SWITCHBLOCKSTMT_H
