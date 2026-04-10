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

#include "SemaVisibilityKind.h"
#include "llvm/ADT/SmallVector.h"

namespace fly {

    class ASTModifier;
    class SourceLocation;

    enum class ASTModifierKind;

    class SemaBuilderModifiers {

        SemaVisibilityKind Visibility = SemaVisibilityKind::DEFAULT;

        bool Constant = false;

        bool Static = false;

        bool Abstract = false;

        bool Final = false;

    public:

        static SemaBuilderModifiers *Build(llvm::SmallVector<ASTModifier *, 8> Modifiers);

        SemaVisibilityKind getVisibility();

        bool isConstant();

        bool isStatic();

        bool isAbstract();

        bool isFinal();

    };
}

#endif //FLY_SEMA_BUILDERSCOPE_H
