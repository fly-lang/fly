//===-------------------------------------------------------------------------------------------------------------===//
// include/Sym/SymType.h - AST Class Type header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_TYPE_H
#define FLY_SYM_TYPE_H

#include <cstdint>
#include <llvm/ADT/StringRef.h>


namespace fly {

    const uint8_t MIN_BYTE = 0x0;
    const uint8_t MAX_BYTE = 0xFF;
    const uint16_t MIN_USHORT = 0x0;
    const uint16_t MAX_USHORT = 0xFFFF;
    const uint16_t MIN_SHORT = 0x8000;
    const uint16_t MAX_SHORT = 0x7FFF;
    const uint32_t MIN_UINT = 0x0;
    const uint32_t MAX_UINT = 0xFFFFFFFF;
    const uint32_t MIN_INT = 0x80000000;
    const uint32_t MAX_INT = 0x7FFFFFFF;
    const uint64_t MIN_ULONG = 0x0;
    const uint64_t MAX_ULONG = 0xFFFFFFFFFFFFFFFF;
    const uint64_t MIN_LONG = 0x8000000000000000;
    const uint64_t MAX_LONG = 0x7FFFFFFFFFFFFFFF;

    enum class SymIntTypeKind
    {
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

    enum class SymFPTypeKind
    {
        // Floating Point
        TYPE_FLOAT = 32,
        TYPE_DOUBLE = 64
    };

    enum class SymTypeKind
    {
        TYPE_VOID,
        TYPE_BOOL,
        TYPE_INTEGER,
        TYPE_FLOATING_POINT,
        TYPE_STRING,
        TYPE_CHAR,
        TYPE_ERROR,
        TYPE_ARRAY,
        TYPE_CLASS,
        TYPE_ENUM
    };

    class SymType
    {
        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        const SymTypeKind Kind;

        std::string Name;

    protected:

        explicit SymType(SymTypeKind Kind);

        explicit SymType(SymTypeKind Kind, std::string Name);

    public:

        const SymTypeKind getKind() const;

        const std::string getName() const;

        bool isBool() const;

        bool isFloatingPoint() const;

        bool isInteger() const;

        bool isArray() const;

        bool isString() const;

        bool isChar() const;

        bool isClass() const;

        bool isEnum() const;

        bool isError() const;

        bool isVoid() const;
    };

    class SymTypeInt : public SymType {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        const SymIntTypeKind IntKind;

        explicit SymTypeInt(SymIntTypeKind IntKind);

        public:

        const SymIntTypeKind getIntKind() const;

        bool isSigned();
    };

    class SymTypeFP : public SymType {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        const SymFPTypeKind FPKind;

        explicit SymTypeFP(SymFPTypeKind FPKind);

        public:

        const SymFPTypeKind getFPKind() const;
    };

    class SymTypeArray : public SymType {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        explicit SymTypeArray(SymType *Type, uint64_t Size);

        SymType *Type;

        const uint64_t Size;

        public:

        SymType *getType();

        const uint64_t getSize() const;

    };

}

#endif //FLY_SYM_TYPE_H
