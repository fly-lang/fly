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

#include "Basic/Debuggable.h"
#include "Basic/SourceLocation.h"

namespace fly {

    enum class ASTStmtKind {
        STMT_BLOCK,
        STMT_EXPR,
        STMT_VAR_DEFINE,
        STMT_VAR_ASSIGN,
        STMT_BREAK,
        STMT_CONTINUE,
        STMT_RETURN
    };

    class ASTExpr;
    class ASTBlock;
    class ASTFunctionBase;

    class ASTStmt : public virtual Debuggable {

        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        const SourceLocation Location;

        ASTStmt *Parent = nullptr;

        ASTFunctionBase *Top = nullptr;

        ASTStmtKind Kind;

        ASTStmt(ASTStmt *Parent, const SourceLocation &Loc, ASTStmtKind Kind);

    public:

        virtual ASTStmt *getParent() const;

        const SourceLocation &getLocation() const;

        ASTStmtKind getKind() const;

        std::string str() const;
    };
}


#endif //FLY_ASTSTMT_H
