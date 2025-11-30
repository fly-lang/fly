//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTEnumEntry.h - AST Var header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_ENUMENTRY_H
#define FLY_AST_ENUMENTRY_H

#include <Sema/SemaEnumEntry.h>

#include "ASTVar.h"

namespace fly {

    class SourceLocation;
    class ASTModifier;
    class ASTType;
	class ASTExpr;

    class ASTEnumEntry : public ASTVar {

        friend class ASTBuilder;

        SemaEnumEntry* Sema;

    protected:

        ASTEnumEntry(const SourceLocation& Loc, ASTType* Type, llvm::StringRef Name, SmallVector<ASTModifier*, 8>& Modifiers);

    public:

        void accept(ASTVisitor& Visitor) override;

        SemaEnumEntry* getSema() const override;

        void setSema(SemaEnumEntry* Sema);

    };
}

#endif //FLY_AST_ENUMENTRY_H
