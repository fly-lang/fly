//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/BlockStmt.h - Block Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_BLOCKSTMT_H
#define FLY_BLOCKSTMT_H

#include "Stmt.h"
#include "VarDecl.h"
#include "llvm/ADT/StringMap.h"
#include <vector>

namespace fly {

    class ReturnStmt;
    class FuncCall;
    class FuncCallStmt;
    class VarDeclStmt;
    class VarStmt;
    class GroupExpr;

    enum BlockStmtKind {
        BLOCK_STMT,
        BLOCK_STMT_IF,
        BLOCK_STMT_ELSIF,
        BLOCK_STMT_ELSE,
        BLOCK_STMT_SWITCH,
        BLOCK_STMT_CASE,
        BLOCK_STMT_DEFAULT,
        BLOCK_STMT_FOR
    };

    class BlockStmt : public Stmt {

        friend class Parser;
        friend class FunctionParser;
        friend class IfBlockStmt;
        friend class ElsifBlockStmt;
        friend class ElseBlockStmt;
        friend class FuncDecl;

        StmtKind Kind = StmtKind::STMT_BLOCK;
        BlockStmtKind BlockKind = BlockStmtKind::BLOCK_STMT;

        std::vector<Stmt *> Content;

        llvm::StringMap<VarDeclStmt *> Vars;

    public:

        BlockStmt(const SourceLocation &Loc, BlockStmt *Parent);

        BlockStmt(const SourceLocation &Loc, FuncDecl *Container, BlockStmt *Parent);

        StmtKind getKind() const override;

        virtual enum BlockStmtKind getBlockKind() const {
            return BlockKind;
        };

        const std::vector<Stmt *> &getContent() const;

        bool isEmpty() const;

        bool addVar(VarStmt *Var);

        bool addVarDecl(VarDeclStmt *Var);

        bool addCall(FuncCall *Invoke);

        const llvm::StringMap<VarDeclStmt *> &getVars() const;

        void addReturn(const SourceLocation &Loc, GroupExpr *Expr);

    };

    class ConditionBlockStmt : public BlockStmt {

    public:
        ConditionBlockStmt(const SourceLocation &Loc, BlockStmt *Parent);
    };

    class LoopBlockStmt : public BlockStmt {

    public:
        LoopBlockStmt(const SourceLocation &Loc, BlockStmt *Parent);
    };

    class BreakStmt : public Stmt {

        StmtKind Kind = StmtKind::STMT_BREAK;

    public:
        BreakStmt(const SourceLocation &Loc, BlockStmt *CurrStmt);

        StmtKind getKind() const override;
    };

    class ContinueStmt : public Stmt {

        StmtKind Kind = StmtKind::STMT_CONTINUE;

    public:
        ContinueStmt(const SourceLocation &Loc, BlockStmt *CurrStmt);

        StmtKind getKind() const override;
    };
}


#endif //FLY_BLOCKSTMT_H
