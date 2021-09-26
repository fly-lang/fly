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
        STMT_FUNC_CALL,
        STMT_VAR_DECL,
        STMT_VAR_ASSIGN,
        STMT_BREAK,
        STMT_CONTINUE,
        STMT_RETURN
    };

    class ASTExpr;
    class ASTBlock;
    class ASTFunc;

    class ASTStmt {

        const SourceLocation Location;

    protected:
        ASTFunc *Top;

        const ASTBlock *Parent;

    public:
        ASTStmt(const SourceLocation &Loc, ASTBlock *Parent);

        ASTStmt(const SourceLocation &Loc, ASTFunc *Top, ASTBlock *Parent);

        const SourceLocation &getLocation() const;

        virtual StmtKind getKind() const = 0;

        ASTFunc *getTop() const;

        const ASTBlock *getParent() const;

        virtual std::string str() const = 0;
    };

    class ASTExprStmt : public ASTStmt {

        ASTExpr *Expr = nullptr;

    public:
        ASTExprStmt(const SourceLocation &Loc, ASTBlock *Block);

        StmtKind getKind() const override;

        ASTExpr *getExpr() const;

        void setExpr(ASTExpr *E);

        std::string str() const override;
    };
}


#endif //FLY_ASTSTMT_H
