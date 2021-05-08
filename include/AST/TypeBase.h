//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/TypeDecl.h - Type declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_TYPEBASE_H
#define FLY_TYPEBASE_H

#include "Decl.h"

namespace fly {

    enum TypeKind {
        TYPE_VOID = 1,
        TYPE_INT = 2,
        TYPE_FLOAT = 3,
        TYPE_BOOL = 4,
        TYPE_CLASS = 5,
    };

    /**
     * Abstract Base Type
     */
    class TypeBase {

    protected:
        TypeBase(SourceLocation Loc) : Loc(Loc) {}
        const SourceLocation Loc;

    public:
        virtual const TypeKind &getKind() const = 0;

        const SourceLocation &getLoc() const {
            return Loc;
        }

        virtual ~TypeBase() = default;
    };

    /**
     * Int Type
     */
    class IntPrimType : public TypeBase {

        const TypeKind Kind = TypeKind::TYPE_INT;

    public:
        IntPrimType(SourceLocation Loc) : TypeBase(Loc) {}

        const TypeKind &getKind() const override {
            return Kind;
        }
    };

    /**
     * Float Type
     */
    class FloatPrimType : public TypeBase {

        const TypeKind Kind = TypeKind::TYPE_FLOAT;

    public:
        FloatPrimType(SourceLocation Loc) : TypeBase(Loc) {}

        const TypeKind &getKind() const override {
            return Kind;
        }
    };

    /**
     * Boolean Type
     */
    class BoolPrimType : public TypeBase {

        const TypeKind Kind = TypeKind::TYPE_BOOL;

    public:
        BoolPrimType(SourceLocation Loc) : TypeBase(Loc) {}

        const TypeKind &getKind() const override {
            return Kind;
        }
    };

    /**
     * Void Type
     */
    class VoidRetType : public TypeBase {

        const TypeKind Kind = TypeKind::TYPE_VOID;

    public:
        VoidRetType(SourceLocation Loc) : TypeBase(Loc) {}

        const TypeKind &getKind() const override {
            return Kind;
        }
    };

    /**
     * Custom Type
     */
    class ClassTypeRef : public Refer, public TypeBase {

        const TypeKind Kind = TypeKind::TYPE_CLASS;
        const StringRef Name;
        Decl *D = NULL;

    public:
        ClassTypeRef(SourceLocation Loc, StringRef &Name) : Refer(Loc), TypeBase(Loc), Name(Name) {}

        const TypeKind &getKind() const override {
            return Kind;
        }

        const StringRef &getName() const {
            return Name;
        }

        Decl *getDecl() const override {
            return D;
        }
    };
}

#endif //FLY_TYPEBASE_H
