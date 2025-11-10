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

#include <AST/ASTValue.h>

#include "Sema/SemaNode.h"
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/StringMap.h>

namespace fly {

    class SemaType;
	class ASTValue;
	class ASTBoolValue;
    class ASTNumberValue;
	class ASTStringValue;

    class SemaValue : public SemaNode {

    	friend class SemaBuilder;
    	friend class Resolver;
    	friend class SemaValidator;

    	ASTValue &AST;

		SemaType *Type;

    protected:

        explicit SemaValue(ASTValue &AST);

    public:

		SemaType *getType() const;

    };

	class SemaBoolValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		bool Value;

		explicit SemaBoolValue(ASTBoolValue &AST);

	public:

		bool getValue() const;

	};

	class SemaIntValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::APInt Value;

		explicit SemaIntValue(ASTNumberValue &AST);

	public:

		llvm::APInt getValue() const;

	};

	class SemaFloatValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::APFloat Value;

		explicit SemaFloatValue(ASTNumberValue &AST);

	public:

		llvm::APFloat getValue() const;

	};

	class SemaStringValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::StringRef Value;

		explicit SemaStringValue(ASTStringValue &AST);

	public:

		llvm::StringRef getValue() const;

	};

	class SemaArrayValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::SmallVector<SemaValue *, 8> Values;

		explicit SemaArrayValue(ASTArrayValue &AST);

	public:

		const llvm::SmallVector<SemaValue *, 8> &getValues() const;

	};

	class SemaStructValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::StringMap<SemaValue *> Values;

		explicit SemaStructValue(ASTStructValue &AST);

	public:

		const llvm::StringMap<SemaValue *> &getValues() const;

	};

	class SemaNullValue : public SemaValue {

		friend class SemaBuilder;

		explicit SemaNullValue(ASTNullValue &AST);
	};

}

#endif //FLY_SEMA_VALUE_H
