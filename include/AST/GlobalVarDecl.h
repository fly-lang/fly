//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/GlobalVarDecl.h - Global Var declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_PACKAGEVAR_H
#define FLY_PACKAGEVAR_H

#include "VarDecl.h"

namespace fly {

    class GlobalVarDecl : public VarDecl, public TopDecl {

        friend class ASTNode;
        const DeclKind Kind = DeclKind::D_GLOBALVAR;

    public:

        GlobalVarDecl(SourceLocation &Loc, TypeBase *Type, StringRef Name) : VarDecl(Loc, Type, Name) {}

        DeclKind getKind() const override {
            return Kind;
        }

        ~GlobalVarDecl() = default;
    };
}

#endif //FLY_PACKAGEVAR_H
