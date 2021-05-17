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
        STMT_FUNC_CALL,
        STMT_VAR_DECL,
        STMT_VAR_ASSIGN,
        STMT_BLOCK,
        DECL_TYPE,
        STMT_BREAK,
        STMT_CONTINUE,
        STMT_RETURN
    };

    class Stmt {

        const SourceLocation Location;

    public:
        explicit Stmt(const SourceLocation &Loc);

        const SourceLocation &getLocation() const;

        virtual StmtKind getKind() const = 0;
    };
}


#endif //FLY_STMT_H
