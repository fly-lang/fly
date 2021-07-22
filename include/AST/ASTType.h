//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTType.h - AST Type
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTTYPE_H
#define FLY_ASTTYPE_H

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
    class ASTType {

    protected:
        ASTType(SourceLocation Loc);
        const SourceLocation Loc;

    public:
        virtual const TypeKind &getKind() const = 0;

        const SourceLocation &getLocation() const;

        virtual ~ASTType() = default;

        virtual std::string str() const = 0;

        virtual bool equals(ASTType *Ty) const;
    };

    /**
     * Int Type
     */
    class IntPrimType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_INT;

    public:
        IntPrimType(SourceLocation Loc);

        const TypeKind &getKind() const override;

        std::string str() const override {
            return "int";
        }
    };

    /**
     * Float Type
     */
    class FloatPrimType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_FLOAT;

    public:
        FloatPrimType(SourceLocation Loc);

        const TypeKind &getKind() const override;

        std::string str() const override {
            return "float";
        }
    };

    /**
     * Boolean Type
     */
    class BoolPrimType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_BOOL;

    public:
        BoolPrimType(SourceLocation Loc);

        const TypeKind &getKind() const override;

        std::string str() const override {
            return "bool";
        }
    };

    /**
     * Void Type
     */
    class VoidRetType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_VOID;

    public:
        VoidRetType(SourceLocation Loc);

        const TypeKind &getKind() const override;

        std::string str() const override {
            return "void";
        }
    };

    /**
     * Custom Type
     */
    class ClassTypeRef : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_CLASS;
        const llvm::StringRef Name;

    public:
        ClassTypeRef(SourceLocation Loc, llvm::StringRef &Name);

        const TypeKind &getKind() const override;

        const llvm::StringRef &getName() const;

        bool operator ==(const ClassTypeRef &Ty) const;

        std::string str() const override {
            return Name.str();
        }
    };
}

#endif //FLY_ASTTYPE_H
