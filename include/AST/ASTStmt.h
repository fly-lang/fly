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
        STMT_VAR,
        STMT_VAR_ASSIGN,
        STMT_FUNCTION_CALL,
        STMT_BREAK,
        STMT_CONTINUE,
        STMT_RETURN
    };

    class ASTExpr;
    class ASTBlock;
    class ASTFunction;

    class ASTStmt {

    protected:

        const SourceLocation Location;

        ASTFunction *Top;

        ASTBlock *Parent;

    public:
        ASTStmt(const SourceLocation &Loc);

        const SourceLocation &getLocation() const;

        virtual StmtKind getKind() const = 0;

        ASTFunction *getTop() const;

        ASTBlock *getParent() const;

        virtual std::string str() const = 0;
    };
}


#endif //FLY_ASTSTMT_H
