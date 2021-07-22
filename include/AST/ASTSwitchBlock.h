//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/SwitchBlock.h - For Block Statements
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTSWITCHBLOCK_H
#define FLY_ASTSWITCHBLOCK_H


#include "ASTBlock.h"

namespace fly {

    class CaseBlockStmt;
    class DefaultBlockStmt;

    class ASTSwitchBlock : public ASTBlock {

        enum BlockStmtKind StmtKind = BlockStmtKind::BLOCK_STMT_SWITCH;
        const ASTVarRef *Var;
        std::vector<CaseBlockStmt *> Cases;
        DefaultBlockStmt *Default;

    public:
        ASTSwitchBlock(const SourceLocation &Loc, ASTBlock *Parent, ASTVarRef *Var);

        CaseBlockStmt * AddCase(const SourceLocation &Loc, ASTExpr *Value);
        DefaultBlockStmt * AddDefault(const SourceLocation &Loc);

        enum BlockStmtKind getBlockKind() const override;

        std::vector<CaseBlockStmt *> &getCases();

        DefaultBlockStmt *getDefault();
    };

    class CaseBlockStmt : public ASTBlock{

        enum BlockStmtKind StmtKind = BlockStmtKind::BLOCK_STMT_CASE;
        ASTExpr *Exp;

    public:
        CaseBlockStmt(const SourceLocation &Loc, ASTSwitchBlock *Switch, ASTExpr *Value);

        ASTExpr *getExpr();

        enum BlockStmtKind getBlockKind() const override;
    };

    class DefaultBlockStmt : public ASTBlock{

        enum BlockStmtKind StmtKind = BlockStmtKind::BLOCK_STMT_DEFAULT;

    public:
        DefaultBlockStmt(const SourceLocation &Loc, ASTSwitchBlock *Switch);

        enum BlockStmtKind getBlockKind() const override;
    };
}


#endif //FLY_ASTSWITCHBLOCK_H
