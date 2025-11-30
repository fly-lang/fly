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

#include "Sema/SemaNode.h"

namespace fly {

	class ASTNode;

	enum class SemaKind {
		MODULE,
		NAMESPACE,
		IMPORT,
		TYPE,
		VAR,
		CALL,
		FUNCTION,
		CLASS,
		ATTRIBUTE,
		METHOD,
		ENUM,
		ENUM_ENTRY,
		VALUE
	};

	class SemaNode {

		SemaKind Kind;

	public:

		SemaNode(SemaKind Kind);

		virtual ~SemaNode() = default;

		SemaKind getKind() const;

		// virtual void accept(SemaVisitor& Visitor) = 0; // FIXME To manage errors
	};

}

#endif //FLY_SEMA_NODE_H
