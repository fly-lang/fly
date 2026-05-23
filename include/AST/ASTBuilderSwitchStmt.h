//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBuilderSwitchStmt.h - AST builder for switch statements
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_BUILDERSWITCHSTMT_H
#define FLY_AST_BUILDERSWITCHSTMT_H

namespace fly {

    class ASTSwitchStmt;
    class SourceLocation;
    class ASTStmt;
    class ASTBlockStmt;
    class ASTExpr;

    class ASTBuilderSwitchStmt {

        friend class ASTSwitchStmt;

        ASTSwitchStmt *SwitchStmt = nullptr;

        explicit ASTBuilderSwitchStmt();

    public:

        static ASTBuilderSwitchStmt *Create(ASTBlockStmt *Parent, const SourceLocation &Loc, ASTExpr *Expr);

        ASTBuilderSwitchStmt *addCase(const SourceLocation &Loc, ASTExpr *Expr, ASTBlockStmt *Stmt);

        ASTBuilderSwitchStmt *setDefault(const SourceLocation &Loc, ASTBlockStmt *Stmt);

        bool hasDefault();
    };
}

#endif //FLY_AST_BUILDERSWITCHSTMT_H
