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

    class GlobalVarDecl : public VarDecl {
        friend ASTNode;

        VisibilityKind Visibility;

    public:

        GlobalVarDecl(const SourceLocation &Loc, ModifiableKind Modifiable, TypeDecl *Type, StringRef &Name) :
            VarDecl(Loc, Modifiable, Type, Name) {}

        DeclKind getKind() override {
            return DeclKind::GlobalVar;
        }

        VisibilityKind getVisibility() const {
            return Visibility;
        }

        void setVisibility(VisibilityKind visibility) {
            Visibility = visibility;
        }

        ~GlobalVarDecl() = default;
    };
}

#endif //FLY_PACKAGEVAR_H
