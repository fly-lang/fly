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
#include "TypeDecl.h"
#include "Basic/TokenKinds.h"

namespace fly {

    class VarDecl : public BaseDecl {

        const ModifiableKind Modifiable;
        const TypeDecl *Type;
        const StringRef Name;

    public:
        VarDecl(const TypeDecl *Type, const StringRef Name) : Modifiable(ModifiableKind::Variable), Type(Type),
                                                                                                    Name(Name) {}

        VarDecl(const ModifiableKind Modifiable, const TypeDecl *Type, const StringRef &Name) : Modifiable(Modifiable),
                                                                                               Type(Type), Name(Name) {}

        virtual DeclKind getKind() = 0;

        const ModifiableKind &getModifiable() const {
            return Modifiable;
        }

        const TypeDecl* getType() const {
            return Type;
        }

        const llvm::StringRef &getName() const {
            return Name;
        }

        ~VarDecl() {
            delete Type;
        }
    };
}

#endif //FLY_IMPORTDECL_H
