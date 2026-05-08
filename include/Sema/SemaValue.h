//===-------------------------------------------------------------------------------------------------------------===//
// include/Sym/SemaValue.h - Sema Value
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_VALUE_H
#define FLY_SEMA_VALUE_H

#include "CodeGen/CodeGenArrayValue.h"
#include "Sema/SemaExpr.h"
#include "SemaType.h"

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/StringMap.h>

namespace fly {

	class ASTValue;
	class ASTBoolValue;
	class ASTNumberValue;
	class ASTStringValue;
	class ASTArrayValue;
	class ASTStructValue;
	class ASTNullValue;
	class ASTUnsetValue;
	class ASTEnumEntry;
	class CodeGenArrayValue;
    class SemaType;

    class SemaValue : public SemaExpr {

    	friend class SemaBuilder;
    	friend class Resolver;
    	friend class SemaValidator;

    	ASTValue &AST;

    protected:

        explicit SemaValue(ASTValue &AST, SemaType *Type);

    public:

        ~SemaValue() override = default;

    	ASTValue *getAST() const;

    	void setCodeGen(CodeGenExpr *CGC);
    };

	class SemaBoolValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		bool Value;

		explicit SemaBoolValue(ASTBoolValue &AST);

	public:

		~SemaBoolValue() override = default;

		bool getValue() const;

		void accept(SemaVisitor& Visitor) override;

	};

	class SemaIntValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::APInt Value;

		explicit SemaIntValue(ASTNumberValue &AST, SemaIntType *Type, llvm::APInt &Value);

	public:

		~SemaIntValue() override = default;

		llvm::APInt getValue() const;

		void accept(SemaVisitor& Visitor) override;

	};

	class SemaFloatValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::APFloat Value;

		explicit SemaFloatValue(ASTNumberValue &AST, SemaFloatType *Type, llvm::APFloat &Value);

	public:

		~SemaFloatValue() override = default;

		llvm::APFloat getValue() const;

		void accept(SemaVisitor& Visitor) override;

	};

	class SemaComplexValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::APFloat Real;
		llvm::APFloat Imag;

		explicit SemaComplexValue(ASTNumberValue &AST, SemaComplexType *Type,
		                          llvm::APFloat &Real, llvm::APFloat &Imag);

	public:

		~SemaComplexValue() override = default;

		llvm::APFloat getReal() const;
		llvm::APFloat getImag() const;

		void accept(SemaVisitor& Visitor) override;

	};

	class SemaStringValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::StringRef Value;

		explicit SemaStringValue(ASTStringValue &AST);

	public:

		~SemaStringValue() override = default;

		llvm::StringRef getValue() const;

		void accept(SemaVisitor& Visitor) override;

	};

	class SemaArrayValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::SmallVector<SemaValue *, 8> Values;

		explicit SemaArrayValue(ASTArrayValue &AST, SemaType *Type);

	public:

		~SemaArrayValue() override = default;

		const llvm::SmallVector<SemaValue *, 8> &getValues() const;

		void accept(SemaVisitor& Visitor) override;

		CodeGenArrayValue *getCodeGen() const override;

	};

	class SemaStructValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::StringMap<SemaValue *> Values;

		explicit SemaStructValue(ASTStructValue &AST, SemaType *Type);

	public:

		~SemaStructValue() override = default;

		const llvm::StringMap<SemaValue *> &getValues() const;

		void accept(SemaVisitor& Visitor) override;

	};

	class SemaNullValue : public SemaValue {

		friend class SemaBuilder;

		explicit SemaNullValue(ASTNullValue &AST);

	public:

		~SemaNullValue() override = default;

		void accept(SemaVisitor& Visitor) override;
	};

	class SemaUnsetValue : public SemaValue {

		friend class SemaBuilder;

		explicit SemaUnsetValue(ASTUnsetValue &AST);

	public:

		~SemaUnsetValue() override = default;

		void accept(SemaVisitor& Visitor) override;
	};

}

#endif //FLY_SEMA_VALUE_H
