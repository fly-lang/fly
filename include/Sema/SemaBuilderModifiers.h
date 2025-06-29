//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaBuilderScope.h - Sema Builder Scope
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_BUILDERSCOPE_H
#define FLY_SEMA_BUILDERSCOPE_H

#include "llvm/ADT/SmallVector.h"

namespace fly {

    class ASTModifier;
    class SourceLocation;

    enum class ASTVisibilityKind;

    class SemaBuilderModifiers {

        llvm::SmallVector<ASTModifier *, 8> Modifiers;

    public:

        static SemaBuilderModifiers *Build();

        SemaBuilderModifiers *addVisibility(const SourceLocation &Loc, ASTVisibilityKind VisibilityKind);

        SemaBuilderModifiers *addConstant(const SourceLocation &Loc);

        SemaBuilderModifiers *addStatic(const SourceLocation &Loc);

        llvm::SmallVector<ASTModifier *, 8> getModifiers() const;
    };
}

#endif //FLY_SEMA_BUILDERSCOPE_H
