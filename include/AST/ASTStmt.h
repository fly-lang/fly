//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTStmt.h - AST Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_STMT_H
#define FLY_AST_STMT_H

#include "ASTNode.h"


namespace fly {

    enum class ASTStmtKind {
        STMT_BLOCK,
        STMT_EXPR,
        STMT_FAIL,
        STMT_HANDLE,
        STMT_BREAK,
        STMT_CONTINUE,
        STMT_DELETE,
        STMT_RETURN,
        STMT_RULE,
        STMT_IF,
        STMT_SWITCH,
        STMT_LOOP,
        STMT_LOOP_IN
    };

    class ASTExpr;
    class ASTBlockStmt;
    class ASTVar;
    class ASTFunction;

    class ASTStmt : public ASTNode {

        friend class ASTBuilder;

    protected:

        ASTStmtKind StmtKind;

        ASTStmt *Parent = nullptr;

        ASTFunction *Function = nullptr; // TODO: remove

        ASTStmt(const SourceLocation &Loc, ASTStmtKind Kind);

    public:

        virtual ASTStmt *getParent() const;

        ASTFunction *getFunction() const;

        // Setters for parent/function (used by ASTBlockStmt when adding content)
        void setParent(ASTStmt *P);
        void setFunction(ASTFunction *F);

        ASTStmtKind getStmtKind() const;

        std::string str() const override;
    };
}


#endif //FLY_AST_STMT_H
