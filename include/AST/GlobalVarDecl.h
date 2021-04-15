//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/PackageVarDecl.h - AST Unit
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

        bool External;

    public:
        GlobalVarDecl(TypeKind Type, StringRef Name) : External(false), VarDecl(Type, Name) {

        }

        DeclKind getKind() override {
            return DeclKind::GlobalVar;
        }

        void setExternal(bool external) {
            GlobalVarDecl::External = external;
        }

        bool isExternal() const {
            return External;
        }
    };
}

#endif //FLY_PACKAGEVAR_H
