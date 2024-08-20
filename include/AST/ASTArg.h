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
    class ASTParam;
    class ASTCall;

    class ASTArg : public ASTBase {

        friend class SemaResolver;

        friend class SemaBuilder;

        ASTExpr *Expr;

        uint64_t Index;

        ASTParam *Def = nullptr;

        ASTCall *Call = nullptr;

        ASTArg(ASTExpr *Expr, uint64_t Index);

    public:

        ASTExpr *getExpr() const;

        uint64_t getIndex() const;

        ASTParam *getDef() const;

        ASTCall *getCall() const;

        std::string str() const override;

    };
}

#endif //FLY_AST_CALL_H