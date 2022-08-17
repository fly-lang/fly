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

    const uint8_t   MIN_BYTE     = 0x0;
    const uint8_t   MAX_BYTE     = 0xFF;
    const uint16_t  MIN_USHORT   = 0x0;
    const uint16_t  MAX_USHORT   = 0xFFFF;
    const uint16_t  MIN_SHORT    = 0x8000;
    const uint16_t  MAX_SHORT    = 0x7FFF;
    const uint32_t  MIN_UINT     = 0x0;
    const uint32_t  MAX_UINT     = 0xFFFFFFFF;
    const uint32_t  MIN_INT      = 0x80000000;
    const uint32_t  MAX_INT      = 0x7FFFFFFF;
    const uint64_t  MIN_ULONG    = 0x0;
    const uint64_t  MAX_ULONG    = 0xFFFFFFFFFFFFFFFF;
    const uint64_t  MIN_LONG     = 0x8000000000000000;
    const uint64_t  MAX_LONG     = 0x7FFFFFFFFFFFFFFF;

    enum TypeKind {
        TYPE_VOID = 0,
        TYPE_BOOL = 1,
        TYPE_BYTE = 2,
        TYPE_USHORT = 3,
        TYPE_SHORT = 4,
        TYPE_UINT = 5,
        TYPE_INT = 6,
        TYPE_ULONG = 7,
        TYPE_LONG = 8,
        TYPE_FLOAT = 9,
        TYPE_DOUBLE = 10,
        TYPE_ARRAY = 11,
        TYPE_CLASS = 12
    };

    class ASTIntegerValue;
    class ASTValueExpr;
    class ASTExpr;
    class ASTClass;

    /**
     * Abstract Base Type
     */
    class ASTType {

        const TypeKind Kind;

        const SourceLocation Loc;

    protected:
        ASTType(const SourceLocation &Loc, TypeKind Kind);

    public:
        const TypeKind &getKind() const;

        const SourceLocation &getLocation() const;

        const bool isBool() const;

        const bool isNumber() const;

        const bool isInteger() const;

        const bool isFloatingPoint() const;

        const bool isClass() const;

        const bool isArray() const;

        virtual ~ASTType() = default;

        virtual std::string str() const = 0;
    };

    /**
     * Void Type
     */
    class ASTVoidType : public ASTType {

        friend class SemaBuilder;

        explicit ASTVoidType(const SourceLocation &Loc);

    public:

        std::string str() const override {
            return "void";
        }
    };

    /**
     * Boolean Type
     */
    class ASTBoolType : public ASTType {

        friend class SemaBuilder;

        explicit ASTBoolType(const SourceLocation &Loc);

    public:

        std::string str() const override {
            return "bool";
        }
    };

    /**
     * Byte Type
     */
    class ASTByteType : public ASTType {

        friend class SemaBuilder;

        explicit ASTByteType(const SourceLocation &Loc);

    public:

        std::string str() const override {
            return "byte";
        }
    };

    /**
     * Unsigned Short Type
     */
    class ASTUShortType : public ASTType {

        friend class SemaBuilder;

        explicit ASTUShortType(const SourceLocation &Loc);

    public:

        std::string str() const override {
            return "ushort";
        }
    };

    /**
     * Short Type
     */
    class ASTShortType : public ASTType {

        friend class SemaBuilder;

        explicit ASTShortType(const SourceLocation &Loc);

    public:

        std::string str() const override {
            return "short";
        }
    };

    /**
     * Unsigned Int Type
     */
    class ASTUIntType : public ASTType {

        friend class SemaBuilder;

        explicit ASTUIntType(const SourceLocation &Loc);

    public:

        std::string str() const override {
            return "uint";
        }
    };

    /**
     * Int Type
     */
    class ASTIntType : public ASTType {

        friend class SemaBuilder;

        explicit ASTIntType(const SourceLocation &Loc);

    public:

        std::string str() const override {
            return "int";
        }
    };

    /**
     * Long Type
     */
    class ASTULongType : public ASTType {

        friend class SemaBuilder;

        explicit ASTULongType(const SourceLocation &Loc);

    public:

        std::string str() const override {
            return "ulong";
        }
    };

    /**
     * Long Type
     */
    class ASTLongType : public ASTType {

        friend class SemaBuilder;

        explicit ASTLongType(const SourceLocation &Loc);

    public:

        std::string str() const override {
            return "long";
        }
    };

    /**
     * Float Type
     */
    class ASTFloatType : public ASTType {

        friend class SemaBuilder;

        explicit ASTFloatType(const SourceLocation &Loc);

    public:

        std::string str() const override {
            return "float";
        }
    };

    /**
     * Long Type
     */
    class ASTDoubleType : public ASTType {

        friend class SemaBuilder;

        explicit ASTDoubleType(const SourceLocation &Loc);

    public:

        std::string str() const override {
            return "double";
        }
    };

    /**
     * String Type
     */
    class ASTArrayType : public ASTType {

        friend class SemaBuilder;

        ASTExpr *Size;

        ASTType *Type;

        ASTArrayType(const SourceLocation &Loc, ASTType *Type, ASTExpr *Size);

    public:

        ASTExpr *getSize() const;

        ASTType *getType() const;

        std::string str() const override;
    };

    /**
     * Class Type
     */
    class ASTClassType : public ASTType {

        friend class SemaBuilder;

        const TypeKind Kind = TypeKind::TYPE_CLASS;

        std::string Name;

        std::string NameSpace;

        ASTClass *Def;

        ASTClassType(const SourceLocation &Loc, std::string Name, std::string NameSpace = "");

    public:

        const std::string &getName() const;

        const std::string &getNameSpace() const;

        ASTClass *getDef() const;

        bool operator ==(const ASTClassType &Ty) const;

        std::string str() const override;
    };
}

#endif //FLY_ASTTYPE_H
