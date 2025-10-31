//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTExpr.h - AST Expression header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_VARREFEXPR_H
#define FLY_AST_VARREFEXPR_H

#include "ASTExpr.h"

namespace fly {

	class ASTRef;

    /**
     * Var Reference Expression
     */
    class ASTVarRefExpr : public ASTExpr {

        friend class ASTBuilder;

        ASTRef *VarRef = nullptr;

        explicit ASTVarRefExpr(ASTRef *VarRef);

    public:

    	void accept(ASTVisitor& Visitor) override;

        ASTRef *getVarRef() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_VARREFEXPR_H
