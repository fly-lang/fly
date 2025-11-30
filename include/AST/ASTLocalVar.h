//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTLocalVar.h - AST Var header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_LOCALVAR_H
#define FLY_AST_LOCALVAR_H

#include <Sema/SemaLocalVar.h>

#include "ASTVar.h"

namespace fly {

    class SourceLocation;
    class ASTModifier;
    class ASTType;
	class ASTExpr;

    class ASTLocalVar : public ASTVar {

        friend class ASTBuilder;

        SemaLocalVar* Sema;

    protected:
        ASTLocalVar(const SourceLocation& Loc, ASTType* Type, llvm::StringRef Name, SmallVector<ASTModifier*, 8>& Modifiers);

    public:

        void accept(ASTVisitor& Visitor) override;

        SemaLocalVar* getSema() const override;

        void setSema(SemaLocalVar* Sema);

    };
}

#endif //FLY_AST_LOCALVAR_H
