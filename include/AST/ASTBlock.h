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

#include "ASTStmt.h"
#include "ASTVar.h"
#include "Basic/Diagnostic.h"
#include "llvm/ADT/StringMap.h"
#include <vector>

namespace fly {

    class ASTReturn;
    class ASTFuncCall;
    class ASTFuncCallStmt;
    class ASTLocalVar;
    class ASTLocalVarRef;
    class ASTGroupExpr;
    class BreakStmt;
    class ContinueStmt;

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

        friend class Parser;
        friend class FunctionParser;
        friend class ASTIfBlock;
        friend class ASTElsifBlock;
        friend class ASTElseBlock;
        friend class ASTFunc;

        // Kind of Stmt identified by enum
        StmtKind Kind = StmtKind::STMT_BLOCK;

        // Kind of BlockStmt identified by enum
        ASTBlockKind BlockKind = ASTBlockKind::BLOCK_STMT;

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

        virtual enum ASTBlockKind getBlockKind() const {
            return BlockKind;
        };

        const std::vector<ASTStmt *> &getContent() const;

        bool isEmpty() const;

        void Clear();

        const llvm::StringMap<ASTLocalVar *> &getDeclVars() const;

        ASTLocalVar *FindVarDecl(const ASTBlock *Block, ASTVarRef *VarRef);

        bool ResolveVarRef(ASTVarRef *VarRef);

        bool ResolveExpr(ASTExpr *Expr);

        bool AddExprStmt(ASTExprStmt *ExprStmt);

        bool AddVarRef(ASTLocalVarRef *LocalVarRef);

        bool AddVar(ASTLocalVar *LocalVar);

        bool AddCall(ASTFuncCall *Invoke);

        bool AddReturn(const SourceLocation &Loc, ASTExpr *Expr);

        bool AddBreak(const SourceLocation &Loc);

        bool AddContinue(const SourceLocation &Loc);

        bool AddBlock(const SourceLocation &Loc, ASTBlock *Block);

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID);
    };

    /**
     * Break Stmt
     */
    class BreakStmt : public ASTStmt {

        StmtKind Kind = StmtKind::STMT_BREAK;

    public:
        BreakStmt(const SourceLocation &Loc, ASTBlock *Parent);

        StmtKind getKind() const override;
    };

    /**
     * Continue Stmt
     */
    class ContinueStmt : public ASTStmt {

        StmtKind Kind = StmtKind::STMT_CONTINUE;

    public:
        ContinueStmt(const SourceLocation &Loc, ASTBlock *Parent);

        StmtKind getKind() const override;
    };
}


#endif //FLY_ASTBLOCK_H
