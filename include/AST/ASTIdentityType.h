//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIdentityType.h - AST Type
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTIDENTITYTYPE_H
#define FLY_ASTIDENTITYTYPE_H

#include "AST/ASTType.h"
#include "AST/ASTIdentity.h"
#include "ASTIdentifier.h"

namespace fly {

    class ASTIdentifier;

    enum class ASTIdentityTypeKind {
        TYPE_NONE = 0,
        TYPE_CLASS = 1,
        TYPE_ENUM  = 2
    };

    /**
     * Identity Type
     */
    class ASTIdentityType : public ASTIdentifier, public ASTType {

        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        ASTIdentity *Def = nullptr;

        ASTIdentityTypeKind IdentityKind;

        ASTIdentityType(ASTIdentifier *Identifier);

        ASTIdentityType(ASTIdentifier *Identifier, ASTIdentityTypeKind IdentityKind);

        ASTIdentityType(ASTIdentity *Def);

    public:

        SourceLocation getLocation() const;

        virtual ASTIdentity *getDef() const;

        ASTIdentityTypeKind getIdentityKind() const;

        bool operator ==(const ASTIdentityType &IdentityType) const;

        const bool isNone() const;

        const bool isClass() const;

        const bool isEnum() const;

        const std::string print() const override;

        std::string str() const override;
    };
}

#endif //FLY_ASTIDENTITYTYPE_H
