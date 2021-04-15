//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/VarDecl.h - AST Variable
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_VARDECL_H
#define FLY_VARDECL_H

#include "Decl.h"
#include "Basic/TokenKinds.h"

namespace fly {

    class VarDecl : public BaseDecl {

        const TypeKind Type;
        const StringRef Name;

    public:
        VarDecl(TypeKind Type, StringRef Name) : Type(Type), Name(Name) {}

        virtual DeclKind getKind() = 0;

        const TypeKind &getType() const {
            return Type;
        }

        const llvm::StringRef &getName() const {
            return Name;
        }
    };
}

#endif //FLY_IMPORTDECL_H
