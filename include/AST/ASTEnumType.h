//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTEnumType.h - AST Enum Type header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_ENUMTYPE_H
#define FLY_AST_ENUMTYPE_H

#include "AST/ASTIdentityType.h"

namespace fly {

    class ASTEnum;

    /**
     * Enum Type
     */
    class ASTEnumType : public ASTIdentityType {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        explicit ASTEnumType(ASTIdentifier *Identifier);

        explicit ASTEnumType(ASTEnum *Def);

    public:

        ASTIdentity *getDef() const override;
    };

}

#endif //FLY_AST_ENUMTYPE_H
