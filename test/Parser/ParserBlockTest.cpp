//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBinary.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTFunction.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTLoopInStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTUnary.h"
#include "AST/ASTValue.h"
#include "AST/ASTVar.h"
#include "ParserTest.h"

#include <AST/ASTLocalVar.h>

namespace {

using namespace fly;

TEST_F(ParserTest, IfElsifElseStmt) {
	llvm::StringRef str =
		"void func(int x, int y) {\n"
		"  if (a == 1) {\n"
		"    b = 0\n"
		"  } elsif (c == 2) {\n"
		"    d = 3\n"
		"  } else {\n"
		"    e = 4\n"
		"  }\n"
		"}\n";
	ASTModule *Module = Parse("IfElsifElseStmt", str);

	// Get Body
	ASTFunction *F = As<ASTFunction >(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();
	ASSERT_FALSE(Body->getContent().empty());

	// ===== If Statement =====
	ASTIfStmt *IfStmt = As<ASTIfStmt>(Body->getContent()[0]);
	EXPECT_EQ(IfStmt->getStmtKind(), ASTStmtKind::STMT_IF);
	EXPECT_NODE_LOC(IfStmt, "if");

	// If condition: a == 1
	ASTBinary *IfCond = As<ASTBinary>(IfStmt->getRule());
	EXPECT_EQ(IfCond->getBinaryKind(), ASTBinaryKind::OP_BINARY_COMPARE_EQ);
	EXPECT_NODE_LOC(IfCond, "a");

	// Left side: identifier 'a'
	ASTIdentifier *IfCondLeft = As<ASTIdentifier>(IfCond->getLeftExpr());
	EXPECT_EQ(IfCondLeft->getName(), "a");
	EXPECT_NODE_LOC(IfCondLeft, "a");

	// Right side: number value '1'
	ASTNumberValue *IfCondRight = As<ASTNumberValue>(IfCond->getRightExpr());
	EXPECT_EQ(IfCondRight->getValue(), "1");
	EXPECT_NODE_LOC(IfCondRight, "1");

	// If body: b = 0
	ASSERT_FALSE(As<ASTBlockStmt>(IfStmt->getStmt())->getContent().empty());
	auto *b_expr_0 = As<ASTExprStmt>(As<ASTBlockStmt>(IfStmt->getStmt())->getContent()[0]);
	auto *assign_0 = As<ASTBinary>(b_expr_0->getExpr());
	EXPECT_EQ(assign_0->getBinaryKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
	EXPECT_NODE_LOC(assign_0, "b");

	// Left side: identifier 'b'
	ASTIdentifier *Assign0Left = As<ASTIdentifier>(assign_0->getLeftExpr());
	EXPECT_EQ(Assign0Left->getName(), "b");
	EXPECT_NODE_LOC(Assign0Left, "b");

	// Right side: number value '0'
	ASTNumberValue *Assign0Right = As<ASTNumberValue>(assign_0->getRightExpr());
	EXPECT_EQ(Assign0Right->getValue(), "0");
	EXPECT_NODE_LOC(Assign0Right, "0");

	// ===== Elsif Statement =====
	ASTRuleStmt *ElsifStmt = IfStmt->getElsif()[0];
	EXPECT_NODE_LOC(ElsifStmt, "elsif");

	// Elsif condition: c == 2
	ASTBinary *ElsifCond = As<ASTBinary>(ElsifStmt->getRule());
	EXPECT_EQ(ElsifCond->getBinaryKind(), ASTBinaryKind::OP_BINARY_COMPARE_EQ);
	EXPECT_NODE_LOC(ElsifCond, "c");

	// Left side: identifier 'c'
	ASTIdentifier *ElsifCondLeft = As<ASTIdentifier>(ElsifCond->getLeftExpr());
	EXPECT_EQ(ElsifCondLeft->getName(), "c");
	EXPECT_NODE_LOC(ElsifCondLeft, "c");

	// Right side: number value '2'
	ASTNumberValue *ElsifCondRight = As<ASTNumberValue>(ElsifCond->getRightExpr());
	EXPECT_EQ(ElsifCondRight->getValue(), "2");
	EXPECT_NODE_LOC(ElsifCondRight, "2");

	// Elsif body: d = 1
	ASSERT_FALSE(As<ASTBlockStmt>(ElsifStmt->getStmt())->getContent().empty());
	auto *d_expr_1 = As<ASTExprStmt>(As<ASTBlockStmt>(ElsifStmt->getStmt())->getContent()[0]);
	auto *assign_1 = As<ASTBinary>(d_expr_1->getExpr());
	EXPECT_EQ(assign_1->getBinaryKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
	EXPECT_NODE_LOC(assign_1, "d");

	// Left side: identifier 'd'
	ASTIdentifier *Assign1Left = As<ASTIdentifier>(assign_1->getLeftExpr());
	EXPECT_EQ(Assign1Left->getName(), "d");
	EXPECT_NODE_LOC(Assign1Left, "d");

	// Right side: number value '1'
	ASTNumberValue *Assign1Right = As<ASTNumberValue>(assign_1->getRightExpr());
	EXPECT_EQ(Assign1Right->getValue(), "3");
	EXPECT_NODE_LOC(Assign1Right, "3");

	// ===== Else Statement =====
	ASTStmt *ElseStmt = IfStmt->getElse();
	EXPECT_NODE_LOC(ElseStmt, "else");

	// Else body: e = 2
	ASSERT_FALSE(As<ASTBlockStmt>(ElseStmt)->getContent().empty());
	auto *e_expr_2 = As<ASTExprStmt>(As<ASTBlockStmt>(ElseStmt)->getContent()[0]);
	auto *assign_2 = As<ASTBinary>(e_expr_2->getExpr());
	EXPECT_EQ(assign_2->getBinaryKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
	EXPECT_NODE_LOC(assign_2, "e");

	// Left side: identifier 'e'
	ASTIdentifier *Assign2Left = As<ASTIdentifier>(assign_2->getLeftExpr());
	EXPECT_EQ(Assign2Left->getName(), "e");
	EXPECT_NODE_LOC(Assign2Left, "e");

	// Right side: number value '2'
	ASTNumberValue *Assign2Right = As<ASTNumberValue>(assign_2->getRightExpr());
	EXPECT_EQ(Assign2Right->getValue(), "4");
	EXPECT_NODE_LOC(Assign2Right, "4");
}

TEST_F(ParserTest, IfElsifElseInlineStmt) {
	llvm::StringRef str =
		"void func(int x, int y) {\n"
		"  if (a == 1) b = 0\n"
		"  elsif c == 5 d = 7\n"
		"  else e = 9\n"
		"}\n";
	ASTModule *Module = Parse("IfElsifElseInlineStmt", str);

	// Get Body
	ASTFunction *F = As<ASTFunction>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();
	ASSERT_FALSE(Body->getContent().empty());

	// ===== If Statement =====
	ASTIfStmt *IfStmt = As<ASTIfStmt>(Body->getContent()[0]);
	EXPECT_NODE_LOC(IfStmt, "if");
	ASSERT_FALSE(As<ASTBlockStmt>(IfStmt->getStmt())->getContent().empty());

	auto *b_expr_0 = As<ASTExprStmt>(As<ASTBlockStmt>(IfStmt->getStmt())->getContent()[0]);
	auto *assign_0 = As<ASTBinary>(b_expr_0->getExpr());
	EXPECT_EQ(assign_0->getBinaryKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
	EXPECT_NODE_LOC(assign_0, "b");
	EXPECT_EQ(As<ASTIdentifier>(assign_0->getLeftExpr())->getName(), "b");
	EXPECT_NODE_LOC(As<ASTIdentifier>(assign_0->getLeftExpr()), "b");
	EXPECT_EQ(As<ASTNumberValue>(assign_0->getRightExpr())->getValue(), "0");
	EXPECT_NODE_LOC(As<ASTNumberValue>(assign_0->getRightExpr()), "0");

	// ===== Elsif Statement =====
	ASTRuleStmt *ElsifStmt = IfStmt->getElsif()[0];
	EXPECT_NODE_LOC(ElsifStmt, "elsif");
	ASSERT_FALSE(As<ASTBlockStmt>(ElsifStmt->getStmt())->getContent().empty());

	auto *d_expr_1 = As<ASTExprStmt>(As<ASTBlockStmt>(ElsifStmt->getStmt())->getContent()[0]);
	auto *assign_1 = As<ASTBinary>(d_expr_1->getExpr());
	EXPECT_EQ(assign_1->getBinaryKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
	EXPECT_NODE_LOC(assign_1, "d");
	EXPECT_EQ(As<ASTIdentifier>(assign_1->getLeftExpr())->getName(), "d");
	EXPECT_NODE_LOC(As<ASTIdentifier>(assign_1->getLeftExpr()), "d");
	EXPECT_EQ(As<ASTNumberValue>(assign_1->getRightExpr())->getValue(), "7");
	EXPECT_NODE_LOC(As<ASTNumberValue>(assign_1->getRightExpr()), "7");

	// ===== Else Statement =====
	ASTStmt *ElseStmt = IfStmt->getElse();
	EXPECT_NODE_LOC(ElseStmt, "else");
	ASSERT_FALSE(As<ASTBlockStmt>(ElseStmt)->getContent().empty());

	auto *e_expr_2 = As<ASTExprStmt>(As<ASTBlockStmt>(ElseStmt)->getContent()[0]);
	auto *assign_2 = As<ASTBinary>(e_expr_2->getExpr());
	EXPECT_EQ(assign_2->getBinaryKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
	EXPECT_NODE_LOC(assign_2, "e");
	EXPECT_EQ(As<ASTIdentifier>(assign_2->getLeftExpr())->getName(), "e");
	EXPECT_NODE_LOC(As<ASTIdentifier>(assign_2->getLeftExpr()), "e");
	EXPECT_EQ(As<ASTNumberValue>(assign_2->getRightExpr())->getValue(), "9");
	EXPECT_NODE_LOC(As<ASTNumberValue>(assign_2->getRightExpr()), "9");
}

TEST_F(ParserTest, SwitchCaseDefaultStmt) {
	llvm::StringRef str =
		"void func(int x) {\n"
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

	// ===== Switch Statement =====
	ASTSwitchStmt *SwitchStmt = As<ASTSwitchStmt>(Body->getContent()[0]);
	EXPECT_NODE_LOC(SwitchStmt, "switch");

	// Switch expression
	ASTExpr *SwitchExpr = SwitchStmt->getVar();
	EXPECT_NODE_LOC(SwitchExpr, "a");

	// ===== Case 1 =====
	ASSERT_FALSE(SwitchStmt->getCases().empty());
	ASTRuleStmt *Case0 = SwitchStmt->getCases()[0];
	EXPECT_NODE_LOC(Case0, "case");
	EXPECT_EQ(As<ASTNumberValue>(Case0->getRule())->getValue(), "1");
	EXPECT_NODE_LOC(As<ASTNumberValue>(Case0->getRule()), "1");

	ASSERT_FALSE(As<ASTBlockStmt>(Case0->getStmt())->getContent().empty());
	ASTStmt *BreakStmt = As<ASTBlockStmt>(Case0->getStmt())->getContent()[0];
	EXPECT_EQ(BreakStmt->getStmtKind(), ASTStmtKind::STMT_BREAK);
	EXPECT_NODE_LOC(BreakStmt, "break");

	// ===== Case 2 =====
	ASTRuleStmt *Case1 = SwitchStmt->getCases()[1];
	EXPECT_NODE_LOC(Case1, "case", 2);
	EXPECT_EQ(As<ASTNumberValue>(Case1->getRule())->getValue(), "2");
	EXPECT_NODE_LOC(As<ASTNumberValue>(Case1->getRule()), "2");
	EXPECT_TRUE(As<ASTBlockStmt>(Case1->getStmt())->getContent().empty());

	// ===== Default =====
	ASTBlockStmt *Default = As<ASTBlockStmt>(SwitchStmt->getDefault());
	EXPECT_EQ(Default->getStmtKind(), ASTStmtKind::STMT_BLOCK);
	EXPECT_NODE_LOC(Default, "default");

	ASSERT_FALSE(Default->getContent().empty());
	ASTStmt *ReturnStmt = Default->getContent()[0];
	EXPECT_EQ(ReturnStmt->getStmtKind(), ASTStmtKind::STMT_RETURN);
	EXPECT_NODE_LOC(ReturnStmt, "return");
}

TEST_F(ParserTest, WhileStmt) {
	llvm::StringRef str =
		"void func(int x) {\n"
		"  while (a==1) {\n"
		"    b++\n"
		"  }\n"
		"}\n";
	ASTModule *Module = Parse("WhileStmt", str);

	// Get Body
	ASTFunction *F = As<ASTFunction>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();
	ASSERT_FALSE(Body->getContent().empty());

	// ===== While Loop =====
	ASTLoopStmt *WhileBlock = As<ASTLoopStmt>(Body->getContent()[0]);
	EXPECT_EQ(WhileBlock->getStmtKind(), ASTStmtKind::STMT_LOOP);
	EXPECT_NODE_LOC(WhileBlock, "while");
	EXPECT_FALSE(WhileBlock->getRule() == nullptr);
	EXPECT_FALSE(As<ASTBlockStmt>(WhileBlock->getStmt())->isEmpty());

	// Condition: a==1
	ASTBinary *Cond = As<ASTBinary>(WhileBlock->getRule());
	EXPECT_NODE_LOC(Cond, "a");
	EXPECT_EQ(As<ASTIdentifier>(Cond->getLeftExpr())->getName(), "a");
	EXPECT_NODE_LOC(As<ASTIdentifier>(Cond->getLeftExpr()), "a");
	EXPECT_EQ(Cond->getBinaryKind(), ASTBinaryKind::OP_BINARY_COMPARE_EQ);
	EXPECT_EQ(As<ASTNumberValue>(Cond->getRightExpr())->getValue(), "1");
	EXPECT_NODE_LOC(As<ASTNumberValue>(Cond->getRightExpr()), "1");

	// Loop body: b++
	ASTBlockStmt *LoopBody = As<ASTBlockStmt>(WhileBlock->getStmt());
	ASSERT_FALSE(LoopBody->getContent().empty());
	ASTExprStmt *ExprStmt = As<ASTExprStmt>(LoopBody->getContent()[0]);
	ASTUnary *PostIncr = As<ASTUnary>(ExprStmt->getExpr());
	EXPECT_EQ(As<ASTIdentifier>(PostIncr->getExpr())->getName(), "b");
	EXPECT_NODE_LOC(As<ASTIdentifier>(PostIncr->getExpr()), "b");
}

TEST_F(ParserTest, WhileValueStmt) {
	llvm::StringRef str = "void func(int x) {\n"
		"  while true a++\n"
		"}\n";
	ASTModule *Module = Parse("WhileValueStmt", str);

	// Get Body
	ASTFunction *F = As<ASTFunction>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();
	ASSERT_FALSE(Body->getContent().empty());

	// ===== While Loop with value =====
	ASTLoopStmt *WhileBlock = As<ASTLoopStmt>(Body->getContent()[0]);
	EXPECT_EQ(WhileBlock->getStmtKind(), ASTStmtKind::STMT_LOOP);
	EXPECT_NODE_LOC(WhileBlock, "while");
	EXPECT_EQ(As<ASTBoolValue>(WhileBlock->getRule())->getValue(), true);
	EXPECT_NODE_LOC(As<ASTBoolValue>(WhileBlock->getRule()), "true");
	EXPECT_FALSE(As<ASTBlockStmt>(WhileBlock->getStmt())->isEmpty());

	// Loop body: a++
	ASTBlockStmt *LoopBody = As<ASTBlockStmt>(WhileBlock->getStmt());
	ASSERT_FALSE(LoopBody->getContent().empty());
	ASTExprStmt *ExprStmt = As<ASTExprStmt>(LoopBody->getContent()[0]);
	ASTUnary *PostIncr = As<ASTUnary>(ExprStmt->getExpr());
	EXPECT_NODE_LOC(PostIncr->getExpr(), "a");
}

TEST_F(ParserTest, LoopStmt) {
	llvm::StringRef str = (
		"private void func(int x) {\n"
		"  for int b = 1, int c = 2; a < 10; d++, --e {\n"
		"  }\n"
		"}\n");
	ASTModule *Module = Parse("ForStmt", str);

	// Get Body
	ASTFunction *F = As<ASTFunction>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();
	ASSERT_FALSE(Body->getContent().empty());

	// ===== For Loop =====
	ASTLoopStmt *ForBlock = As<ASTLoopStmt>(Body->getContent()[0]);
	EXPECT_EQ(ForBlock->getStmtKind(), ASTStmtKind::STMT_LOOP);
	EXPECT_NODE_LOC(ForBlock, "for");

	// ===== Init section =====
	ASTBlockStmt *InitStmt = As<ASTBlockStmt>(ForBlock->getInit());
	ASTBlockStmt *LoopStmt = As<ASTBlockStmt>(ForBlock->getLoop());
	ASTBlockStmt *PostStmt = As<ASTBlockStmt>(ForBlock->getPost());

	ASSERT_FALSE(InitStmt->getContent().empty());
	ASSERT_TRUE(InitStmt->getContent().size() == 2); // Each var decl+assignment is one ASTDeclStmt

	// int b = 1 (declaration with assignment)
	auto *b_decl_stmt = As<ASTDeclStmt>(InitStmt->getContent()[0]);
	EXPECT_EQ(b_decl_stmt->getLocalVar()->getName(), "b");
	EXPECT_NODE_LOC(b_decl_stmt->getLocalVar(), "b");
	auto *b_expr = As<ASTBinary>(b_decl_stmt->getExpr());
	EXPECT_EQ(b_expr->getBinaryKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
	EXPECT_NODE_LOC(b_expr, "b");
	EXPECT_EQ(As<ASTIdentifier>(b_expr->getLeftExpr())->getName(), "b");
	EXPECT_NODE_LOC(As<ASTIdentifier>(b_expr->getLeftExpr()), "b");
	EXPECT_EQ(As<ASTNumberValue>(b_expr->getRightExpr())->getValue(), "1");
	EXPECT_NODE_LOC(As<ASTNumberValue>(b_expr->getRightExpr()), "1");

	// int c = 2 (declaration with assignment)
	auto *c_decl_stmt = As<ASTDeclStmt>(InitStmt->getContent()[1]);
	EXPECT_EQ(c_decl_stmt->getLocalVar()->getName(), "c");
	EXPECT_NODE_LOC(c_decl_stmt->getLocalVar(), "c");
	auto *c_expr = As<ASTBinary>(c_decl_stmt->getExpr());
	EXPECT_EQ(c_expr->getBinaryKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
	EXPECT_NODE_LOC(c_expr, "c");
	EXPECT_EQ(As<ASTIdentifier>(c_expr->getLeftExpr())->getName(), "c");
	EXPECT_NODE_LOC(As<ASTIdentifier>(c_expr->getLeftExpr()), "c");
	EXPECT_EQ(As<ASTNumberValue>(c_expr->getRightExpr())->getValue(), "2");
	EXPECT_NODE_LOC(As<ASTNumberValue>(c_expr->getRightExpr()), "2");

	// ===== Condition: a < 10 =====
	ASTBinary *Cond = As<ASTBinary>(ForBlock->getRule());
	EXPECT_NODE_LOC(Cond, "a");
	EXPECT_EQ(As<ASTIdentifier>(Cond->getLeftExpr())->getName(), "a");
	EXPECT_NODE_LOC(As<ASTIdentifier>(Cond->getLeftExpr()), "a");
	EXPECT_EQ(Cond->getBinaryKind(), ASTBinaryKind::OP_BINARY_COMPARE_LT);
	EXPECT_EQ(As<ASTNumberValue>(Cond->getRightExpr())->getValue(), "10");
	EXPECT_NODE_LOC(As<ASTNumberValue>(Cond->getRightExpr()), "10");

	// ===== Post section =====
	ASSERT_TRUE(PostStmt->getContent().size() == 2);
	ASSERT_FALSE(PostStmt->getContent().empty());

	// d++
	ASTExprStmt *ExprStmt1 = As<ASTExprStmt>(PostStmt->getContent()[0]);
	ASTUnary *dIncrExpr = As<ASTUnary>(ExprStmt1->getExpr());
	EXPECT_EQ(As<ASTIdentifier>(dIncrExpr->getExpr())->getName(), "d");
	EXPECT_NODE_LOC(As<ASTIdentifier>(dIncrExpr->getExpr()), "d");
	EXPECT_EQ(dIncrExpr->getOpKind(), ASTUnaryKind::OP_UNARY_POST_INCR);

	// --e
	ASTExprStmt *ExprStmt2 = As<ASTExprStmt>(PostStmt->getContent()[1]);
	ASTUnary *eDecrExpr = As<ASTUnary>(ExprStmt2->getExpr());
	EXPECT_EQ(As<ASTIdentifier>(eDecrExpr->getExpr())->getName(), "e");
	EXPECT_NODE_LOC(As<ASTIdentifier>(eDecrExpr->getExpr()), "e");
	EXPECT_EQ(eDecrExpr->getOpKind(), ASTUnaryKind::OP_UNARY_PRE_DECR);

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

	// ===== For-In Loop =====
	ASTLoopInStmt *ForInBlock = As<ASTLoopInStmt>(Body->getContent()[0]);
	EXPECT_EQ(ForInBlock->getStmtKind(), ASTStmtKind::STMT_LOOP_IN);
	EXPECT_NODE_LOC(ForInBlock, "for");

	// Item identifier 'b'
	EXPECT_EQ(As<ASTIdentifier>(ForInBlock->getItem())->getName(), "b");
	EXPECT_NODE_LOC(ForInBlock->getItem(), "b");

	// List identifier 'a' (second occurrence - first is parameter)
	EXPECT_EQ(As<ASTIdentifier>(ForInBlock->getList())->getName(), "a");
	auto ListLoc = LocToLineCol(ForInBlock->getList()->getLocation());
	EXPECT_EQ(ListLoc.first, 2u);  // line 2
	EXPECT_EQ(ListLoc.second, 12u); // col 12 (...in a)

	ASTBlockStmt *LoopStmt = As<ASTBlockStmt>(ForInBlock->getStmt());
	EXPECT_TRUE(LoopStmt->isEmpty());
}
}