//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTEnumType.h - AST Type
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTENUMTYPE_H
#define FLY_ASTENUMTYPE_H

#include "AST/ASTIdentityType.h"

namespace fly {

    class ASTEnum;

    /**
     * Enum Type
     */
    class ASTEnumType : public ASTIdentityType {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTEnumType(ASTIdentifier *Identifier);

        ASTEnumType(ASTEnum *Def);

    public:

        ASTIdentity *getDef() const override;
    };

}

#endif //FLY_ASTENUMTYPE_H
