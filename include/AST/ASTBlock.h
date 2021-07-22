//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBlock.h - Block Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTBLOCK_H
#define FLY_ASTBLOCK_H

#include "Basic/Diagnostic.h"
#include "ASTStmt.h"
#include "ASTVar.h"
#include "llvm/ADT/StringMap.h"
#include <vector>

namespace fly {

    class ASTReturn;
    class ASTFuncCall;
    class ASTFuncCallStmt;
    class ASTLocalVar;
    class ASTLocalVarStmt;
    class ASTGroupExpr;
    class BreakStmt;
    class ContinueStmt;

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

    class ASTBlock : public ASTStmt {

        friend class Parser;
        friend class FunctionParser;
        friend class ASTIfBlock;
        friend class ElsifBlockStmt;
        friend class ElseBlockStmt;
        friend class ASTFunc;

        // Kind of Stmt identified by enum
        StmtKind Kind = StmtKind::STMT_BLOCK;

        // Kind of BlockStmt identified by enum
        BlockStmtKind BlockKind = BlockStmtKind::BLOCK_STMT;

        // List of Statements of the Block
        std::vector<ASTStmt *> Content;

        // Contains all vars declared in this Block
        llvm::StringMap<ASTLocalVar *> DeclVars;

        // Order assigned when add a VarDeclStmt or when add VarStmt
        unsigned long Order = 0;

    public:

        ASTBlock(const SourceLocation &Loc, ASTBlock *Parent);

        ASTBlock(const SourceLocation &Loc, ASTFunc *Top, ASTBlock *Parent);

        StmtKind getKind() const override;

        virtual enum BlockStmtKind getBlockKind() const {
            return BlockKind;
        };

        const std::vector<ASTStmt *> &getContent() const;

        bool isEmpty() const;

        const llvm::StringMap<ASTLocalVar *> &getDeclVars() const;

        ASTLocalVar *findVarDecl(const ASTBlock *Block, ASTVarRef *Var);

        bool ResolveVarRef(ASTVarRef *Var);

        bool ResolveExpr(ASTExpr *E);

        bool addVar(ASTLocalVarStmt *Var);

        bool addVar(ASTLocalVar *Var);

        bool addCall(ASTFuncCall *Invoke);

        bool addReturn(const SourceLocation &Loc, ASTGroupExpr *Expr);

        bool addBreak(const SourceLocation &Loc);

        bool addContinue(const SourceLocation &Loc);

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID);
    };

    class ConditionBlockStmt : public ASTBlock {

    public:
        ConditionBlockStmt(const SourceLocation &Loc, ASTBlock *Parent);
    };

    class LoopBlockStmt : public ASTBlock {

    public:
        LoopBlockStmt(const SourceLocation &Loc, ASTBlock *Parent);
    };

    class BreakStmt : public ASTStmt {

        StmtKind Kind = StmtKind::STMT_BREAK;

    public:
        BreakStmt(const SourceLocation &Loc, ASTBlock *Parent);

        StmtKind getKind() const override;
    };

    class ContinueStmt : public ASTStmt {

        StmtKind Kind = StmtKind::STMT_CONTINUE;

    public:
        ContinueStmt(const SourceLocation &Loc, ASTBlock *Parent);

        StmtKind getKind() const override;
    };
}


#endif //FLY_ASTBLOCK_H
