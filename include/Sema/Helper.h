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
	class ASTCall;
	class ASTFunction;

	class Helper {

	public:

		// Utility Functions
		static std::string Flatten(llvm::SmallVector<ASTName *, 4>);

		// static std::string Mangle(llvm::StringRef Name, const llvm::SmallVector<SemaType *, 8> &Types);

		static std::string Mangle(ASTCall *AST);

		static std::string Mangle(ASTFunction *AST);
	};
}


#endif //SEMA_HELPER_H
