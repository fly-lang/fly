//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaType.h - type semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_TYPE_H
#define FLY_SEMA_TYPE_H

#include "CodeGen/CodeGenType.h"
#include "Sema/SemaNode.h"

#include "llvm/ADT/StringRef.h"

#include <cstdint>
#include <string>

namespace fly {

    class SemaValue;
    class ASTExpr;

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

    class SemaType : public SemaNode {

        const size_t Id;

        const std::string Name;

    	CodeGenType *CG = nullptr;

    public:

        explicit SemaType(SemaKind Kind, std::string Name);

        ~SemaType() override;

        const size_t getId() const;

        const std::string getName() const;

        bool isBool() const;

        bool isFloat() const;

        bool isInteger() const;

    	bool isNumber() const;

        bool isComplex() const;

        bool isArray() const;

        bool isString() const;

        bool isClass() const;

        bool isEnum() const;

        bool isError() const;

        bool isVoid() const;

        bool isEquals(const SemaType *Type) const;

        bool operator!=(const SemaType *Type) const;

        bool operator==(const SemaType *Type) const;

    	virtual CodeGenType *getCodeGen() const;

    	void setCodeGen(CodeGenType *CG);

		std::string str() const override;
    };

	class SemaBoolType : public SemaType {

	public:
		explicit SemaBoolType() : SemaType(SemaKind::TYPE_BOOL, "bool") {}

		~SemaBoolType() override = default;

		std::string str() const override;

		void accept(SemaVisitor& Visitor) override;
	};

	class SemaNumberType : public SemaType {

		unsigned Rank;

	public:
		explicit SemaNumberType(SemaKind Kind, std::string Name, unsigned Rank);

		~SemaNumberType() override = default;

		unsigned getRank();

		std::string str() const override;
	};

    class SemaIntType : public SemaNumberType {

        const SemaIntTypeKind IntKind;

    public:

        explicit SemaIntType(SemaIntTypeKind IntKind, std::string Name);

        ~SemaIntType() override = default;

        const SemaIntTypeKind getIntKind() const;

    	bool isUnsigned();

        bool isSigned();

        std::string str() const override;

        void accept(SemaVisitor& Visitor) override;
    };

    class SemaFloatType : public SemaNumberType {

        const SemaFloatTypeKind FloatKind;

    public:

        explicit SemaFloatType(SemaFloatTypeKind FPKind, std::string Name);

        ~SemaFloatType() override = default;

        const SemaFloatTypeKind getFloatKind() const;

        std::string str() const override;

        void accept(SemaVisitor& Visitor) override;
    };

    class SemaComplexType : public SemaType {

    public:

        explicit SemaComplexType() : SemaType(SemaKind::TYPE_COMPLEX, "complex") {}

        ~SemaComplexType() override = default;

        std::string str() const override;

        void accept(SemaVisitor& Visitor) override;
    };

    class SemaArrayType : public SemaType {

        SemaType *ElementType;

    	uint64_t Size;

        SemaExpr *SizeExpr = nullptr;

    public:

        explicit SemaArrayType(SemaType *ElementType, SemaExpr *SizeExpr);

		explicit SemaArrayType(SemaType *Type, uint64_t Size);

        ~SemaArrayType() override = default;

        SemaType *getElementType();

    	void setElementType(SemaType *ElementType);

		SemaExpr *getSizeExpr();

    	uint64_t getSize() const;

        std::string str() const override;

        void accept(SemaVisitor& Visitor) override;

    };

    class SemaErrorType : public SemaType {

    public:
		explicit SemaErrorType() : SemaType(SemaKind::TYPE_ERROR, "error") {}

		~SemaErrorType() override = default;

		std::string str() const override;

		void accept(SemaVisitor& Visitor) override;

	};

	class SemaStringType : public SemaType {

	public:
		explicit SemaStringType() : SemaType(SemaKind::TYPE_STRING, "string") {}

		~SemaStringType() override = default;

		std::string str() const override;

		void accept(SemaVisitor& Visitor) override;
	};

	class SemaVoidType : public SemaType {

	public:
		explicit SemaVoidType() : SemaType(SemaKind::TYPE_VOID, "void") {}

		~SemaVoidType() override = default;

		std::string str() const override;

		void accept(SemaVisitor& Visitor) override;
	};

	// SemaTypeParam represents a type variable (T, U) in a generic class/method.
	// It carries an optional upper bound (T extends Foo → Bound = SemaClassType*Foo).
	// Instances exist only as template metadata; they are substituted away during
	// monomorphization and never reach CodeGen.
	class SemaTypeParam : public SemaType {

		SemaType *Bound = nullptr;   // null = unconstrained

	public:
		explicit SemaTypeParam(llvm::StringRef Name, SemaType *Bound = nullptr)
		    : SemaType(SemaKind::TYPE_PARAM, Name.str()), Bound(Bound) {}

		~SemaTypeParam() override = default;

		SemaType *getBound() const { return Bound; }

		bool isClass() const { return false; }

		std::string str() const override;

		void accept(SemaVisitor& Visitor) override;
	};

}

#endif //FLY_SEMA_TYPE_H
