//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaNode.h - Sema Node header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_SEMA_NODE_H
#define FLY_SEMA_NODE_H

namespace fly {

	enum class SemaKind {
		MODULE,
		NAMESPACE,
		TYPE,
		VAR,
		FUNCTION,
		CLASS,
		ENUM,
		ATTRIBUTE,
		METHOD
	};

	class SemaNode {

	public:

		SemaNode() = default;

		virtual ~SemaNode() = default;

		// virtual void accept(SemaVisitor& v) = 0;
	};

}

#endif //FLY_SEMA_NODE_H
