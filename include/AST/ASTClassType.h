//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTClassType.h - AST Class Type header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_CLASSTYPE_H
#define FLY_AST_CLASSTYPE_H

#include "AST/ASTIdentityType.h"

namespace fly {

    class ASTClass;

    /**
     * Class Type
     */
    class ASTClassType : public ASTIdentityType {

        friend class SemaBuilder;
        friend class SemaResolver;

        explicit ASTClassType(ASTIdentifier *Identifier);

        explicit ASTClassType(ASTClass *Class);

    public:

        ASTIdentity *getDef() const override;
    };

}

#endif //FLY_AST_CLASSTYPE_H
