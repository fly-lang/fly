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

        // Boolean
        TYPE_BOOL = 1,

        // Signed Integer
        TYPE_BYTE = 2,
        TYPE_SHORT = 3,
        TYPE_INT = 4,
        TYPE_LONG = 5,

        // Unsigned Integer
        TYPE_USHORT = 6,
        TYPE_UINT = 7,
        TYPE_ULONG = 8,

        // Floating Point
        TYPE_FLOAT = 9,
        TYPE_DOUBLE = 10,

        // Aggregates
        TYPE_ARRAY = 11,
        TYPE_CLASS = 12
    };

    enum MacroTypeKind {
        MACRO_TYPE_VOID,
        MACRO_TYPE_BOOL,
        MACRO_TYPE_INTEGER,
        MACRO_TYPE_FLOATING_POINT,
        MACRO_TYPE_ARRAY,
        MACRO_TYPE_CLASS
    };

    class ASTIntegerValue;
    class ASTValueExpr;
    class ASTExpr;
    class ASTClass;

    /**
     * Abstract Base Type
     */
    class ASTType {

        friend class SemaBuilder;
        friend class SemaResolver;

        const TypeKind Kind;

        const MacroTypeKind MacroKind;

        const SourceLocation Loc;

    protected:
        ASTType(const SourceLocation &Loc, TypeKind Kind, MacroTypeKind MacroKind);

    public:

        virtual ~ASTType() = default;

        const SourceLocation &getLocation() const;

        const TypeKind &getKind() const;

        const MacroTypeKind &getMacroKind() const;

        const bool isBool() const;

        const bool isFloatingPoint() const;

        const bool isInteger() const;

        const bool isNumber() const;

        const bool isArray() const;

        const bool isClass() const;

        const std::string printMacroType();

        static const std::string printMacroType(const MacroTypeKind Kind);

        virtual const std::string print() const = 0;

        virtual const std::string str() const = 0;
    };

    /**
     * Void Type
     */
    class ASTVoidType : public ASTType {

        friend class SemaBuilder;

        explicit ASTVoidType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        const std::string str() const override;
    };

    /**
     * Boolean Type
     */
    class ASTBoolType : public ASTType {

        friend class SemaBuilder;

        explicit ASTBoolType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        const std::string str() const override;
    };

    /**
     * Byte Type
     */
    class ASTByteType : public ASTType {

        friend class SemaBuilder;

        explicit ASTByteType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        const std::string str() const override;
    };

    /**
     * Unsigned Short Type
     */
    class ASTUShortType : public ASTType {

        friend class SemaBuilder;

        explicit ASTUShortType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        const std::string str() const override;
    };

    /**
     * Short Type
     */
    class ASTShortType : public ASTType {

        friend class SemaBuilder;

        explicit ASTShortType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        const std::string str() const override;
    };

    /**
     * Unsigned Int Type
     */
    class ASTUIntType : public ASTType {

        friend class SemaBuilder;

        explicit ASTUIntType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        const std::string str() const override;
    };

    /**
     * Int Type
     */
    class ASTIntType : public ASTType {

        friend class SemaBuilder;

        explicit ASTIntType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        const std::string str() const override;
    };

    /**
     * Long Type
     */
    class ASTULongType : public ASTType {

        friend class SemaBuilder;

        explicit ASTULongType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        const std::string str() const override;
    };

    /**
     * Long Type
     */
    class ASTLongType : public ASTType {

        friend class SemaBuilder;

        explicit ASTLongType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        const std::string str() const override;
    };

    /**
     * Float Type
     */
    class ASTFloatType : public ASTType {

        friend class SemaBuilder;

        explicit ASTFloatType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        const std::string str() const override;
    };

    /**
     * Long Type
     */
    class ASTDoubleType : public ASTType {

        friend class SemaBuilder;

        explicit ASTDoubleType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        const std::string str() const override;
    };

    /**
     * String Type
     */
    class ASTArrayType : public ASTType {

        friend class SemaBuilder;

        ASTExpr *Size = nullptr;

        ASTType *Type = nullptr;

        ASTArrayType(const SourceLocation &Loc, ASTType *Type, ASTExpr *Size);

    public:

        ASTExpr *getSize() const;

        ASTType *getType() const;

        const std::string print() const override;

        const std::string str() const override;
    };

    /**
     * Class Type
     */
    class ASTClassType : public ASTType {

        friend class SemaBuilder;

        const TypeKind Kind = TypeKind::TYPE_CLASS;

        std::string Name;

        std::string NameSpace;

        ASTClass *Def = nullptr;

        ASTClassType(const SourceLocation &Loc, std::string Name, std::string NameSpace = "");

    public:

        const std::string getName() const;

        const std::string getNameSpace() const;

        ASTClass *getDef() const;

        bool operator ==(const ASTClassType &Ty) const;

        const std::string print() const override;

        const std::string str() const override;
    };
}

#endif //FLY_ASTTYPE_H
