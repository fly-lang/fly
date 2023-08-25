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

#include "Basic/Debuggable.h"
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

    enum class ASTIntegerTypeKind {

        // Unsigned Integer
        TYPE_BYTE = 8,
        TYPE_USHORT = 16,
        TYPE_UINT = 32,
        TYPE_ULONG = 64,

        // Signed Integer
        TYPE_SHORT = 15,
        TYPE_INT = 31,
        TYPE_LONG = 63,
    };

    enum class ASTFloatingPointTypeKind {

        // Floating Point
        TYPE_FLOAT = 32,
        TYPE_DOUBLE = 64
    };

    enum class ASTTypeKind {
        TYPE_VOID,
        TYPE_BOOL,
        TYPE_INTEGER,
        TYPE_FLOATING_POINT,
        TYPE_STRING,
        TYPE_ARRAY,
        TYPE_IDENTITY
    };

    class ASTExpr;

    /**
     * Abstract Base Type
     */
    class ASTType : public Debuggable {

        friend class SemaBuilder;
        friend class SemaResolver;

        const ASTTypeKind Kind;

        const SourceLocation Loc;

    protected:

        ASTType(const SourceLocation &Loc, ASTTypeKind MacroKind);

    public:

        virtual ~ASTType() = default;

        const SourceLocation &getLocation() const;

        const ASTTypeKind &getKind() const;

        const bool isBool() const;

        const bool isFloatingPoint() const;

        const bool isInteger() const;

        const bool isArray() const;

        const bool isString() const;

        const bool isIdentity() const;

        const bool isVoid() const;

        const std::string printType();

        static const std::string printType(const ASTTypeKind Kind);

        virtual const std::string print() const = 0;

        virtual std::string str() const;
    };

    /**
     * Void Type
     */
    class ASTVoidType : public ASTType {

        friend class SemaBuilder;

        explicit ASTVoidType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        std::string str() const override;
    };

    /**
     * Boolean Type
     */
    class ASTBoolType : public ASTType {

        friend class SemaBuilder;

        explicit ASTBoolType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        std::string str() const override;
    };

    class ASTIntegerType : public ASTType {

        ASTIntegerTypeKind Kind;

    protected:

        ASTIntegerType(const SourceLocation &Loc, ASTIntegerTypeKind Kind);

    public:

        ASTIntegerTypeKind getIntegerKind() const;

        const bool isUnsigned() const;

        const bool isSigned() const;

        const uint32_t getSize();
    };

    class ASTFloatingPointType : public ASTType {

        ASTFloatingPointTypeKind Kind;

    protected:

        ASTFloatingPointType(const SourceLocation &Loc, ASTFloatingPointTypeKind Kind);

    public:

        ASTFloatingPointTypeKind getFloatingPointKind() const;

        const uint32_t getSize();
    };

    /**
     * Byte Type
     */
    class ASTByteType : public ASTIntegerType {

        friend class SemaBuilder;

        explicit ASTByteType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        std::string str() const override;
    };

    /**
     * Unsigned Short Type
     */
    class ASTUShortType : public ASTIntegerType {

        friend class SemaBuilder;

        explicit ASTUShortType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        std::string str() const override;
    };

    /**
     * Short Type
     */
    class ASTShortType : public ASTIntegerType {

        friend class SemaBuilder;

        explicit ASTShortType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        std::string str() const override;
    };

    /**
     * Unsigned Int Type
     */
    class ASTUIntType : public ASTIntegerType {

        friend class SemaBuilder;

        explicit ASTUIntType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        std::string str() const override;
    };

    /**
     * Int Type
     */
    class ASTIntType : public ASTIntegerType {

        friend class SemaBuilder;

        explicit ASTIntType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        std::string str() const override;
    };

    /**
     * Long Type
     */
    class ASTULongType : public ASTIntegerType {

        friend class SemaBuilder;

        explicit ASTULongType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        std::string str() const override;
    };

    /**
     * Long Type
     */
    class ASTLongType : public ASTIntegerType {

        friend class SemaBuilder;

        explicit ASTLongType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        std::string str() const override;
    };

    /**
     * Float Type
     */
    class ASTFloatType : public ASTFloatingPointType {

        friend class SemaBuilder;

        explicit ASTFloatType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        std::string str() const override;
    };

    /**
     * Long Type
     */
    class ASTDoubleType : public ASTFloatingPointType {

        friend class SemaBuilder;

        explicit ASTDoubleType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        std::string str() const override;
    };

    /**
     * Array Type
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

        std::string str() const override;
    };

    /**
     * String Type
     */
    class ASTStringType : public ASTType {

        friend class SemaBuilder;

        ASTStringType(const SourceLocation &Loc);

    public:

        const std::string print() const override;

        std::string str() const override;
    };
}

#endif //FLY_ASTTYPE_H
