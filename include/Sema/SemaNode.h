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

#include "Sema/SemaVisitor.h"

namespace fly {

	enum class SemaKind {
		NAMESPACE,
		IMPORT,
		TYPE,

		// Types
		TYPE_VOID,
		TYPE_BOOL,
		TYPE_INTEGER,
		TYPE_FLOAT,
		TYPE_STRING,
		TYPE_ERROR,
		TYPE_ARRAY,
		TYPE_CLASS,
		TYPE_ENUM,

		// Variables
		PARAM_VAR,
		LOCAL_VAR,
		ERROR_VAR,
		ATTRIBUTE,
		INSTANCE_VAR,

		// Expressions
		MEMBER,
		CALL,
		UNARY,
		BINARY,
		TERNARY,
		CAST,

		// Top Level nodes
		FUNCTION,
		CLASS,
		METHOD,
		ENUM,

		// Values
		ENUM_ENTRY,
		ENUM_LIST,
		VALUE
	};

	class SemaNode {

	protected:

		SemaKind Kind;

	public:

		SemaNode(SemaKind Kind);

		virtual ~SemaNode() = default;

		SemaKind getKind() const;

		virtual void accept(SemaVisitor& Visitor) = 0;
	};

}

#endif //FLY_SEMA_NODE_H
