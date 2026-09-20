//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTReturnStmt.h - AST return statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_RETURNSTMT_H
#define FLY_AST_RETURNSTMT_H

#include "ASTStmt.h"

#include <llvm/ADT/SmallVector.h>

namespace fly {

    class ASTExpr;

    /**
     * The Return Declaration into a Function
     * Ex.
     *   return true
     *   return a, b     // multi-return: one value per declared return type
     *
     * The values are not the lowered form: Resolver rewrites them into assignments
     * to the hidden return params (__ret / __ret_i) emitted just before the return,
     * so codegen sees exactly what the old explicit `out = expr` used to write.
     */
    class ASTReturnStmt : public ASTStmt {

        friend class ASTBuilder;
        friend class ASTBuilderStmt;
        friend class Parser;

        ASTReturnStmt(const SourceLocation &Loc);

        llvm::SmallVector<ASTExpr *, 4> Exprs;

    public:

        void accept(ASTVisitor& Visitor) override;

        void addExpr(ASTExpr *Expr);

        const llvm::SmallVector<ASTExpr *, 4> &getExprs() const;

        std::string str() const override;
    };
}

#endif // FLY_AST_RETURNSTMT_H