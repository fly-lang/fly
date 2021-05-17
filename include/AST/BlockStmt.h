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

#include "FuncDecl.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include <utility>
#include <vector>

namespace fly {

    class ReturnStmt;
    class FuncCallStmt;

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

        StmtKind Kind = StmtKind::STMT_BLOCK;
        BlockStmtKind BlockKind = BlockStmtKind::BLOCK_STMT;

        const BlockStmt *Parent;

        std::vector<Stmt *> Content;

        llvm::StringMap<VarDeclStmt *> Vars;

        llvm::StringMap<FuncCallStmt *> FuncCalls;

        ReturnStmt* Return;

        bool addVar(VarAssignStmt *Var);

        bool addVar(VarDeclStmt *Var);

        bool addInvoke(FuncCallStmt *Invoke);

    public:

        BlockStmt(const SourceLocation &Loc, BlockStmt *Parent);

        StmtKind getKind() const override;

        virtual enum BlockStmtKind getBlockKind() const {
            return BlockKind;
        };

        const std::vector<Stmt *> &getContent() const;

        bool isEmpty() const;

        const llvm::StringMap<VarDeclStmt *> &getVars() const;

        ReturnStmt *getReturn() const;
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
        BreakStmt(const SourceLocation &Loc);

        StmtKind getKind() const override;
    };

    class ContinueStmt : public Stmt {

        StmtKind Kind = StmtKind::STMT_CONTINUE;

    public:
        ContinueStmt(const SourceLocation &Loc);

        StmtKind getKind() const override;
    };
}


#endif //FLY_BLOCKSTMT_H