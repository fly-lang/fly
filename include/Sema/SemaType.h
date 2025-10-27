//===-------------------------------------------------------------------------------------------------------------===//
// include/Sym/SemaType.h - AST Class Type header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_TYPE_H
#define FLY_SEMA_TYPE_H

#include "Sema/SemaNode.h"
#include <cstdint>
#include <llvm/ADT/StringRef.h>


namespace fly {

    class SemaValue;

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

    enum class SemaIntTypeKind
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

    enum class SemaFloatTypeKind
    {
        // Floating Point
        TYPE_FLOAT = 32,
        TYPE_DOUBLE = 64
    };

    enum class SemaTypeKind
    {
        TYPE_VOID,
        TYPE_BOOL,
        TYPE_INTEGER,
        TYPE_FLOATING_POINT,
        TYPE_STRING,
        TYPE_ERROR,
        TYPE_ARRAY,
        TYPE_CLASS,
        TYPE_ENUM
    };

    class SemaType : public SemaNode {

        const size_t Id;

        const SemaTypeKind TypeKind;

        const std::string Name;

        SemaValue *DefaultValue;

    public:

        explicit SemaType(SemaTypeKind Kind, std::string Name);

        const size_t getId() const;

        const SemaTypeKind getTypeKind() const;

        const std::string getName() const;

        SemaValue *getDefaultValue() const;

        bool isBool() const;

        bool isFloatingPoint() const;

        bool isInteger() const;

        bool isArray() const;

        bool isString() const;

        bool isClass() const;

        bool isEnum() const;

        bool isError() const;

        bool isVoid() const;

        bool isEquals(const SemaType *Type) const;

        bool operator!=(const SemaType *Type) const;

        bool operator==(const SemaType *Type) const;
    };

    class SemaIntType : public SemaType {

        const SemaIntTypeKind IntKind;

    public:

        explicit SemaIntType(SemaIntTypeKind IntKind, std::string Name);

        const SemaIntTypeKind getIntKind() const;

        bool isSigned();
    };

    class SemaFloatType : public SemaType {

        const SemaFloatTypeKind FPKind;

    public:

        explicit SemaFloatType(SemaFloatTypeKind FPKind, std::string Name);

        const SemaFloatTypeKind getFPKind() const;
    };

    class SemaArrayType : public SemaType {

        SemaType *Type;

    public:

        explicit SemaArrayType(SemaType *Type);

        SemaType *getType();

    };

    class SemaErrorType : public SemaType {

    public:
		explicit SemaErrorType() : SemaType(SemaTypeKind::TYPE_ERROR, "error") {}

	};

}

#endif //FLY_SEMA_TYPE_H
