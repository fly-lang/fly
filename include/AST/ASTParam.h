//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTParam.h - AST Var header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_PARAM_H
#define FLY_AST_PARAM_H

#include <Sema/SemaParam.h>

#include "ASTVar.h"

namespace fly {

    class SourceLocation;
    class ASTModifier;
    class ASTType;

    class ASTParam : public ASTVar {

        friend class ASTBuilder;

        SemaParam* Sema;

    protected:
        ASTParam(const SourceLocation& Loc, ASTType* Type, llvm::StringRef Name, SmallVector<ASTModifier*, 8>& Modifiers);

    public:

        void accept(ASTVisitor& Visitor) override;

        SemaParam* getSema() const override;

        void setSema(SemaParam* Sema);

    };
}

#endif //FLY_AST_PARAM_H
