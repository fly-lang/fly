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

#include "ASTTopDef.h"

namespace fly {

    class ASTIdentityType;

    class ASTIdentity : public ASTTopDef {

        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        llvm::StringRef Name;

        // Source Location
        SourceLocation Location;

        ASTIdentity(ASTNode *Node, ASTTopDefKind TopDefKind, ASTScopes *Scopes,
                    const SourceLocation &Loc, llvm::StringRef Name);

    public:

        llvm::StringRef getName() const;

        const SourceLocation &getLocation() const;

        virtual ASTIdentityType *getType() const = 0;

        virtual std::string print() const = 0;

        virtual std::string str() const = 0;

    };
}

#endif //FLY_ASTIDENTITY_H
