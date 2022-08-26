//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTTopDecl.h - Abstract Top declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTTOPDEF_H
#define FLY_ASTTOPDEF_H

#include "Basic/SourceLocation.h"
#include "llvm/ADT/SmallVector.h"

namespace fly {

    class ASTNode;

    enum TopDeclKind {
        DECL_NONE,
        DECL_NAMESPACE,
        DECL_IMPORT,
        DECL_GLOBALVAR,
        DECL_FUNCTION,
        DECL_CLASS
    };

    class ASTNameSpace;

    enum VisibilityKind {
        V_DEFAULT = 1,
        V_PUBLIC = 2,
        V_PRIVATE = 3
    };

    class ASTTopDef {

        friend class SemaBuilder;

    protected:
        ASTNode *Node = nullptr;

        ASTNameSpace *NameSpace = nullptr;

        // File Source Location
        const SourceLocation Location;

        // Visibility of the declaration
        VisibilityKind Visibility = V_DEFAULT;

        // Kind of TopDecl identified by enum
        TopDeclKind Kind;

        std::string Comment;

    protected:

        ASTTopDef(const SourceLocation &Loc, ASTNode *Node, TopDeclKind Kind, VisibilityKind Visibility);

    public:

        ASTNode *getNode();

        virtual const std::string &getName() const = 0;

        ASTNameSpace *getNameSpace() const;

        const SourceLocation &getLocation() const;

        VisibilityKind getVisibility() const;

        TopDeclKind getKind() const;

        const std::string &getComment() const;

        virtual std::string str() const;
    };
}

#endif //FLY_ASTTOPDEF_H
