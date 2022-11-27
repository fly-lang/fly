//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTClassMethod.h - The Method in a Class
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

    class ASTClassFunction : public ASTFunctionBase {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTClass *Class = nullptr;

        llvm::StringRef Comment;

        ASTClassScopes *Scopes = nullptr;

        ASTClassFunction(const SourceLocation &Loc, ASTClass *Class, ASTClassScopes *Scopes, ASTType *Type,
                         llvm::StringRef Name);

    public:

        ASTClass *getClass() const;

        llvm::StringRef getComment() const;

        ASTClassScopes *getScopes() const;

        virtual std::string str() const;

    };
}

#endif //FLY_ASTCLASSMETHODD_H
