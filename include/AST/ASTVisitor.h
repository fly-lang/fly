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

	class ASTVisitor {

	public:

		virtual void visit(ASTModule &AST) = 0;
		virtual void visit(ASTNameSpace &AST) = 0;
		virtual void visit(ASTImport &AST) = 0;
		virtual void visit(ASTClass& Class) = 0;
		virtual void visit(ASTFunction& Function) = 0;
		virtual void visit(ASTEnum& Enum) = 0;
	};

}

#endif //FLY_AST_VISITOR_H