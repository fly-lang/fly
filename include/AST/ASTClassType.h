//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTClassType.h - AST Type
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTCLASSTYPE_H
#define FLY_ASTCLASSTYPE_H

#include "AST/ASTIdentityType.h"

namespace fly {

    class ASTClass;

    /**
     * Class Type
     */
    class ASTClassType : public ASTIdentityType {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTClassType(ASTIdentifier *Identifier);

        ASTClassType(ASTClass *Class);

    public:

        ASTIdentity *getDef() const override;
    };

}

#endif //FLY_ASTCLASSTYPE_H
