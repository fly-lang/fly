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
	class ASTAlias;
	class ASTClass;
	class ASTFunction;
	class ASTEnum;
	class ASTComment;
	class ASTCall;
	class ASTValue;
	class ASTVar;
	class ASTRef;
	class ASTNameSpaceRef;
	class ASTTypeRef;
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
	class ASTVarStmt;
	class ASTBlockStmt;
	class ASTValueExpr;
	class ASTVarRefExpr;
	class ASTCallExpr;
	class ASTOpExpr;
	class ASTCastExpr;

	class ASTVisitor {

	public:

		// Top Level Nodes
		virtual void visit(ASTModule &AST) = 0;
		virtual void visit(ASTNameSpace &AST) = 0;
		virtual void visit(ASTImport &AST) = 0;
		virtual void visit(ASTClass& Class) = 0;
		virtual void visit(ASTFunction& Function) = 0;
		virtual void visit(ASTEnum& Enum) = 0;
		virtual void visit(ASTComment &AST) = 0;

		virtual void visit(ASTValue &AST) = 0;
		virtual void visit(ASTVar &AST) = 0;

		virtual void visit(ASTRef &AST) = 0;
		virtual void visit(ASTCall &AST) = 0;
		virtual void visit(ASTNameSpaceRef &AST) = 0;
		virtual void visit(ASTTypeRef &AST) = 0;

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
		virtual void visit(ASTVarStmt &AST) = 0;
		virtual void visit(ASTBlockStmt &AST) = 0;

		virtual void visit(ASTValueExpr &AST) = 0;
		virtual void visit(ASTVarRefExpr &AST) = 0;
		virtual void visit(ASTCallExpr &AST) = 0;
		virtual void visit(ASTOpExpr &AST) = 0;
		virtual void visit(ASTCastExpr &AST) = 0;
	};

}

#endif //FLY_AST_VISITOR_H