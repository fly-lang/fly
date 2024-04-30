//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBlock.h - AST Block Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_BLOCK_H
#define FLY_AST_BLOCK_H

#include "ASTStmt.h"

#include "llvm/ADT/StringMap.h"

namespace fly {

    class DiagnosticBuilder;
    class ASTReturnStmt;
    class ASTFunctionBase;
    class ASTCall;
    class ASTLocalVar;
    class ASTVarStmt;
    class ASTGroupExpr;
    class ASTVarRef;
    class ASTExpr;
    class ASTBreakStmt;
    class ASTContinueStmt;
    class ASTIfBlock;
    class ASTIfBlock;
    class ASTSwitchBlock;
    class ASTForBlock;
    class ASTWhileBlock;

    enum class ASTBlockKind {
        BLOCK,
        BLOCK_IF,
        BLOCK_ELSIF,
        BLOCK_ELSE,
        BLOCK_SWITCH,
        BLOCK_SWITCH_CASE,
        BLOCK_SWITCH_DEFAULT,
        BLOCK_WHILE,
        BLOCK_FOR,
        BLOCK_FOR_LOOP,
        BLOCK_FOR_POST,
        BLOCK_HANDLE
    };

    /**
     * AST Block
     */
    class ASTBlock : public ASTStmt {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class Sema;

        ASTBlockKind BlockKind;

        // List of Statements of the Block
        llvm::SmallVector<ASTStmt *, 8> Content;

        // Contains all vars declared in this Block
        llvm::StringMap<ASTLocalVar *> LocalVars;

    protected:

        ASTBlock(const SourceLocation &Loc);

        ASTBlock(const SourceLocation &Loc, ASTBlockKind Kind);

    public:

        ASTBlockKind getBlockKind() const;

        const llvm::SmallVector<ASTStmt *, 8> &getContent() const;

        bool isEmpty() const;

        void Clear();

        const llvm::StringMap<ASTLocalVar *> &getLocalVars() const;

        std::string str() const override;
    };

    /**
     * Break Stmt
     */
    class ASTBreakStmt : public ASTStmt {

    public:
        explicit ASTBreakStmt(const SourceLocation &Loc);

        std::string str() const override;
    };

    /**
     * Continue Stmt
     */
    class ASTContinueStmt : public ASTStmt {

    public:
        explicit ASTContinueStmt(const SourceLocation &Loc);

        std::string str() const override;
    };
}

#endif //FLY_AST_BLOCK_H
