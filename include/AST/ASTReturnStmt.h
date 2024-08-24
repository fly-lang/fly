//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTReturnStmt.h - AST Variable Assign statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_RETURNSTMT_H
#define FLY_AST_RETURNSTMT_H

#include "ASTStmt.h"

namespace fly {

    /**
     * The Return Declaration into a Function
     * Ex.
     *   return true
     */
    class ASTReturnStmt : public ASTStmt {

        friend class SemaBuilder;
        friend class SemaBuilderStmt;
        friend class SemaResolver;

        ASTExpr *Expr = nullptr;

        ASTReturnStmt(const SourceLocation &Loc);

    public:

        ASTExpr *getExpr() const;

        std::string str() const override;
    };
}

#endif // FLY_AST_RETURNSTMT_H