//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaVisibilityKind.h - visibility kind enumeration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_VISIBILITYKIND_H
#define FLY_SEMA_VISIBILITYKIND_H


namespace fly {

	enum class SemaVisibilityKind {
		PRIVATE,
		PROTECTED,
		DEFAULT,
		PUBLIC,
	};

}

#endif //FLY_SEMA_VISIBILITYKIND_H
