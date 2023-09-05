//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTTopDecl.h - Abstract Top declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_AST_TOPDEF_H
#define FLY_AST_TOPDEF_H

#include "ASTScopes.h"
#include "Basic/Debuggable.h"
#include "Basic/SourceLocation.h"

#include "llvm/ADT/SmallVector.h"

namespace fly {

    class SourceLocation;
    class ASTNode;
    class ASTNameSpace;
    class ASTScopes;

    enum class ASTTopDefKind {
        DEF_NONE,
        DEF_NAMESPACE,
        DEF_IMPORT,
        DEF_GLOBALVAR,
        DEF_FUNCTION,
        DEF_CLASS,
        DEF_ENUM
    };

    class ASTTopDef : public virtual Debuggable {

        friend class SemaBuilder;

    protected:

        ASTNode *Node = nullptr;

        // Kind of TopDecl identified by enum
        ASTTopDefKind Kind;

        // The TopDef Scopes
        ASTScopes *Scopes = nullptr;

        llvm::StringRef Comment;

    protected:

        ASTTopDef(ASTNode *Node, ASTTopDefKind Kind, ASTScopes *Scopes);

    public:

        ASTNode *getNode();

        virtual llvm::StringRef getName() const = 0;

        ASTNameSpace *getNameSpace() const;

        ASTScopes *getScopes() const;

        ASTTopDefKind getKind() const;

        llvm::StringRef getComment() const;

        std::string str() const;

    };
}

#endif //FLY_AST_TOPDEF_H
