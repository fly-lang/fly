//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaNode.h - AST node semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_SEMA_NODE_H
#define FLY_SEMA_NODE_H

#include "Sema/SemaVisitor.h"

#include <string>

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
		TYPE_COMPLEX,
		TYPE_STRING,
		TYPE_ERROR,
		TYPE_ARRAY,
		TYPE_CLASS,
		TYPE_ENUM,
		TYPE_PARAM,

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
		METHOD,

		// Values
		ENUM_ENTRY,
		ENUM_LIST,
		VALUE,

		// Types (suite)
		TYPE_SUITE,

		// Statements
		STMT_BLOCK,
		STMT_DECL,
		STMT_EXPR,
		STMT_RETURN,
		STMT_IF,
		STMT_SWITCH,
		STMT_LOOP,
		STMT_LOOP_IN,
		STMT_DELETE,
		STMT_BREAK,
		STMT_CONTINUE,
		STMT_FAIL,
		STMT_HANDLE,
		STMT_TEST,
		STMT_CASE
	};

	class SemaNode {

	protected:

		SemaKind Kind;

	public:

		SemaNode(SemaKind Kind);

		virtual ~SemaNode() = default;

		SemaKind getKind() const;

		virtual std::string str() const;

		virtual void accept(SemaVisitor& Visitor) = 0;
	};

}

#endif //FLY_SEMA_NODE_H
