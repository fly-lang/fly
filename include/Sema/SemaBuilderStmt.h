//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaBuilderStmt.h - Sema Builder Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_BUILDERSTMT_H
#define FLY_SEMA_BUILDERSTMT_H

#include "llvm/ADT/SmallVector.h"

namespace fly {

    class SemaBuilder;
    class ASTStmt;
    class ASTBlockStmt;
    class SourceLocation;
    class ASTExpr;
    class ASTVarRef;
    class ASTVar;

    class SemaBuilderStmt {

        SemaBuilder *Builder;

        SemaBuilderStmt(SemaBuilder *Builder);

        ASTStmt *Stmt;

    public:

        static SemaBuilderStmt *CreateVar(SemaBuilder *Builder, ASTBlockStmt *Parent, ASTVarRef *VarRef);

        static SemaBuilderStmt *CreateVar(SemaBuilder *Builder, ASTBlockStmt *Parent, ASTVar *Var);

        static SemaBuilderStmt *CreateReturn(SemaBuilder *Builder, ASTBlockStmt *Parent, const SourceLocation &Loc);

        static SemaBuilderStmt *CreateFail(SemaBuilder *Builder, ASTBlockStmt *Parent, const SourceLocation &Loc);

        static SemaBuilderStmt *CreateExpr(SemaBuilder *Builder, ASTBlockStmt *Parent, const SourceLocation &Loc);

        void setExpr(ASTExpr *Expr);
    };
}

#endif //FLY_SEMA_BUILDERSTMT_H
