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

#include <string>

namespace fly {

	class ASTIdentifier;

	class Helper {

	public:

		// Utility Functions
		static std::string Flatten(ASTIdentifier *Identifier);
	};
}


#endif //SEMA_HELPER_H
