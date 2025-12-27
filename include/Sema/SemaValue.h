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

#include "Sema/SemaExpr.h"
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/StringMap.h>

#include "SemaType.h"

namespace fly {

	class ASTValue;
	class ASTBoolValue;
	class ASTNumberValue;
	class ASTStringValue;
	class ASTArrayValue;
	class ASTStructValue;
	class ASTNullValue;
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

	};

	class SemaNullValue : public SemaValue {

		friend class SemaBuilder;

		explicit SemaNullValue(ASTNullValue &AST);

	public:

		~SemaNullValue() override = default;
	};

}

#endif //FLY_SEMA_VALUE_H
