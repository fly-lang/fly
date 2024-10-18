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

#include "ASTBase.h"

namespace fly {

    enum class ASTStmtKind {
        STMT_BLOCK,
        STMT_EXPR,
        STMT_ASSIGN,
        STMT_FAIL,
        STMT_HANDLE,
        STMT_BREAK,
        STMT_CONTINUE,
        STMT_DELETE,
        STMT_RETURN,
        STMT_IF,
        STMT_SWITCH,
        STMT_LOOP,
        STMT_LOOP_IN
    };

    class ASTExpr;
    class ASTBlockStmt;
    class ASTVar;
    class ASTFunctionBase;

    class ASTStmt : public ASTBase {

        friend class SemaBuilder;
        friend class SemaBuilderStmt;
        friend class SemaBuilderIfStmt;
        friend class SemaBuilderSwitchStmt;
        friend class SemaBuilderLoopStmt;
        friend class SemaResolver;
        friend class SemaValidator;

    protected:

        ASTStmtKind Kind;

        ASTStmt *Parent = nullptr;

        ASTFunctionBase *Function = nullptr;

        ASTStmt(const SourceLocation &Loc, ASTStmtKind Kind);

    public:

        virtual ASTStmt *getParent() const;

        ASTFunctionBase *getFunction() const;

        ASTStmtKind getKind() const;

        std::string str() const override;
    };
}


#endif //FLY_AST_STMT_H
