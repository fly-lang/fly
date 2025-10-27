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

#include "ASTNode.h"

namespace fly {

    enum class ASTModifierKind {
        MOD_CONSTANT,
        MOD_STATIC,
        MOD_PUBLIC,
        MOD_PRIVATE,
        MOD_PROTECTED,
        MOD_DEFAULT
    };

    class ASTModifier : public ASTNode {

        friend class ASTBuilder;
        friend class SemaBuilderModifiers;
        friend class Resolver;
        friend class SemaValidator;

        ASTModifierKind Kind;

        ASTModifier(const SourceLocation &Loc, ASTModifierKind Kind);

    public:

        ASTModifierKind getModifierKind();

        std::string str() const override;
    };
}

#endif //FLY_AST_MODIFIER_H
