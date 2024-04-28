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

    enum class ASTVisibilityKind {
        V_DEFAULT,
        V_PUBLIC,
        V_PRIVATE,
        V_PROTECTED
    };

    class ASTScopes : public ASTBase {

        friend class SemaBuilder;

        // Visibility of the Fields or Methods
        ASTVisibilityKind Visibility = ASTVisibilityKind::V_DEFAULT;

        // Constant Fields or Methods
        bool Constant = false;

        // Static Fields or Methods
        bool Static = false;

        explicit ASTScopes(const SourceLocation &Loc);

    public:
        ASTVisibilityKind getVisibility() const;

        void setVisibility(ASTVisibilityKind visibility);

        bool isConstant() const;

        void setConstant(bool constant);

        bool isStatic() const;

        void setStatic(bool S);

        std::string str() const override;
    };
}

#endif //FLY_AST_SCOPES_H
