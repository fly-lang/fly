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

        virtual bool equals(ASTType *Ty) const;

        virtual std::string str() const = 0;
    };

    /**
     * Int Type
     */
    class ASTIntType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_INT;

    public:
        ASTIntType(SourceLocation Loc);

        const TypeKind &getKind() const override;

        std::string str() const override {
            return "{int}";
        }
    };

    /**
     * Float Type
     */
    class ASTFloatType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_FLOAT;

    public:
        ASTFloatType(SourceLocation Loc);

        const TypeKind &getKind() const override;

        std::string str() const override {
            return "{float}";
        }
    };

    /**
     * Boolean Type
     */
    class ASTBoolType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_BOOL;

    public:
        ASTBoolType(SourceLocation Loc);

        const TypeKind &getKind() const override;

        std::string str() const override {
            return "{bool}";
        }
    };

    /**
     * Void Type
     */
    class ASTVoidType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_VOID;

    public:
        ASTVoidType(SourceLocation Loc);

        const TypeKind &getKind() const override;

        std::string str() const override {
            return "{void}";
        }
    };

    /**
     * Custom Type
     */
    class ASTClassType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_CLASS;
        const std::string Name;
        const std::string NameSpace;

    public:
        ASTClassType(SourceLocation Loc, std::string Name, std::string NameSpace = "");

        const TypeKind &getKind() const override;

        const std::string &getName() const;

        bool operator ==(const ASTClassType &Ty) const;

        std::string str() const override {
            return Name;
        }
    };
}

#endif //FLY_ASTTYPE_H
