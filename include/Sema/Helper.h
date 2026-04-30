//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Helper.h -  Helper Functions
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef SEMA_HELPER_H
#define SEMA_HELPER_H

#include "llvm/ADT/SmallVector.h"
#include <string>

namespace fly {

	class ASTName;
	class SemaType;
	class SemaFunctionBase;
	class SemaIntType;
	class SemaFloatType;
	class SemaNumberType;
	class SemaExpr;

	class Helper {

	public:

		// Utility Functions
		static std::string Flatten(llvm::SmallVector<ASTName *, 4>);

		static SemaIntType *SelectIntType(SemaIntType *Type1, SemaIntType *Type2);

		static SemaFloatType *SelectFloatType(SemaFloatType *Type1, SemaFloatType *Type2);

		static SemaType * SelectNumberType(SemaNumberType *Type1, SemaNumberType *Type2);
	};
}


#endif //SEMA_HELPER_H
