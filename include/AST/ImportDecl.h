//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ImportDecl.h - AST Import
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_IMPORTDECL_H
#define FLY_IMPORTDECL_H

#include "Decl.h"

namespace fly {

    class ImportDecl : public BaseDecl {

        StringRef Name;

        StringRef Alias;

    public:
        explicit ImportDecl(StringRef Name) : Name(Name) {}

        ImportDecl(StringRef Name, StringRef Alias) : Name(Name), Alias(Alias) {}

        DeclKind getKind() override {
            return DeclKind::Dependency;
        }

        const StringRef &getName() const {
            return Name;
        }

        const StringRef &getAsName() const {
            return Alias;
        }
    };
}

#endif //FLY_IMPORTDECL_H
