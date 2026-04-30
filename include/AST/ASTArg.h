//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTArg.h - AST Call arg
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_ARG_H
#define FLY_AST_ARG_H

#include "ASTBase.h"

namespace fly {

    class ASTExpr;
    class ASTVar;
    class ASTCall;
    class ASTVisitor;

    class ASTArg : public ASTBase {

        friend class ASTBuilder;

        ASTExpr *Expr;

        size_t Index;

        ASTArg(ASTExpr *Expr, size_t Index);

    public:

        ASTExpr *getExpr() const;

        size_t getIndex() const;

        std::string str() const override;

    };
}

#endif //FLY_AST_ARG_H