//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTValueExpr.h - AST Value Expression header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_VALUEEXPR_H
#define FLY_AST_VALUEEXPR_H

#include "ASTExpr.h"

namespace fly {

	class ASTValue;

    /**
     * Value Expression
     */
    class ASTValueExpr : public ASTExpr {

        friend class ASTBuilder;

        ASTValue *Value = nullptr;

        explicit ASTValueExpr(ASTValue *Val);

    public:

    	void accept(ASTVisitor& Visitor) override;

        ASTValue *getValue() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_VALUEEXPR_H
