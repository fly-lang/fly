//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTField.h - Field of a Struct
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTCLASSFIELD_H
#define FLY_ASTCLASSFIELD_H

#include "ASTTopDef.h"
#include "ASTVar.h"

namespace fly {

    class ASTClass;
    class ASTClassScopes;
    class ASTType;
    class ASTValue;
    class ASTVar;

    class ASTClassField : public ASTVar {

        friend class SemaBuilder;
        friend class SemaResolver;

        const SourceLocation &Loc;

        const ASTClass *Class = nullptr;

        ASTClassScopes *Scopes = nullptr;

        const ASTValue *Value = nullptr;

        ASTClassField(const SourceLocation &Loc, ASTClass *Class, ASTClassScopes *Scopes, ASTType *Type,
                      std::string &Name);

    public:

        const SourceLocation &getLocation() const;

        const ASTClass *getClass() const;

        const ASTValue *getValue() const;

        virtual std::string str() const;
    };
}

#endif //FLY_ASTCLASSFIELD_H
