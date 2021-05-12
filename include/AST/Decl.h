//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/Decl.h - Abstract Base declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
/// \file
/// Defines the fly::BaseDecl with DeclKind enum.
///
//===--------------------------------------------------------------------------------------------------------------===//
#ifndef FLY_DECL_H
#define FLY_DECL_H

#include "Basic/SourceLocation.h"
#include "Basic/LLVM.h"
#include "llvm/ADT/StringRef.h"

namespace fly {

    enum DeclKind {
        D_NAMESPACE,
        D_DEPENDENCY,
        D_GLOBALVAR,
        D_FUNCTION,
        R_FUNCTION,
        D_VAR,
        R_VAR,
        D_STMT,
        D_TYPE,
        D_BREAK,
        D_CONTINUE,
        D_RETURN
    };

    enum VisibilityKind {
        V_DEFAULT = 1,
        V_PUBLIC = 2,
        V_PRIVATE = 3
    };

    class Decl {

        const SourceLocation Location;

    public:
        explicit Decl(const SourceLocation &Loc);

        const SourceLocation &getLocation() const;

        virtual DeclKind getKind() const = 0;
    };

    class ASTNode; // Pre-declare

    class TopDecl {

        friend class ASTNode;
        friend class Parser;

        VisibilityKind Visibility;

    public:
        VisibilityKind getVisibility() const;
    };
}


#endif //FLY_DECL_H
