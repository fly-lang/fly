//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "ParserTest.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTModule.h"
#include "AST/ASTVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTOpExpr.h"
#include "AST/ASTExprStmt.h"

namespace {

using namespace fly;

TEST_F(ParserTest, IfElsifElseStmt) {
	llvm::StringRef str =
		"void func(int a, int b) {\n"
		"  if (a == 1) {"
		"    b = 0"
		"  } elsif (a == 2) {"
		"    b = 1"
		"  } else {"
		"    b = 2"
		"  }"
		"}\n";
	ASTModule *Module = Parse("IfElsifElseStmt", str);
	ASSERT_TRUE(Resolve());

	// Get Body
	ASTFunction *F = Module->getFunctions()[0];
	const ASTBlockStmt *Body = F->getBody();

	// If
	ASTIfStmt *IfStmt = (ASTIfStmt *)Body->getContent()[0];
	EXPECT_EQ(IfStmt->getStmtKind(), ASTStmtKind::STMT_IF);
	ASTBinaryOpExpr *IfCond = (ASTBinaryOpExpr *)IfStmt->getRule();
	EXPECT_EQ(((ASTVarRefExpr *) IfCond->getLeftExpr())->getVarRef()->getName(), "a");
	EXPECT_EQ(IfCond->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_EQ);
	EXPECT_EQ(((ASTIntegerValue *)((ASTValueExpr *) IfCond->getRightExpr())->getValue())->getValue(), "1");
	ASTAssignmentStmt *b_assign_0 = (ASTAssignmentStmt *)((ASTBlockStmt *)IfStmt->getStmt())->getContent()[0];
	EXPECT_EQ(b_assign_0->getVarRef()->getName(), "b");
	EXPECT_EQ(((ASTIntegerValue *)((ASTValueExpr *) b_assign_0->getExpr())->getValue())->getValue(), "0");

	// Elsif
	ASTRuleStmt *ElsifStmt = IfStmt->getElsif()[0];
	ASTBinaryOpExpr *ElsifCond = (ASTBinaryOpExpr *)ElsifStmt->getRule();
	EXPECT_EQ(((ASTVarRefExpr *) ElsifCond->getLeftExpr())->getVarRef()->getName(), "a");
	EXPECT_EQ(ElsifCond->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_EQ);
	EXPECT_EQ(((ASTIntegerValue *)((ASTValueExpr *) ElsifCond->getRightExpr())->getValue())->getValue(), "2");
	ASTAssignmentStmt *b_assign_1 = (ASTAssignmentStmt *)((ASTBlockStmt *)ElsifStmt->getStmt())->getContent()[0];
	EXPECT_EQ(b_assign_1->getVarRef()->getName(), "b");
	EXPECT_EQ(((ASTIntegerValue *)((ASTValueExpr *) b_assign_1->getExpr())->getValue())->getValue(), "1");

	// Else
	ASTStmt *ElseStmt = IfStmt->getElse();
	ASTAssignmentStmt *b_assign_2 = (ASTAssignmentStmt *)((ASTBlockStmt *)ElseStmt)->getContent()[0];
	EXPECT_EQ(b_assign_2->getVarRef()->getName(), "b");
	EXPECT_EQ(((ASTIntegerValue *)((ASTValueExpr *) b_assign_2->getExpr())->getValue())->getValue(), "2");
}

TEST_F(ParserTest, IfElsifElseInlineStmt) {
	llvm::StringRef str =
		"void func(int a, int b) {\n"
		"  if (a == 1) b = 0"
		"  elsif a == 2 b = 1"
		"  else b = 2"
		"}\n";
	ASTModule *Module = Parse("IfElsifElseInlineStmt", str);
	ASSERT_TRUE(Resolve());

	// Get Body
	ASTFunction *F = Module->getFunctions()[0];
	const ASTBlockStmt *Body = F->getBody();

	// If
	ASTIfStmt *IfStmt = (ASTIfStmt *)Body->getContent()[0];
	EXPECT_EQ(IfStmt->getStmtKind(), ASTStmtKind::STMT_IF);
	ASTBinaryOpExpr *IfCond = (ASTBinaryOpExpr *)IfStmt->getRule();
	EXPECT_EQ(((ASTVarRefExpr *) IfCond->getLeftExpr())->getVarRef()->getName(), "a");
	EXPECT_EQ(IfCond->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_EQ);
	EXPECT_EQ(((ASTIntegerValue *)((ASTValueExpr *) IfCond->getRightExpr())->getValue())->getValue(), "1");
	ASTAssignmentStmt *b_assign_0 = (ASTAssignmentStmt *)((ASTBlockStmt *)IfStmt->getStmt())->getContent()[0];
	EXPECT_EQ(b_assign_0->getVarRef()->getName(), "b");
	EXPECT_EQ(((ASTIntegerValue *)((ASTValueExpr *) b_assign_0->getExpr())->getValue())->getValue(), "0");

	// Elsif
	ASTRuleStmt *ElsifStmt = IfStmt->getElsif()[0];
	ASTBinaryOpExpr *ElsifCond = (ASTBinaryOpExpr *)ElsifStmt->getRule();
	EXPECT_EQ(((ASTVarRefExpr *) ElsifCond->getLeftExpr())->getVarRef()->getName(), "a");
	EXPECT_EQ(ElsifCond->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_EQ);
	EXPECT_EQ(((ASTIntegerValue *)((ASTValueExpr *) ElsifCond->getRightExpr())->getValue())->getValue(), "2");
	ASTAssignmentStmt *b_assign_1 = (ASTAssignmentStmt *)((ASTBlockStmt *)ElsifStmt->getStmt())->getContent()[0];
	EXPECT_EQ(b_assign_1->getVarRef()->getName(), "b");
	EXPECT_EQ(((ASTIntegerValue *)((ASTValueExpr *) b_assign_1->getExpr())->getValue())->getValue(), "1");

	// Else
	ASTStmt *ElseStmt = IfStmt->getElse();
	ASTAssignmentStmt *b_assign_2 = (ASTAssignmentStmt *)((ASTBlockStmt *)ElseStmt)->getContent()[0];
	EXPECT_EQ(b_assign_2->getVarRef()->getName(), "b");
	EXPECT_EQ(((ASTIntegerValue *)((ASTValueExpr *) b_assign_2->getExpr())->getValue())->getValue(), "2");
}

TEST_F(ParserTest, SwitchCaseDefaultStmt) {
	llvm::StringRef str =
		"void func(int a) {\n"
		"  switch (a) {"
		"    case 1:"
		"      break"
		"    case 2:"
		"    default:"
		"      return"
		"  }"
		"}\n";
	ASTModule *Module = Parse("SwitchCaseDefaultStmt", str);
	ASSERT_TRUE(Resolve());

	// Get Body
	ASTFunction *F = Module->getFunctions()[0];
	const ASTBlockStmt *Body = F->getBody();

	ASTSwitchStmt *SwitchStmt = (ASTSwitchStmt *)Body->getContent()[0];
	EXPECT_EQ(SwitchStmt->getStmtKind(), ASTStmtKind::STMT_SWITCH);

	ASTRuleStmt *Case0 = SwitchStmt->getCases()[0];
	EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *) Case0->getRule())->getValue())->getValue(), "1");
	EXPECT_EQ(((ASTBlockStmt *) Case0->getStmt())->getContent()[0]->getStmtKind(), ASTStmtKind::STMT_BREAK);

	ASTRuleStmt *Case1 = SwitchStmt->getCases()[1];
	EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *) Case1->getRule())->getValue())->getValue(), "2");
	EXPECT_TRUE(((ASTBlockStmt *) Case1->getStmt())->getContent().empty());

	ASTBlockStmt *Default = (ASTBlockStmt *)SwitchStmt->getDefault();
	EXPECT_EQ(Default->getStmtKind(), ASTStmtKind::STMT_BLOCK);
	EXPECT_EQ(Default->getContent()[0]->getStmtKind(), ASTStmtKind::STMT_RETURN);
}

