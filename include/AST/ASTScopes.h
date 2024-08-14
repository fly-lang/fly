//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTScopes.h - AST Scopes header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_SCOPES_H
#define FLY_AST_SCOPES_H

#include "ASTBase.h"

namespace fly {

    enum class ASTScopeKind {
        SCOPE_VISIBILITY,
        SCOPE_CONSTANT,
        SCOPE_STATIC
    };

    enum class ASTVisibilityKind {
        V_DEFAULT,
        V_PUBLIC,
        V_PRIVATE,
        V_PROTECTED
    };

    class ASTScope : public ASTBase {

        friend class SemaBuilder;
        friend class SemaBuilderScopes;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTScopeKind Kind;

        ASTVisibilityKind Visibility;

        bool Constant;

        bool Static;

        ASTScope(const SourceLocation &Loc, ASTScopeKind Kind);

    public:

        ASTScopeKind getKind();

        ASTVisibilityKind getVisibility() const;

        bool isConstant() const;

        bool isStatic() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_SCOPES_H
