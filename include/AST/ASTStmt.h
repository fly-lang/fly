//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTStmt.h - AST Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTSTMT_H
#define FLY_ASTSTMT_H

#include "ASTBase.h"

namespace fly {

    enum class ASTStmtKind {
        STMT_BLOCK,
        STMT_EXPR,
        STMT_VAR,
        STMT_FAIL,
        STMT_HANDLE,
        STMT_BREAK,
        STMT_CONTINUE,
        STMT_DELETE,
        STMT_RETURN
    };

    class ASTExpr;
    class ASTBlock;
    class ASTVar;
    class ASTFunctionBase;

    class ASTStmt : public ASTBase {

        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        ASTStmt *Parent = nullptr;

        ASTFunctionBase *Top = nullptr;

        ASTStmtKind Kind;

        ASTVar *ErrorHandler = nullptr;

        ASTStmt(ASTStmt *Parent, const SourceLocation &Loc, ASTStmtKind Kind);

    public:

        virtual ASTStmt *getParent() const;

        ASTFunctionBase *getTop() const;

        ASTStmtKind getKind() const;

        void setErrorHandler(ASTVar *ErrorHandler);

        ASTVar *getErrorHandler();

        virtual std::string str() const;
    };
}


#endif //FLY_ASTSTMT_H