TEST_F(ParserTest, WhileStmt) {
	llvm::StringRef str =
		"void func(int a) {\n"
		"  while (a==1) {"
		"    a++"
		"  }\n"
		"}\n";
	ASTModule *Module = Parse("WhileStmt", str);
	ASSERT_TRUE(Resolve());

	// Get Body
	ASTFunction *F = Module->getFunctions()[0];
	const ASTBlockStmt *Body = F->getBody();

	ASTLoopStmt *WhileBlock = (ASTLoopStmt *)Body->getContent()[0];
	EXPECT_EQ(WhileBlock->getStmtKind(), ASTStmtKind::STMT_LOOP);
	EXPECT_FALSE(WhileBlock->getRule() == nullptr);
	EXPECT_FALSE(((ASTBlockStmt *) WhileBlock->getStmt())->isEmpty());

	ASTBinaryOpExpr *Cond = (ASTBinaryOpExpr *) WhileBlock->getRule();
	EXPECT_EQ(((ASTVarRefExpr *) Cond->getLeftExpr())->getVarRef()->getName(), "a");
	EXPECT_EQ(Cond->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_EQ);
	EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *) Cond->getRightExpr())->getValue())->getValue(), "1");

}

TEST_F(ParserTest, WhileValueStmt) {
	llvm::StringRef str = "void func(int a) {\n"
		"  while true a++\n"
		"}\n";
	ASTModule *Module = Parse("WhileValueStmt", str);
	ASSERT_TRUE(Resolve());

	// Get Body
	ASTFunction *F = Module->getFunctions()[0];
	const ASTBlockStmt *Body = F->getBody();

	ASTLoopStmt *WhileBlock = (ASTLoopStmt *)Body->getContent()[0];
	EXPECT_EQ(WhileBlock->getStmtKind(), ASTStmtKind::STMT_LOOP);
	EXPECT_EQ(((ASTBoolValue *)((ASTValueExpr *) WhileBlock->getRule())->getValue())->getValue(), true);
	EXPECT_FALSE(((ASTBlockStmt *) WhileBlock->getStmt())->isEmpty());
}

