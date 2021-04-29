//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/TypeDecl.h - Type declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_TYPEDECL_H
#define FLY_TYPEDECL_H

#include "Decl.h"

namespace fly {

    enum TypeKind {
        TYPE_NONE,
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_BOOL,
        TYPE_VOID,
    };

    /**
     * Abstract Base Type
     */
    class TypeDecl {

    protected:
        TypeDecl(SourceLocation Loc) : Loc(Loc) {}
        const SourceLocation Loc;

    public:
        const virtual TypeKind &getKind() const = 0;

        const SourceLocation &getLoc() const {
            return Loc;
        }

        virtual ~TypeDecl() = default;
    };

    /**
     * Int Type
     */
    class IntTypeDecl : public TypeDecl {

        TypeKind Kind = TypeKind::TYPE_INT;

    public:
        IntTypeDecl(SourceLocation Loc) : TypeDecl(Loc) {}

        const TypeKind &getKind() const override {
            return Kind;
        }
    };

    /**
     * Float Type
     */
    class FloatTypeDecl : public TypeDecl {

        TypeKind Kind = TypeKind::TYPE_FLOAT;

    public:
        FloatTypeDecl(SourceLocation Loc) : TypeDecl(Loc) {}

        const TypeKind &getKind() const override {
            return Kind;
        }
    };

    /**
     * Boolean Type
     */
    class BoolTypeDecl : public TypeDecl {

        TypeKind Kind = TypeKind::TYPE_BOOL;

    public:
        BoolTypeDecl(SourceLocation Loc) : TypeDecl(Loc) {}

        const TypeKind &getKind() const override {
            return Kind;
        }
    };

    /**
     * Void Type
     */
    class VoidTypeDecl : public TypeDecl {

        TypeKind Kind = TypeKind::TYPE_VOID;

    public:
        VoidTypeDecl(SourceLocation Loc) : TypeDecl(Loc) {}

        const TypeKind &getKind() const override {
            return Kind;
        }
    };
}

#endif //FLY_TYPEDECL_H
