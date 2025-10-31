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

#include "Sema/SemaNode.h"
#include <cstdint>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/StringMap.h>

namespace fly {

    class SemaType;
	class ASTValue;
    class ASTNumberValue;

    class SemaValue : public SemaNode {

    	friend class SemaBuilder;
    	friend class Resolver;
    	friend class SemaValidator;

    	static const bool DEFAULT_BOOL_VALUE;

    	static const llvm::StringRef DEFAULT_INTEGER_VALUE;

    	static const llvm::StringRef DEFAULT_FLOATING_VALUE;

    	static const llvm::StringRef DEFAULT_STRING_VALUE;

    	static const llvm::StringRef DEFAULT_CHAR_VALUE;

		SemaType *Type;

    protected:

        explicit SemaValue();

    public:

		SemaType *getType() const;

    };

	class SemaBoolValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		bool Value;

		explicit SemaBoolValue(bool Value);

	public:

		bool getValue() const;

	};

	class SemaIntValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::APInt Value;

		explicit SemaIntValue(llvm::StringRef Value, uint8_t Radix);

	public:

		llvm::APInt getValue() const;

	};

	class SemaFloatValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::APFloat Value;

		explicit SemaFloatValue(llvm::StringRef Value);

	public:

		llvm::APFloat getValue() const;

	};

	class SemaStringValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::StringRef Value;

		explicit SemaStringValue(llvm::StringRef Value);

	public:

		llvm::StringRef getValue() const;

	};

	class SemaArrayValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::SmallVector<SemaValue *, 8> Values;

		explicit SemaArrayValue();

	public:

		const llvm::SmallVector<SemaValue *, 8> &getValues() const;

	};

	class SemaStructValue : public SemaValue {

		friend class SemaBuilder;
		friend class Resolver;
		friend class SemaValidator;

		llvm::StringMap<SemaValue *> Values;

		explicit SemaStructValue();

	public:

		const llvm::StringMap<SemaValue *> &getValues() const;

	};

	class SemaNullValue : public SemaValue {

	};

}

#endif //FLY_SEMA_VALUE_H
