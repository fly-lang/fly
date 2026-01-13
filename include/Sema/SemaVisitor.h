//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaVisitor.h - Sema Visitor
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_VISITOR_H
#define FLY_SEMA_VISITOR_H

namespace fly {

	// Forward declarations
	class SemaModule;
	class SemaNameSpace;
	class SemaImport;

	// Types
	class SemaType;
	class SemaIntType;
	class SemaFloatType;
	class SemaArrayType;
	class SemaClassType;
	class SemaEnumType;
	class SemaErrorType;

	// Functions
	class SemaFunctionBase;
	class SemaFunction;
	class SemaClassMethod;

	// Variables
	class SemaVar;
	class SemaLocalVar;
	class SemaParam;
	class SemaMemberVar;
	class SemaClassAttribute;
	class SemaClassInstance;
	class SemaEnumValue;
	class SemaErrorHandler;

	// Expressions
	class SemaExpr;
	class SemaCall;
	class SemaUnary;
	class SemaBinary;
	class SemaTernary;
	class SemaCast;

	// Values
	class SemaValue;
	class SemaBoolValue;
	class SemaIntValue;
	class SemaFloatValue;
	class SemaStringValue;
	class SemaArrayValue;
	class SemaStructValue;
	class SemaNullValue;

	/**
	 * SemaVisitor - Visitor pattern for all Sema nodes
	 *
	 * This visitor allows traversal of the semantic representation tree.
	 * Implement this interface to perform operations on Sema nodes.
	 */
	class SemaVisitor {

	public:

		virtual ~SemaVisitor() = default;

		// Module and Namespace
		virtual void visit(SemaModule &Sema) = 0;
		virtual void visit(SemaNameSpace &Sema) = 0;
		virtual void visit(SemaImport &Sema) = 0;

		// Types
		virtual void visit(SemaType &Sema) = 0;
		virtual void visit(SemaIntType &Sema) = 0;
		virtual void visit(SemaFloatType &Sema) = 0;
		virtual void visit(SemaArrayType &Sema) = 0;
		virtual void visit(SemaClassType &Sema) = 0;
		virtual void visit(SemaEnumType &Sema) = 0;
		virtual void visit(SemaErrorType &Sema) = 0;

		// Functions
		virtual void visit(SemaFunctionBase &Sema) = 0;
		virtual void visit(SemaFunction &Sema) = 0;
		virtual void visit(SemaClassMethod &Sema) = 0;

		// Variables
		virtual void visit(SemaVar &Sema) = 0;
		virtual void visit(SemaLocalVar &Sema) = 0;
		virtual void visit(SemaParam &Sema) = 0;
		virtual void visit(SemaMemberVar &Sema) = 0;
		virtual void visit(SemaClassAttribute &Sema) = 0;
		virtual void visit(SemaClassInstance &Sema) = 0;
		virtual void visit(SemaEnumValue &Sema) = 0;
		virtual void visit(SemaErrorHandler &Sema) = 0;

		// Expressions
		virtual void visit(SemaExpr &Sema) = 0;
		virtual void visit(SemaCall &Sema) = 0;
		virtual void visit(SemaUnary &Sema) = 0;
		virtual void visit(SemaBinary &Sema) = 0;
		virtual void visit(SemaTernary &Sema) = 0;
		virtual void visit(SemaCast &Sema) = 0;

		// Values
		virtual void visit(SemaValue &Sema) = 0;
		virtual void visit(SemaBoolValue &Sema) = 0;
		virtual void visit(SemaIntValue &Sema) = 0;
		virtual void visit(SemaFloatValue &Sema) = 0;
		virtual void visit(SemaStringValue &Sema) = 0;
		virtual void visit(SemaArrayValue &Sema) = 0;
		virtual void visit(SemaStructValue &Sema) = 0;
		virtual void visit(SemaNullValue &Sema) = 0;
	};

}

#endif //FLY_SEMA_VISITOR_H

