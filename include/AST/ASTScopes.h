//
// Created by marco on 3/9/23.
//

#ifndef FLY_ASTSCOPES_H
#define FLY_ASTSCOPES_H

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

        ASTScopes(const SourceLocation &Loc);

    public:
        ASTVisibilityKind getVisibility() const;

        void setVisibility(ASTVisibilityKind visibility);

        bool isConstant() const;

        void setConstant(bool constant);

        bool isStatic() const;

        void setStatic(bool S);

        std::string str() const;
    };
}

#endif //FLY_ASTSCOPES_H
