//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIdentity.h - AST Identity header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_IDENTITY_H
#define FLY_AST_IDENTITY_H

#include "AST/ASTBase.h"
#include "ASTTopDef.h"

namespace fly {

    class ASTIdentityType;

    class ASTIdentity : public ASTBase, public virtual ASTTopDef {

        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        ASTTopDefKind TopDefKind;

        ASTScopes *Scopes;

        llvm::StringRef Name;

        ASTModule *Module = nullptr;

        ASTIdentityType *Type = nullptr;

        ASTIdentity(ASTTopDefKind TopDefKind, ASTScopes *Scopes,
                    const SourceLocation &Loc, llvm::StringRef Name);

    public:

        ASTTopDefKind getTopDefKind() const override;

        llvm::StringRef getName() const override;

        ASTScopes *getScopes() const;

        ASTModule *getModule() const override;

        ASTNameSpace *getNameSpace() const override;

        virtual std::string print() const = 0;

        virtual std::string str() const override;

    };
}

#endif //FLY_AST_IDENTITY_H
