//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/Decl.h - AST Types enum
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

#include "Basic/LLVM.h"
#include "llvm/ADT/StringRef.h"

namespace fly {

    enum DeclKind {
        NameSpace,
        Dependency,
        GlobalVar,
        Function,
        Type
    };

    enum VisibilityKind {
        Default,
        Public,
        Private
    };

    enum ModifiableKind {
        Variable,
        Constant
    };

    enum TypeKind {
        Int,
        Float,
        Boolean,
    };

    class BaseDecl {
    public:
        virtual DeclKind getKind() = 0;
    };
}


#endif //FLY_DECL_H
