//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTParam.h - AST function parameter header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_PARAM_H
#define FLY_AST_PARAM_H

#include "ASTVar.h"

namespace fly {

    class SourceLocation;
    class ASTModifier;
    class ASTType;
    struct Symbol;

    class ASTParam : public ASTVar {

        friend class ASTBuilder;

    protected:
        ASTParam(const SourceLocation& Loc, ASTType* Type, llvm::StringRef Name, SmallVector<ASTModifier*, 8>& Modifiers);

    public:

        void accept(ASTVisitor& Visitor) override;

        Symbol* getSymbol() const override;

        void setSymbol(Symbol* Sym) override;
    };
}

#endif //FLY_AST_PARAM_H
