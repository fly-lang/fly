//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTTopDecl.h - Abstract Top declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTTOPDECL_H
#define FLY_ASTTOPDECL_H

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

    class ASTTopDecl {

        friend class SemaBuilder;

    protected:
        ASTNode *Node;

        ASTNameSpace *NameSpace;

        // File Source Location
        const SourceLocation Location;

        // Visibility of the declaration
        VisibilityKind Visibility = V_DEFAULT;

        // Kind of TopDecl identified by enum
        TopDeclKind Kind;

        std::string Comment;

    public:
        ASTTopDecl(const SourceLocation &Loc, ASTNode *Node, TopDeclKind Kind,  VisibilityKind Visibility);

        ASTNode *getNode();

        virtual const std::string &getName() const = 0;

        ASTNameSpace *getNameSpace() const;

        const SourceLocation &getLocation() const;

        VisibilityKind getVisibility() const;

        void setVisibility(VisibilityKind V);

        TopDeclKind getKind() const;

        const std::string &getComment() const;

        virtual std::string str() const;
    };
}

#endif //FLY_ASTTOPDECL_H
