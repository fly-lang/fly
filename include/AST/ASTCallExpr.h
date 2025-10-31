//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTCallExpr.h - AST Var Expression header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_CALLEXPR_H
#define FLY_AST_CALLEXPR_H

#include "ASTExpr.h"

namespace fly {

	class ASTCall;

    /**
     * Function Call Expression
     */
    class ASTCallExpr : public ASTExpr {

        friend class ASTBuilder;

        ASTCall *Call = nullptr;

        explicit ASTCallExpr(ASTCall *Call);

    public:

    	void accept(ASTVisitor& Visitor) override;

        ASTCall *getCall() const;

        std::string str() const override;
    };

}

#endif //FLY_AST_CALLEXPR_H
