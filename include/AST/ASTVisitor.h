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
	class ASTFunction;
	class ASTEnum;
	class ASTComment;
	class ASTCall;
	class ASTBoolValue;
	class ASTNumberValue;
	class ASTStringValue;
	class ASTArrayValue;
	class ASTStructValue;
	class ASTNullValue;
	class ASTVar;
	class ASTIdentifier;
	class ASTType;
	class ASTArrayType;
	class ASTBreakStmt;
	class ASTContinueStmt;
	class ASTDeleteStmt;
	class ASTExprStmt;
	class ASTFailStmt;
	class ASTHandleStmt;
	class ASTReturnStmt;
	class ASTRuleStmt;
	class ASTIfStmt;
	class ASTSwitchStmt;
	class ASTLoopStmt;
	class ASTLoopInStmt;
	class ASTVarStmt;
	class ASTBlockStmt;
	class ASTUnaryOpExpr;
	class ASTBinaryOpExpr;
	class ASTTernaryOpExpr;
	class ASTCast;
	class ASTMember;

	class ASTVisitor {

	public:

		// Top Level Nodes
		virtual void visit(ASTModule &AST) = 0;
		virtual void visit(ASTNameSpace &AST) = 0;
		virtual void visit(ASTImport &AST) = 0;
		virtual void visit(ASTClass& Class) = 0;
		virtual void visit(ASTFunction& Function) = 0;
		virtual void visit(ASTEnum& Enum) = 0;
		virtual void visit(ASTVar &AST) = 0;
		virtual void visit(ASTComment &AST) = 0;

		// Types
		virtual void visit(ASTType &AST) = 0;
		virtual void visit(ASTArrayType &AST) = 0;

		// Statements
		virtual void visit(ASTBreakStmt &AST) = 0;
		virtual void visit(ASTContinueStmt &AST) = 0;
		virtual void visit(ASTDeleteStmt &AST) = 0;
		virtual void visit(ASTExprStmt &AST) = 0;
		virtual void visit(ASTFailStmt &AST) = 0;
		virtual void visit(ASTHandleStmt &AST) = 0;
		virtual void visit(ASTReturnStmt &AST) = 0;
		virtual void visit(ASTRuleStmt &AST) = 0;
		virtual void visit(ASTIfStmt &AST) = 0;
		virtual void visit(ASTSwitchStmt &AST) = 0;
		virtual void visit(ASTLoopStmt &AST) = 0;
		virtual void visit(ASTLoopInStmt &AST) = 0;
		virtual void visit(ASTVarStmt &AST) = 0;
		virtual void visit(ASTBlockStmt &AST) = 0;

		// Expressions
		virtual void visit(ASTIdentifier &AST) = 0;
		virtual void visit(ASTMember &AST) = 0;
		virtual void visit(ASTCall &AST) = 0;
		virtual void visit(ASTUnaryOpExpr &AST) = 0;
		virtual void visit(ASTBinaryOpExpr &AST) = 0;
		virtual void visit(ASTTernaryOpExpr &AST) = 0;
		virtual void visit(ASTCast &AST) = 0;
		virtual void visit(ASTBoolValue &AST) = 0;
		virtual void visit(ASTNumberValue &AST) = 0;
		virtual void visit(ASTStringValue &AST) = 0;
		virtual void visit(ASTArrayValue &AST) = 0;
		virtual void visit(ASTStructValue &AST) = 0;
		virtual void visit(ASTNullValue &AST) = 0;
	};

}

#endif //FLY_AST_VISITOR_H