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
        STMT_VAR_DEFINE,
        STMT_BREAK,
        STMT_CONTINUE,
        STMT_DELETE,
        STMT_RETURN
    };

    class ASTExpr;
    class ASTBlock;
    class ASTFunctionBase;

    class ASTStmt : public ASTBase {

        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        ASTStmt *Parent = nullptr;

        ASTFunctionBase *Top = nullptr;

        ASTStmtKind Kind;

        bool HandleError = false;

        ASTStmt(ASTStmt *Parent, const SourceLocation &Loc, ASTStmtKind Kind);

    public:

        virtual ASTStmt *getParent() const;

        ASTStmtKind getKind() const;

        void setHandleError(bool HandleError);

        bool isHandlerError();

        std::string str() const;
    };
}


#endif //FLY_ASTSTMT_H
