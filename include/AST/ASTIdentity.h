//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTClass.h - Class declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTIDENTITY_H
#define FLY_ASTIDENTITY_H

#include "AST/ASTBase.h"
#include "ASTTopDef.h"

namespace fly {

    class ASTIdentityType;

    class ASTIdentity : public ASTBase, public virtual ASTTopDef {

        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        ASTTopDefKind TopDefKind;

        ASTNode *Node;

        ASTScopes *Scopes;

        llvm::StringRef Name;

        ASTIdentity(ASTNode *Node, ASTTopDefKind TopDefKind, ASTScopes *Scopes,
                    const SourceLocation &Loc, llvm::StringRef Name);

    public:

        ASTTopDefKind getTopDefKind() const override;

        ASTNode *getNode() const override;

        ASTNameSpace *getNameSpace() const override;

        llvm::StringRef getName() const override;

        ASTScopes *getScopes() const;

        virtual ASTIdentityType *getType() = 0;

        virtual std::string print() const = 0;

        virtual std::string str() const;

    };
}

#endif //FLY_ASTIDENTITY_H
