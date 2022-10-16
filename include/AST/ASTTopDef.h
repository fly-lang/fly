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

#include "Basic/Debuggable.h"
#include "Basic/SourceLocation.h"

#include "llvm/ADT/SmallVector.h"

namespace fly {

    class SourceLocation;
    class ASTNode;
    class ASTNameSpace;

    enum class ASTTopDefKind {
        DEF_NONE,
        DEF_NAMESPACE,
        DEF_IMPORT,
        DEF_GLOBALVAR,
        DEF_FUNCTION,
        DEF_CLASS
    };

    enum class ASTVisibilityKind {
        V_DEFAULT,
        V_PUBLIC,
        V_PRIVATE
    };

    class ASTTopScopes : public Debuggable {

        friend class SemaBuilder;

        // Visibility of the declaration
        ASTVisibilityKind Visibility;

        bool Constant = false;

        ASTTopScopes(ASTVisibilityKind visibility, bool constant);

    public:
        ASTVisibilityKind getVisibility() const;

        bool isConstant() const;

        std::string str() const;
    };

    class ASTTopDef : public virtual Debuggable {

        friend class SemaBuilder;

    protected:

        ASTNode *Node = nullptr;

        ASTNameSpace *NameSpace = nullptr;

        // File Source Location
        const SourceLocation Location;

        // Kind of TopDecl identified by enum
        ASTTopDefKind Kind;

        // The TopDef Scopes
        ASTTopScopes *Scopes = nullptr;

        std::string Comment;

    protected:

        ASTTopDef(const SourceLocation &Loc, ASTNode *Node, ASTTopDefKind Kind, ASTTopScopes *Scopes);

    public:

        ASTNode *getNode();

        const SourceLocation &getLocation() const;

        virtual std::string getName() const = 0;

        ASTNameSpace *getNameSpace() const;

        ASTTopScopes *getScopes() const;

        ASTTopDefKind getKind() const;

        const std::string getComment() const;

        std::string str() const;

    };
}

#endif //FLY_AST_TOPDEF_H
