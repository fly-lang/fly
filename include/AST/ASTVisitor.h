//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVisitor.h - AST Visitor
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_VISITOR_H
#define FLY_AST_VISITOR_H

namespace fly {

	class ASTModule;
	class ASTNameSpace;
	class ASTImport;
	class ASTClass;
	class ASTAttribute;
	class ASTMethod;
	class ASTFunction;
	class ASTEnum;
	class ASTEnumEntry;
	class ASTComment;
	class ASTCall;
	class ASTBoolValue;
	class ASTNumberValue;
	class ASTStringValue;
	class ASTArrayValue;
	class ASTStructValue;
	class ASTNullValue;
	class ASTLocalVar;
	class ASTIdentifier;
	class ASTBuiltinType;
	class ASTNamedType;
	class ASTArrayType;
	class ASTBreakStmt;
	class ASTContinueStmt;
	class ASTDeleteStmt;
	class ASTExprStmt;
	class ASTFailStmt;
	class ASTHandleStmt;
	class ASTReturnStmt;
	class ASTDeclStmt;
	class ASTRuleStmt;
	class ASTIfStmt;
	class ASTSwitchStmt;
	class ASTLoopStmt;
	class ASTLoopInStmt;
	class ASTBlockStmt;
	class ASTUnary;
	class ASTBinary;
	class ASTTernary;
	class ASTCast;
	class ASTMember;
	class ASTParam;

	class ASTVisitor {

	public:

		// Top Level Nodes
		virtual void visit(ASTModule &AST) = 0;
		virtual void visit(ASTNameSpace &AST) = 0;
		virtual void visit(ASTImport &AST) = 0;
		virtual void visit(ASTFunction& Function) = 0;
		virtual void visit(ASTClass& AST) = 0;
		virtual void visit(ASTAttribute& AST) = 0;
		virtual void visit(ASTMethod& AST) = 0;
		virtual void visit(ASTEnum& AST) = 0;
		virtual void visit(ASTLocalVar &AST) = 0;
		virtual void visit(ASTParam &AST) = 0;
		virtual void visit(ASTComment &AST) = 0;

		// Types
		virtual void visit(ASTBuiltinType &AST) = 0;
		virtual void visit(ASTNamedType &AST) = 0;
		virtual void visit(ASTArrayType &AST) = 0;

		// Statements
		virtual void visit(ASTBreakStmt &AST) = 0;
		virtual void visit(ASTContinueStmt &AST) = 0;
		virtual void visit(ASTDeleteStmt &AST) = 0;
		virtual void visit(ASTExprStmt &AST) = 0;
		virtual void visit(ASTDeclStmt &AST) = 0;
		virtual void visit(ASTFailStmt &AST) = 0;
		virtual void visit(ASTHandleStmt &AST) = 0;
		virtual void visit(ASTReturnStmt &AST) = 0;
		virtual void visit(ASTRuleStmt &AST) = 0;
		virtual void visit(ASTIfStmt &AST) = 0;
		virtual void visit(ASTSwitchStmt &AST) = 0;
		virtual void visit(ASTLoopStmt &AST) = 0;
		virtual void visit(ASTLoopInStmt &AST) = 0;
		virtual void visit(ASTBlockStmt &AST) = 0;

		// Expressions
		virtual void visit(ASTIdentifier &AST) = 0;
		virtual void visit(ASTMember &AST) = 0;
		virtual void visit(ASTCall &AST) = 0;
		virtual void visit(ASTUnary &AST) = 0;
		virtual void visit(ASTBinary &AST) = 0;
		virtual void visit(ASTTernary &AST) = 0;
		virtual void visit(ASTCast &AST) = 0;
		virtual void visit(ASTBoolValue &AST) = 0;
		virtual void visit(ASTNumberValue &AST) = 0;
		virtual void visit(ASTStringValue &AST) = 0;
		virtual void visit(ASTArrayValue &AST) = 0;
		virtual void visit(ASTStructValue &AST) = 0;
		virtual void visit(ASTNullValue &AST) = 0;
		virtual void visit(ASTEnumEntry &AST) = 0;
	};

}

#endif //FLY_AST_VISITOR_H