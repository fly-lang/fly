//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/TopDecl.h - Abstract Top declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_TOPDECL_H
#define FLY_TOPDECL_H

#include "Basic/SourceLocation.h"

namespace fly {

    class ASTNode;
    enum TopDeclKind {
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

    class TopDecl {

        friend class ASTNode;
        friend class Parser;

    protected:
        ASTNode *Node;
        const SourceLocation Location;
        VisibilityKind Visibility;

    public:
        TopDecl(ASTNode *Node, const SourceLocation &Loc);

        ASTNode *getNode();

        const ASTNameSpace &getNameSpace() const;

        const SourceLocation &getLocation() const;

        VisibilityKind getVisibility() const;

        void setVisibility(VisibilityKind V);

        virtual TopDeclKind getKind() const = 0;

    };
}

#endif //FLY_TOPDECL_H
