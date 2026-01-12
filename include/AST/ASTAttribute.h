//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTAttribute.h - AST Var header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_ATTRIBUTE_H
#define FLY_AST_ATTRIBUTE_H

#include "ASTVar.h"
#include "Sema/SemaClassAttribute.h"

namespace fly {

    class ASTModifier;
    class ASTType;
	class ASTExpr;

    class ASTAttribute : public ASTVar {

        friend class ASTBuilder;

        SemaClassAttribute* Sema;

    protected:
        ASTAttribute(const SourceLocation& Loc, ASTType* Type, llvm::StringRef Name, SmallVector<ASTModifier*, 8>& Modifiers);

    public:

        void accept(ASTVisitor& Visitor) override;

        SemaClassAttribute* getSema() const override;

        void setSema(SemaClassAttribute* Sema);

    };
}

#endif //FLY_AST_ATTRIBUTE_H
