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

    const uint8_t   MAX_BYTE     = 0xFF;
    const uint16_t  MAX_USHORT   = 0xFFFF;
    const uint16_t  MAX_SHORT    = 0x7FFF;
    const uint16_t  MIN_SHORT    = 0x8000;
    const uint32_t  MAX_UINT     = 0xFFFFFFFF;
    const uint32_t  MAX_INT      = 0x7FFFFFFF;
    const uint32_t  MIN_INT      = 0x80000000;
    const uint64_t  MAX_ULONG    = 0xFFFFFFFFFFFFFFFF;
    const uint64_t  MAX_LONG     = 0x7FFFFFFFFFFFFFFF;
    const uint64_t  MIN_LONG     = 0x8000000000000000;

    enum TypeKind {
        TYPE_VOID,
        TYPE_BOOL,
        TYPE_BYTE,
        TYPE_USHORT,
        TYPE_SHORT,
        TYPE_UINT,
        TYPE_INT,
        TYPE_ULONG,
        TYPE_LONG,
        TYPE_FLOAT,
        TYPE_DOUBLE,
        TYPE_CLASS,
    };

    /**
     * Abstract Base Type
     */
    class ASTType {

        const TypeKind Kind;
        const SourceLocation Loc;

    protected:
        ASTType(SourceLocation Loc, TypeKind Kind);

    public:
        const TypeKind &getKind() const;

        const SourceLocation &getLocation() const;

        virtual ~ASTType() = default;

        virtual bool equals(ASTType *Ty) const;

        virtual std::string str() const = 0;
    };

    /**
     * Boolean Type
     */
    class ASTBoolType : public ASTType {

    public:
        explicit ASTBoolType(SourceLocation Loc);

        std::string str() const override {
            return "{bool}";
        }
    };

    /**
     * Byte Type
     */
    class ASTByteType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_BYTE;

    public:
        explicit ASTByteType(SourceLocation Loc);

        std::string str() const override {
            return "{byte}";
        }
    };

    /**
     * Unsigned Short Type
     */
    class ASTUShortType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_USHORT;

    public:
        explicit ASTUShortType(SourceLocation Loc);

        std::string str() const override {
            return "{unsigned short}";
        }
    };

    /**
     * Short Type
     */
    class ASTShortType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_SHORT;

    public:
        explicit ASTShortType(SourceLocation Loc);

        std::string str() const override {
            return "{short}";
        }
    };

    /**
     * Unsigned Int Type
     */
    class ASTUIntType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_INT;

    public:
        explicit ASTUIntType(SourceLocation Loc);

        std::string str() const override {
            return "{unsigned int}";
        }
    };

    /**
     * Int Type
     */
    class ASTIntType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_INT;

    public:
        explicit ASTIntType(SourceLocation Loc);

        std::string str() const override {
            return "{int}";
        }
    };

    /**
     * Long Type
     */
    class ASTULongType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_LONG;

    public:
        explicit ASTULongType(SourceLocation Loc);

        std::string str() const override {
            return "{unsigned long}";
        }
    };

    /**
     * Long Type
     */
    class ASTLongType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_LONG;

    public:
        explicit ASTLongType(SourceLocation Loc);

        std::string str() const override {
            return "{long}";
        }
    };

    /**
     * Float Type
     */
    class ASTFloatType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_FLOAT;

    public:
        explicit ASTFloatType(SourceLocation Loc);

        std::string str() const override {
            return "{float}";
        }
    };

    /**
     * Long Type
     */
    class ASTDoubleType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_DOUBLE;

    public:
        explicit ASTDoubleType(SourceLocation Loc);

        std::string str() const override {
            return "{double}";
        }
    };

    /**
     * Void Type
     */
    class ASTVoidType : public ASTType {

        const TypeKind Kind = TypeKind::TYPE_VOID;

    public:
        explicit ASTVoidType(SourceLocation Loc);

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

        const std::string &getName() const;

        bool operator ==(const ASTClassType &Ty) const;

        std::string str() const override {
            return Name;
        }
    };
}

#endif //FLY_ASTTYPE_H
