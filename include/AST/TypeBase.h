//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/TypeBase.h - Base Type
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_TYPEBASE_H
#define FLY_TYPEBASE_H

#include "Basic/SourceLocation.h"
#include "llvm/ADT/StringRef.h"

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
        TypeBase(SourceLocation Loc);
        const SourceLocation Loc;

    public:
        virtual const TypeKind &getKind() const = 0;

        const SourceLocation &getLocation() const;

        virtual ~TypeBase() = default;

        virtual bool operator ==(const TypeBase &Ty) const;
    };

    /**
     * Int Type
     */
    class IntPrimType : public TypeBase {

        const TypeKind Kind = TypeKind::TYPE_INT;

    public:
        IntPrimType(SourceLocation Loc);

        const TypeKind &getKind() const override;
    };

    /**
     * Float Type
     */
    class FloatPrimType : public TypeBase {

        const TypeKind Kind = TypeKind::TYPE_FLOAT;

    public:
        FloatPrimType(SourceLocation Loc);

        const TypeKind &getKind() const override;
    };

    /**
     * Boolean Type
     */
    class BoolPrimType : public TypeBase {

        const TypeKind Kind = TypeKind::TYPE_BOOL;

    public:
        BoolPrimType(SourceLocation Loc);

        const TypeKind &getKind() const override;
    };

    /**
     * Void Type
     */
    class VoidRetType : public TypeBase {

        const TypeKind Kind = TypeKind::TYPE_VOID;

    public:
        VoidRetType(SourceLocation Loc);

        const TypeKind &getKind() const override;
    };

    /**
     * Custom Type
     */
    class ClassTypeRef : public TypeBase {

        const TypeKind Kind = TypeKind::TYPE_CLASS;
        const llvm::StringRef Name;

    public:
        ClassTypeRef(SourceLocation Loc, llvm::StringRef &Name);

        const TypeKind &getKind() const override;

        const llvm::StringRef &getName() const;

        bool operator ==(const ClassTypeRef &Ty) const;
    };
}

#endif //FLY_TYPEBASE_H
