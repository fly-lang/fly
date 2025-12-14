//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTDeclStmt.h - AST Decl Stmt
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_DECLSTMT_H
#define FLY_AST_DECLSTMT_H

#include "ASTStmt.h"

namespace fly {

	class ASTLocalVar;

    class ASTDeclStmt : public ASTStmt {

        friend class ASTBuilder;

    protected:

        ASTLocalVar *LocalVar;

        ASTExpr *Expr;

        explicit ASTDeclStmt(const SourceLocation &Loc, ASTLocalVar *LocalVar, ASTExpr *Expr = nullptr);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTLocalVar *getLocalVar() const;

        ASTExpr *getExpr() const;

        void setExpr(ASTExpr *Expr);

        std::string str() const override;
    };
}


#endif //FLY_AST_DECLSTMT_H
