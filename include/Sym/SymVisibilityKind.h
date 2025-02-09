//===-------------------------------------------------------------------------------------------------------------===//
// include/Sym/SymVisibilityKind.h - Sybolic Table for Visibility
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_VISIBILITYKIND_H
#define FLY_SYM_VISIBILITYKIND_H


namespace fly {

	enum class SymVisibilityKind {
		DEFAULT,
		PUBLIC,
		PRIVATE
	};

}

#endif //FLY_SYM_VISIBILITYKIND_H