TEST_F(ParserTest, ForStmt) {
	llvm::StringRef str = (
		"private void func(int a) {\n"
		"  for int b = 1, int c = 2; a < 10; b++, --c {"
		"  }"
		"}\n");
	ASTModule *Module = Parse("ForStmt", str);

	ASSERT_TRUE(Resolve());

	// Get Body
	ASTFunction *F = Module->getFunctions()[0];
	const ASTBlockStmt *Body = F->getBody();

	ASTLoopStmt *ForBlock = (ASTLoopStmt *)Body->getContent()[0];
	EXPECT_EQ(ForBlock->getStmtKind(), ASTStmtKind::STMT_LOOP);

	ASTBlockStmt *InitStmt = (ASTBlockStmt *) ForBlock->getInit();
	ASTBlockStmt *LoopStmt = (ASTBlockStmt *) ForBlock->getLoop();
	ASTBlockStmt *PostStmt = (ASTBlockStmt *) ForBlock->getPost();

	// int b = 1
	EXPECT_EQ(((ASTAssignmentStmt *) InitStmt->getContent()[0])->getVarRef()->getName(), "b");
	// int c = 2
	EXPECT_EQ(((ASTAssignmentStmt *) InitStmt->getContent()[1])->getVarRef()->getName(), "c");

	// a < 10
	ASTBinaryOpExpr *Cond = (ASTBinaryOpExpr *) ForBlock->getRule();
	EXPECT_EQ(((ASTVarRefExpr *) Cond->getLeftExpr())->getVarRef()->getName(), "a");
	EXPECT_EQ(Cond->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_LT);
	EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *) Cond->getRightExpr())->getValue())->getValue(), "10");

	// b++
	ASTExprStmt *ExprStmt1 = (ASTExprStmt *)PostStmt->getContent()[0];
	ASTUnaryOpExpr * bIncrExpr = (ASTUnaryOpExpr *) ExprStmt1->getExpr();
	EXPECT_EQ(((ASTVarRefExpr *) bIncrExpr->getExpr())->getVarRef()->getName(), "b");
	EXPECT_EQ(bIncrExpr->getOpKind(), ASTUnaryOpExprKind::OP_UNARY_POST_INCR);

	// --c
	ASTExprStmt *ExprStmt2 = (ASTExprStmt *)PostStmt->getContent()[1];
	ASTUnaryOpExpr * cIncrExpr = (ASTUnaryOpExpr *) ExprStmt2->getExpr();
	EXPECT_EQ(((ASTVarRefExpr *) cIncrExpr->getExpr())->getVarRef()->getName(), "c");
	EXPECT_EQ(cIncrExpr->getOpKind(), ASTUnaryOpExprKind::OP_UNARY_PRE_DECR);

	EXPECT_TRUE(LoopStmt->isEmpty());
}
}