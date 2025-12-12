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
#include "AST/ASTIdentifier.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTOp.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTLoopInStmt.h"

namespace {

using namespace fly;

TEST_F(ParserTest, IfElsifElseStmt) {
	llvm::StringRef str =
		"void func(int a, int b) {\n"
		"  if (a == 1) {\n"
		"    b = 0\n"
		"  } elsif (a == 2) {\n"
		"    b = 1\n"
		"  } else {\n"
		"    b = 2\n"
		"  }\n"
		"}\n";
	ASTModule *Module = Parse("IfElsifElseStmt", str);

	// Get Body
	ASTFunction *F = As<ASTFunction >(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();
	ASSERT_FALSE(Body->getContent().empty());

	// If
	ASTIfStmt *IfStmt = As<ASTIfStmt>(Body->getContent()[0]);
	EXPECT_EQ(IfStmt->getStmtKind(), ASTStmtKind::STMT_IF);
	ASTBinaryOp *IfCond = As<ASTBinaryOp>(IfStmt->getRule());
	EXPECT_EQ(As<ASTIdentifier>(IfCond->getLeftExpr())->getName(), "a");
	EXPECT_EQ(IfCond->getOpKind(), ASTBinaryOpKind::OP_BINARY_EQ);
	EXPECT_EQ(As<ASTNumberValue>(IfCond->getRightExpr())->getValue(), "1");
	ASSERT_FALSE(As<ASTBlockStmt>(IfStmt->getStmt())->getContent().empty());
	auto *b_expr_0 = As<ASTExprStmt>(As<ASTBlockStmt>(IfStmt->getStmt())->getContent()[0]);
	auto *assign_0 = As<ASTBinaryOp>(b_expr_0->getExpr());
	EXPECT_EQ(assign_0->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
	EXPECT_EQ(As<ASTIdentifier>(assign_0->getLeftExpr())->getName(), "b");
	EXPECT_EQ(As<ASTNumberValue>(assign_0->getRightExpr())->getValue(), "0");

	// Elsif
	ASTRuleStmt *ElsifStmt = IfStmt->getElsif()[0];
	ASTBinaryOp *ElsifCond = As<ASTBinaryOp>(ElsifStmt->getRule());
	EXPECT_EQ(As<ASTIdentifier>(ElsifCond->getLeftExpr())->getName(), "a");
	EXPECT_EQ(ElsifCond->getOpKind(), ASTBinaryOpKind::OP_BINARY_EQ);
	EXPECT_EQ(As<ASTNumberValue>(ElsifCond->getRightExpr())->getValue(), "2");
	ASSERT_FALSE(As<ASTBlockStmt>(ElsifStmt->getStmt())->getContent().empty());
	auto *b_expr_1 = As<ASTExprStmt>(As<ASTBlockStmt>(ElsifStmt->getStmt())->getContent()[0]);
	auto *assign_1 = As<ASTBinaryOp>(b_expr_1->getExpr());
	EXPECT_EQ(assign_1->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
	EXPECT_EQ(As<ASTIdentifier>(assign_1->getLeftExpr())->getName(), "b");
	EXPECT_EQ(As<ASTNumberValue>(assign_1->getRightExpr())->getValue(), "1");

	// Else
	ASTStmt *ElseStmt = IfStmt->getElse();
	ASSERT_FALSE(As<ASTBlockStmt>(ElseStmt)->getContent().empty());
	auto *b_expr_2 = As<ASTExprStmt>(As<ASTBlockStmt>(ElseStmt)->getContent()[0]);
	auto *assign_2 = As<ASTBinaryOp>(b_expr_2->getExpr());
	EXPECT_EQ(assign_2->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
	EXPECT_EQ(As<ASTIdentifier>(assign_2->getLeftExpr())->getName(), "b");
	EXPECT_EQ(As<ASTNumberValue>(assign_2->getRightExpr())->getValue(), "2");
}

TEST_F(ParserTest, IfElsifElseInlineStmt) {
	llvm::StringRef str =
		"void func(int a, int b) {\n"
		"  if (a == 1) b = 0\n"
		"  elsif a == 2 b = 1\n"
		"  else b = 2\n"
		"}\n";
	ASTModule *Module = Parse("IfElsifElseInlineStmt", str);


	// Get Body
	ASTFunction *F = As<ASTFunction>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();
	ASSERT_FALSE(Body->getContent().empty());

	// If
	ASTIfStmt *IfStmt = As<ASTIfStmt>(Body->getContent()[0]);
	ASSERT_FALSE(As<ASTBlockStmt>(IfStmt->getStmt())->getContent().empty());
	auto *b_expr_0 = As<ASTExprStmt>(As<ASTBlockStmt>(IfStmt->getStmt())->getContent()[0]);
	auto *assign_0 = As<ASTBinaryOp>(b_expr_0->getExpr());
	EXPECT_EQ(assign_0->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
	EXPECT_EQ(As<ASTIdentifier>(assign_0->getLeftExpr())->getName(), "b");
	EXPECT_EQ(As<ASTNumberValue>(assign_0->getRightExpr())->getValue(), "0");

	// Elsif
	ASTRuleStmt *ElsifStmt = IfStmt->getElsif()[0];
	ASSERT_FALSE(As<ASTBlockStmt>(ElsifStmt->getStmt())->getContent().empty());
	auto *b_expr_1 = As<ASTExprStmt>(As<ASTBlockStmt>(ElsifStmt->getStmt())->getContent()[0]);
	auto *assign_1 = As<ASTBinaryOp>(b_expr_1->getExpr());
	EXPECT_EQ(assign_1->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
	EXPECT_EQ(As<ASTIdentifier>(assign_1->getLeftExpr())->getName(), "b");
	EXPECT_EQ(As<ASTNumberValue>(assign_1->getRightExpr())->getValue(), "1");

	// Else
	ASTStmt *ElseStmt = IfStmt->getElse();
	ASSERT_FALSE(As<ASTBlockStmt>(ElseStmt)->getContent().empty());
	auto *b_expr_2 = As<ASTExprStmt>(As<ASTBlockStmt>(ElseStmt)->getContent()[0]);
	auto *assign_2 = As<ASTBinaryOp>(b_expr_2->getExpr());
	EXPECT_EQ(assign_2->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
	EXPECT_EQ(As<ASTIdentifier>(assign_2->getLeftExpr())->getName(), "b");
	EXPECT_EQ(As<ASTNumberValue>(assign_2->getRightExpr())->getValue(), "2");
}

TEST_F(ParserTest, SwitchCaseDefaultStmt) {
	llvm::StringRef str =
		"void func(int a) {\n"
		"  switch (a) {\n"
		"    case 1:\n"
		"      break\n"
		"    case 2:\n"
		"    default:\n"
		"      return\n"
		"  }\n"
		"}\n";
	ASTModule *Module = Parse("SwitchCaseDefaultStmt", str);


	// Get Body
	ASTFunction *F = As<ASTFunction>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();
	ASSERT_FALSE(Body->getContent().empty());

	// Switch
	ASTSwitchStmt *SwitchStmt = As<ASTSwitchStmt>(Body->getContent()[0]);
	ASSERT_FALSE(SwitchStmt->getCases().empty());
	ASTRuleStmt *Case0 = SwitchStmt->getCases()[0];
	EXPECT_EQ(As<ASTNumberValue>(Case0->getRule())->getValue(), "1");
	ASSERT_FALSE(As<ASTBlockStmt>(Case0->getStmt())->getContent().empty());
	EXPECT_EQ(As<ASTBlockStmt>(Case0->getStmt())->getContent()[0]->getStmtKind(), ASTStmtKind::STMT_BREAK);

	ASTRuleStmt *Case1 = SwitchStmt->getCases()[1];
	EXPECT_EQ(As<ASTNumberValue>(Case1->getRule())->getValue(), "2");
	EXPECT_TRUE(As<ASTBlockStmt>(Case1->getStmt())->getContent().empty());

	ASTBlockStmt *Default = As<ASTBlockStmt>(SwitchStmt->getDefault());
	EXPECT_EQ(Default->getStmtKind(), ASTStmtKind::STMT_BLOCK);
	ASSERT_FALSE(Default->getContent().empty());
	EXPECT_EQ(Default->getContent()[0]->getStmtKind(), ASTStmtKind::STMT_RETURN);
}

TEST_F(ParserTest, WhileStmt) {
	llvm::StringRef str =
		"void func(int a) {\n"
		"  while (a==1) {\n"
		"    a++\n"
		"  }\n"
		"}\n";
	ASTModule *Module = Parse("WhileStmt", str);


	// Get Body
	ASTFunction *F = As<ASTFunction>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();
	ASSERT_FALSE(Body->getContent().empty());

	ASTLoopStmt *WhileBlock = As<ASTLoopStmt>(Body->getContent()[0]);
	EXPECT_EQ(WhileBlock->getStmtKind(), ASTStmtKind::STMT_LOOP);
	EXPECT_FALSE(WhileBlock->getRule() == nullptr);
	EXPECT_FALSE(As<ASTBlockStmt>(WhileBlock->getStmt())->isEmpty());

	ASTBinaryOp *Cond = As<ASTBinaryOp>(WhileBlock->getRule());
	EXPECT_EQ(As<ASTIdentifier>(Cond->getLeftExpr())->getName(), "a");
	EXPECT_EQ(Cond->getOpKind(), ASTBinaryOpKind::OP_BINARY_EQ);
	EXPECT_EQ(As<ASTNumberValue>(Cond->getRightExpr())->getValue(), "1");

}

TEST_F(ParserTest, WhileValueStmt) {
	llvm::StringRef str = "void func(int a) {\n"
		"  while true a++\n"
		"}\n";
	ASTModule *Module = Parse("WhileValueStmt", str);


	// Get Body
	ASTFunction *F = As<ASTFunction>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();
	ASSERT_FALSE(Body->getContent().empty());

	ASTLoopStmt *WhileBlock = As<ASTLoopStmt>(Body->getContent()[0]);
	EXPECT_EQ(WhileBlock->getStmtKind(), ASTStmtKind::STMT_LOOP);
	EXPECT_EQ(As<ASTBoolValue>(WhileBlock->getRule())->getValue(), true);
	EXPECT_FALSE(As<ASTBlockStmt>(WhileBlock->getStmt())->isEmpty());
}

TEST_F(ParserTest, LoopStmt) {
	llvm::StringRef str = (
		"private void func(int a) {\n"
		"  for int b = 1, int c = 2; a < 10; b++, --c {\n"
		"  }\n"
		"}\n");
	ASTModule *Module = Parse("ForStmt", str);

	// Get Body
	ASTFunction *F = As<ASTFunction>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();
	ASSERT_FALSE(Body->getContent().empty());

	ASTLoopStmt *ForBlock = As<ASTLoopStmt>(Body->getContent()[0]);
	EXPECT_EQ(ForBlock->getStmtKind(), ASTStmtKind::STMT_LOOP);

	ASTBlockStmt *InitStmt = As<ASTBlockStmt>(ForBlock->getInit());
	ASTBlockStmt *LoopStmt = As<ASTBlockStmt>(ForBlock->getLoop());
	ASTBlockStmt *PostStmt = As<ASTBlockStmt>(ForBlock->getPost());

	ASSERT_FALSE(InitStmt->getContent().empty());
	ASSERT_TRUE(InitStmt->getContent().size() == 2);

	// int b = 1
	auto *b_expr_stmt = As<ASTExprStmt>(InitStmt->getContent()[0]);
	auto *b_expr = As<ASTBinaryOp>(b_expr_stmt->getExpr());
	EXPECT_EQ(b_expr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
	EXPECT_EQ(As<ASTIdentifier>(b_expr->getLeftExpr())->getName(), "b");
	EXPECT_EQ(As<ASTNumberValue>(b_expr->getRightExpr())->getValue(), "1");

	// int c = 2
	auto *c_expr_stmt = As<ASTExprStmt>(InitStmt->getContent()[1]);
	auto *c_expr = As<ASTBinaryOp>(c_expr_stmt->getExpr());
	EXPECT_EQ(c_expr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
	EXPECT_EQ(As<ASTIdentifier>(c_expr->getLeftExpr())->getName(), "c");
	EXPECT_EQ(As<ASTNumberValue>(c_expr->getRightExpr())->getValue(), "2");

	// a < 10
	ASTBinaryOp *Cond = As<ASTBinaryOp>(ForBlock->getRule());
	EXPECT_EQ(As<ASTIdentifier>(Cond->getLeftExpr())->getName(), "a");
	EXPECT_EQ(Cond->getOpKind(), ASTBinaryOpKind::OP_BINARY_LT);
	EXPECT_EQ(As<ASTNumberValue>(Cond->getRightExpr())->getValue(), "10");

	ASSERT_TRUE(PostStmt->getContent().size() == 2);

	// b++
	ASSERT_FALSE(PostStmt->getContent().empty());
	ASTExprStmt *ExprStmt1 = As<ASTExprStmt>(PostStmt->getContent()[0]);
	ASTUnaryOp * bIncrExpr = As<ASTUnaryOp>(ExprStmt1->getExpr());
	EXPECT_EQ(As<ASTIdentifier>(bIncrExpr->getExpr())->getName(), "b");
	EXPECT_EQ(bIncrExpr->getOpKind(), ASTUnaryOpKind::OP_UNARY_POST_INCR);

	// --c
	ASTExprStmt *ExprStmt2 = As<ASTExprStmt>(PostStmt->getContent()[1]);
	ASTUnaryOp * cIncrExpr = As<ASTUnaryOp>(ExprStmt2->getExpr());
	EXPECT_EQ(As<ASTIdentifier>(cIncrExpr->getExpr())->getName(), "c");
	EXPECT_EQ(cIncrExpr->getOpKind(), ASTUnaryOpKind::OP_UNARY_PRE_DECR);

	EXPECT_TRUE(LoopStmt->isEmpty());
}

TEST_F(ParserTest, LoopInStmt) {
	llvm::StringRef str = (
		"private void func(int[] a) {\n"
		"  for b in a {\n"
		"  }\n"
		"}\n");
	ASTModule *Module = Parse("ForInStmt", str);

	// Get Body
	ASTFunction *F = As<ASTFunction>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();
	ASSERT_FALSE(Body->getContent().empty());

	ASTLoopInStmt *ForInBlock = As<ASTLoopInStmt>(Body->getContent()[0]);
	EXPECT_EQ(ForInBlock->getStmtKind(), ASTStmtKind::STMT_LOOP_IN);

	EXPECT_EQ(As<ASTIdentifier>(ForInBlock->getItem())->getName(), "b");
	EXPECT_EQ(As<ASTIdentifier>(ForInBlock->getList())->getName(), "a");

	ASTBlockStmt *LoopStmt = As<ASTBlockStmt>(ForInBlock->getStmt());
	EXPECT_TRUE(LoopStmt->isEmpty());
}
}