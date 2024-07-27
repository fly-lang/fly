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
#include "AST/ASTScopes.h"

namespace fly {

    class ASTIdentityType;

    class ASTIdentity : public ASTBase, public virtual ASTTopDef {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

    protected:

        ASTTopDefKind TopDefKind;

        llvm::SmallVector<ASTScope *, 8> Scopes;

        llvm::StringRef Name;

        ASTVisibilityKind Visibility;

        ASTModule *Module = nullptr;

        ASTIdentityType *Type = nullptr;

        ASTIdentity(ASTModule *Module, ASTTopDefKind TopDefKind, llvm::SmallVector<ASTScope *, 8> &Scopes,
                    const SourceLocation &Loc, llvm::StringRef Name);

    public:

        ASTTopDefKind getTopDefKind() const override;

        ASTIdentityType *getType() const;

        llvm::StringRef getName() const override;

        ASTVisibilityKind getVisibility() const;

        llvm::SmallVector<ASTScope *, 8> getScopes() const;

        ASTModule *getModule() const override;

        ASTNameSpace *getNameSpace() const override;

        virtual std::string str() const override;

    };
}

#endif //FLY_AST_IDENTITY_H
