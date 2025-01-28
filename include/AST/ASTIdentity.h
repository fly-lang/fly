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

namespace fly {

    class ASTModule;
    class ASTNameSpace;
    class ASTComment;
    class ASTIdentityType;
    class ASTScope;
    enum class ASTVisibilityKind;

    enum class ASTIdentityKind {
        ID_CLASS,
        ID_ENUM
    };

    class ASTIdentity : public ASTBase {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        llvm::SmallVector<ASTBase *, 8> Definitions;

    protected:

        llvm::SmallVector<ASTScope *, 8> Scopes;

        llvm::StringRef Name;

        ASTIdentityKind IdentityKind;

        ASTVisibilityKind Visibility;

        ASTModule *Module;

        ASTIdentityType *Type = nullptr;

        ASTIdentity(ASTModule *Module, ASTIdentityKind IdentityKind, llvm::SmallVector<ASTScope *, 8> &Scopes,
                    const SourceLocation &Loc, llvm::StringRef Name);

    public:

        ASTIdentityType *getType() const;

        ASTIdentityKind getIdentityKind() const;

        llvm::StringRef getName() const;

        ASTVisibilityKind getVisibility() const;

        llvm::SmallVector<ASTScope *, 8> getScopes() const;

        ASTNameSpace *getNameSpace() const;

        virtual std::string str() const override;

    };
}

#endif //FLY_AST_IDENTITY_H
