//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBuilderIfStmt.h - AST builder for if statements
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_BUILDERIFSTMT_H
#define FLY_AST_BUILDERIFSTMT_H

namespace fly {

    class ASTBuilder;
    class ASTIfStmt;
    class ASTStmt;
    class ASTBlockStmt;
    class SourceLocation;
    class ASTExpr;

    class ASTBuilderIfStmt {

        friend class ASTIfStmt;

        ASTBlockStmt *Parent; // FIXME remove, extends Stmt?

        ASTIfStmt *IfStmt = nullptr;

        explicit ASTBuilderIfStmt(ASTBlockStmt *Parent);

    public:

        static ASTBuilderIfStmt *Create(ASTBlockStmt *Parent);

        ASTBuilderIfStmt *If(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt);

        ASTBuilderIfStmt *ElseIf(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt);

        ASTBuilderIfStmt *Else(const SourceLocation &Loc, ASTBlockStmt *Stmt);
    };
}

#endif //FLY_AST_BUILDERIFSTMT_H
