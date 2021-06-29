//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/Stmt.h - Abstract Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_STMT_H
#define FLY_STMT_H

#include "Basic/SourceLocation.h"

namespace fly {

    class ASTNode; // Pre-declare

    enum StmtKind {
        STMT_BLOCK,
        STMT_FUNC_CALL,
        STMT_VAR_DECL,
        STMT_VAR_ASSIGN,
        DECL_TYPE,
        STMT_BREAK,
        STMT_CONTINUE,
        STMT_RETURN
    };

    class BlockStmt;
    class FuncDecl;

    class Stmt {

        const SourceLocation Location;

    protected:
        FuncDecl *Top;

        const BlockStmt *Parent;

    public:
        Stmt(const SourceLocation &Loc, BlockStmt *Parent);

        Stmt(const SourceLocation &Loc, FuncDecl *Container, BlockStmt *Parent);

        const SourceLocation &getLocation() const;

        virtual StmtKind getKind() const = 0;

        FuncDecl *getTop() const;

        const BlockStmt *getParent() const;
    };
}


#endif //FLY_STMT_H
