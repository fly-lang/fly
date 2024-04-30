//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIdentityType.h - AST Identity Type header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_IDENTITYTYPE_H
#define FLY_AST_IDENTITYTYPE_H

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

        ASTIdentityTypeKind IdentityTypeKind;

        explicit ASTIdentityType(ASTIdentifier *Identifier);

        explicit ASTIdentityType(ASTIdentity *Def);

        ASTIdentityType(ASTIdentifier *Identifier, ASTIdentityTypeKind IdentityKind);

    public:

        virtual ASTIdentity *getDef() const;

        const SourceLocation &getLocation() const override;

        ASTIdentityTypeKind getIdentityTypeKind() const;

        bool operator ==(const ASTIdentityType &IdentityType) const;

        bool isNone() const;

        bool isClass() const;

        bool isEnum() const;

        std::string print() const override;

        std::string str() const override;
    };
}

#endif //FLY_AST_IDENTITYTYPE_H
