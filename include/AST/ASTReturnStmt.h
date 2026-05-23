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

namespace fly {

    /**
     * The Return Declaration into a Function
     * Ex.
     *   return true
     */
    class ASTReturnStmt : public ASTStmt {

        friend class ASTBuilder;
        friend class ASTBuilderStmt;

        ASTReturnStmt(const SourceLocation &Loc);

    public:

        void accept(ASTVisitor& Visitor) override;

        std::string str() const override;
    };
}

#endif // FLY_AST_RETURNSTMT_H