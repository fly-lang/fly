//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ImportDecl.h - AST Import declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_IMPORTDECL_H
#define FLY_IMPORTDECL_H

#include "Decl.h"
#include "ASTNameSpace.h"

namespace fly {

    class ASTNameSpace;

    class ImportDecl : public DeclBase {

        StringRef Name;

        StringRef Alias;

        ASTNameSpace *NameSpace = nullptr;

    public:

        ImportDecl(const SourceLocation &Loc, StringRef Name) : DeclBase(Loc), Name(Name), Alias(Name) {}

        ImportDecl(const SourceLocation &Loc, StringRef Name, StringRef Alias) : DeclBase(Loc), Name(Name),
                                                                                 Alias(Alias) {}

        ~ImportDecl() {
            NameSpace = nullptr;
        }

        DeclKind getKind() override {
            return DeclKind::D_DEPENDENCY;
        }

        const StringRef &getName() const {
            return Name;
        }

        const StringRef &getAlias() const {
            return Alias;
        }

        ASTNameSpace *getNameSpace() const {
            return NameSpace;
        }

        void setNameSpace(ASTNameSpace *NS) {
            NameSpace = NS;
        }
    };
}

#endif //FLY_IMPORTDECL_H
