//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTModifier.h - AST Modifiers header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_MODIFIER_H
#define FLY_AST_MODIFIER_H

#include "ASTBase.h"

namespace fly {

    enum class ASTModifierKind {
        M_VISIBILITY,
        M_CONSTANT,
        M_STATIC
    };

    enum class ASTVisibilityKind {
        V_DEFAULT,
        V_PUBLIC,
        V_PRIVATE,
        V_PROTECTED
    };

    class ASTModifier : public ASTBase {

        friend class ASTBuilder;
        friend class SemaBuilderModifiers;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTModifierKind Kind;

        ASTVisibilityKind Visibility;

        bool Constant;

        bool Static;

        ASTModifier(const SourceLocation &Loc, ASTModifierKind Kind);

    public:

        ASTModifierKind getModifierKind();

        ASTVisibilityKind getVisibility() const;

        bool isConstant() const;

        bool isStatic() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_MODIFIER_H
