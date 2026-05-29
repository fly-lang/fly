//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBuilderStmt.h - AST builder for generic statements
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_BUILDERSTMT_H
#define FLY_AST_BUILDERSTMT_H

namespace fly {

    class ASTBuilder;
    class ASTStmt;
    class ASTBlockStmt;
    class SourceLocation;
    class ASTExpr;
    class ASTIdentifier;
    class ASTVar;
    enum class ASTAssignOperatorKind;

    class ASTBuilderStmt {

        ASTBuilderStmt();

        ASTStmt *Stmt = nullptr;

    public:

        static ASTBuilderStmt *CreateAssignment(ASTBlockStmt *Parent, ASTIdentifier *VarRef, ASTAssignOperatorKind AssignOperatorKind);

        static ASTBuilderStmt *CreateVar(ASTBlockStmt *Parent, ASTVar *Var);

        static ASTBuilderStmt *CreateReturn(ASTBlockStmt *Parent, const SourceLocation &Loc);

        static ASTBuilderStmt *CreateFail(ASTBlockStmt *Parent, const SourceLocation &Loc);

        static ASTBuilderStmt *CreateExpr(ASTBlockStmt *Parent, const SourceLocation &Loc);
    };
}

#endif //FLY_AST_BUILDERSTMT_H
