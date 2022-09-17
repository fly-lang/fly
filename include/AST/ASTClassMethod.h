//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTClassMethod.h - Class Method of a Struct
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTCLASSMETHODD_H
#define FLY_ASTCLASSMETHODD_H

#include "ASTFunctionBase.h"

namespace fly {

    class ASTClass;
    class ASTClassScopes;
    class ASTType;
    class ASTFunction;

    class ASTClassMethod : public ASTFunctionBase {

        friend class SemaBuilder;
        friend class SemaResolver;

        const SourceLocation &Loc;

        const ASTClass *Class = nullptr;

        ASTClassScopes *Scopes = nullptr;

        ASTClassMethod(const SourceLocation &Loc, ASTClass *Class, ASTClassScopes *Scopes, ASTType *Type,
                       std::string &Name);

    public:

        const SourceLocation &getLocation() const;

        const ASTClass *getClass() const;

        virtual std::string str() const;

    };
}

#endif //FLY_ASTCLASSMETHODD_H
