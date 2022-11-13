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
    class ASTFunctionBase;
    class ASTCall;
    class ASTLocalVar;
    class ASTVarAssign;
    class ASTGroupExpr;
    class ASTVarRef;
    class ASTExpr;
    class ASTBreak;
    class ASTContinue;
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
        BLOCK_FOR_POST
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
        std::vector<ASTStmt *> Content;

        // Contains all vars declared in this Block
        llvm::StringMap<ASTLocalVar *> LocalVars;

        // Contains all declared vars not yet defined with value;
        llvm::StringMap<ASTLocalVar *> UndefVars;

    protected:

        ASTBlock(ASTBlock *Parent, const SourceLocation &Loc);

        ASTBlock(ASTBlock *Parent, const SourceLocation &Loc, ASTBlockKind Kind);

    public:

        ASTFunctionBase *getTop() const;

        ASTBlockKind getBlockKind() const;

        const std::vector<ASTStmt *> &getContent() const;

        bool isEmpty() const;

        void Clear();

        const llvm::StringMap<ASTLocalVar *> &getLocalVars() const;

        const llvm::StringMap<ASTLocalVar *> &getUndefVars() const;

        std::string str() const override;
    };

    /**
     * Break Stmt
     */
    class ASTBreak : public ASTStmt {

    public:
        ASTBreak(ASTBlock *Parent, const SourceLocation &Loc);

        std::string str() const override;
    };

    /**
     * Continue Stmt
     */
    class ASTContinue : public ASTStmt {

    public:
        ASTContinue(ASTBlock *Parent, const SourceLocation &Loc);

        std::string str() const override;
    };
}


#endif //FLY_ASTBLOCK_H
