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

#include "Basic/SourceLocation.h"

namespace fly {

    enum StmtKind {
        STMT_BLOCK,
        STMT_EXPR,
        STMT_ARG,
        STMT_VAR_DEFINE,
        STMT_VAR_ASSIGN,
        STMT_BREAK,
        STMT_CONTINUE,
        STMT_RETURN
    };

    class ASTExpr;
    class ASTBlock;
    class ASTFunction;

    class ASTStmt {

        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        const SourceLocation Location;

        ASTStmt *Parent = nullptr;

        StmtKind Kind;

        ASTStmt(ASTStmt *Parent, const SourceLocation &Loc, StmtKind Kind);

    public:

        virtual ASTStmt *getParent() const;

        const SourceLocation &getLocation() const;

        StmtKind getKind() const;

        virtual std::string str() const = 0;
    };
}


#endif //FLY_ASTSTMT_H
