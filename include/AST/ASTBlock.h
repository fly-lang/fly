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

#include "ASTExprStmt.h"
#include "llvm/ADT/StringMap.h"
#include <vector>

namespace fly {

    class DiagnosticBuilder;
    class ASTReturn;
    class ASTFunctionCall;
    class ASTLocalVar;
    class ASTVarAssign;
    class ASTGroupExpr;
    class ASTVarRef;
    class ASTExpr;
    class BreakStmt;
    class ContinueStmt;
    class ASTIfBlock;
    class ASTIfBlock;
    class ASTSwitchBlock;
    class ASTForBlock;
    class ASTWhileBlock;

    enum ASTBlockKind {
        BLOCK_STMT,
        BLOCK_STMT_IF,
        BLOCK_STMT_ELSIF,
        BLOCK_STMT_ELSE,
        BLOCK_STMT_SWITCH,
        BLOCK_STMT_CASE,
        BLOCK_STMT_DEFAULT,
        BLOCK_STMT_WHILE,
        BLOCK_STMT_FOR
    };

    /**
     * AST Block
     */
    class ASTBlock : public ASTStmt {

        friend class SemaBuilder;
        friend class Sema;

        // Kind of Stmt identified by enum
        StmtKind Kind = StmtKind::STMT_BLOCK;

        // Kind of BlockStmt identified by enum
        ASTBlockKind BlockKind = ASTBlockKind::BLOCK_STMT;

        // List of Statements of the Block
        std::vector<ASTStmt *> Content;

        // Contains all vars declared in this Block
        llvm::StringMap<ASTLocalVar *> LocalVars;

    protected:

        // Contains all declared vars not yet defined with value;
        llvm::StringMap<ASTLocalVar *> UndefVars;

    public:

        ASTBlock(const SourceLocation &Loc);

        StmtKind getKind() const override;

        virtual enum ASTBlockKind getBlockKind() const {
            return BlockKind;
        };

        const std::vector<ASTStmt *> &getContent() const;

        bool isEmpty() const;

        void Clear();

        const llvm::StringMap<ASTLocalVar *> &getLocalVars() const;

        std::string str() const override;
    };

    /**
     * Break Stmt
     */
    class BreakStmt : public ASTStmt {

        StmtKind Kind = StmtKind::STMT_BREAK;

    public:
        BreakStmt(const SourceLocation &Loc);

        StmtKind getKind() const override;

        std::string str() const override;
    };

    /**
     * Continue Stmt
     */
    class ContinueStmt : public ASTStmt {

        StmtKind Kind = StmtKind::STMT_CONTINUE;

    public:
        ContinueStmt(const SourceLocation &Loc);

        StmtKind getKind() const override;

        std::string str() const override;
    };
}


#endif //FLY_ASTBLOCK_H
