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


	// Get Body
	ASTFunction *F = static_cast<ASTFunction *>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();

	// If
	ASTIfStmt *IfStmt = As<ASTIfStmt>(Body->getContent()[0]);
	EXPECT_EQ(IfStmt->getStmtKind(), ASTStmtKind::STMT_IF);
	ASTBinaryOp *IfCond = As<ASTBinaryOp>(IfStmt->getRule());
	EXPECT_EQ(As<ASTIdentifier>(IfCond->getLeftExpr())->getName(), "a");
	EXPECT_EQ(IfCond->getOpKind(), ASTBinaryOpKind::OP_BINARY_EQ);
	EXPECT_EQ(As<ASTNumberValue>(IfCond->getRightExpr())->getValue(), "1");
	ASTAssignStmt *b_assign_0 = As<ASTAssignStmt>(As<ASTBlockStmt>(IfStmt->getStmt())->getContent()[0]);
	EXPECT_EQ(As<ASTIdentifier>(b_assign_0->getSource())->getName(), "b");
	EXPECT_EQ(As<ASTNumberValue>(b_assign_0->getTarget())->getValue(), "0");

	// Elsif
	ASTRuleStmt *ElsifStmt = IfStmt->getElsif()[0];
	ASTBinaryOp *ElsifCond = As<ASTBinaryOp>(ElsifStmt->getRule());
	EXPECT_EQ(As<ASTIdentifier>(ElsifCond->getLeftExpr())->getName(), "a");
	EXPECT_EQ(ElsifCond->getOpKind(), ASTBinaryOpKind::OP_BINARY_EQ);
	EXPECT_EQ(As<ASTNumberValue>(ElsifCond->getRightExpr())->getValue(), "2");
	ASTAssignStmt *b_assign_1 = As<ASTAssignStmt>(As<ASTBlockStmt>(ElsifStmt->getStmt())->getContent()[0]);
	EXPECT_EQ(As<ASTIdentifier>(b_assign_1->getSource())->getName(), "b");
	EXPECT_EQ(As<ASTNumberValue>(b_assign_1->getTarget())->getValue(), "1");

	// Else
	ASTStmt *ElseStmt = IfStmt->getElse();
	ASTAssignStmt *b_assign_2 = As<ASTAssignStmt>(As<ASTBlockStmt>(ElseStmt)->getContent()[0]);
	EXPECT_EQ(As<ASTIdentifier>(b_assign_2->getSource())->getName(), "b");
	EXPECT_EQ(As<ASTNumberValue>(b_assign_2->getTarget())->getValue(), "2");
}

TEST_F(ParserTest, IfElsifElseInlineStmt) {
	llvm::StringRef str =
		"void func(int a, int b) {\n"
		"  if (a == 1) b = 0"
		"  elsif a == 2 b = 1"
		"  else b = 2"
		"}\n";
	ASTModule *Module = Parse("IfElsifElseInlineStmt", str);


	// Get Body
	ASTFunction *F = static_cast<ASTFunction *>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();

	// If
	ASTIfStmt *IfStmt = As<ASTIfStmt>(Body->getContent()[0]);
	EXPECT_EQ(IfStmt->getStmtKind(), ASTStmtKind::STMT_IF);
	ASTBinaryOp *IfCond = As<ASTBinaryOp>(IfStmt->getRule());
	EXPECT_EQ(As<ASTIdentifier>(IfCond->getLeftExpr())->getName(), "a");
	EXPECT_EQ(IfCond->getOpKind(), ASTBinaryOpKind::OP_BINARY_EQ);
	EXPECT_EQ(As<ASTNumberValue>(IfCond->getRightExpr())->getValue(), "1");
	ASTAssignStmt *b_assign_0 = As<ASTAssignStmt>(As<ASTBlockStmt>(IfStmt->getStmt())->getContent()[0]);
	EXPECT_EQ(As<ASTIdentifier>(b_assign_0->getSource())->getName(), "b");
	EXPECT_EQ(As<ASTNumberValue>(b_assign_0->getTarget())->getValue(), "0");

	// Elsif
	ASTRuleStmt *ElsifStmt = IfStmt->getElsif()[0];
	ASTBinaryOp *ElsifCond = As<ASTBinaryOp>(ElsifStmt->getRule());
	EXPECT_EQ(As<ASTIdentifier>(ElsifCond->getLeftExpr())->getName(), "a");
	EXPECT_EQ(ElsifCond->getOpKind(), ASTBinaryOpKind::OP_BINARY_EQ);
	EXPECT_EQ(As<ASTNumberValue>(ElsifCond->getRightExpr())->getValue(), "2");
	ASTAssignStmt *b_assign_1 = As<ASTAssignStmt>(As<ASTBlockStmt>(ElsifStmt->getStmt())->getContent()[0]);
	EXPECT_EQ(As<ASTIdentifier>(b_assign_1->getSource())->getName(), "b");
	EXPECT_EQ(As<ASTNumberValue>(b_assign_1->getTarget())->getValue(), "1");

	// Else
	ASTStmt *ElseStmt = IfStmt->getElse();
	ASTAssignStmt *b_assign_2 = As<ASTAssignStmt>(As<ASTBlockStmt>(ElseStmt)->getContent()[0]);
	EXPECT_EQ(As<ASTIdentifier>(b_assign_2->getSource())->getName(), "b");
	EXPECT_EQ(As<ASTNumberValue>(b_assign_2->getTarget())->getValue(), "2");
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


	// Get Body
	ASTFunction *F = static_cast<ASTFunction *>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();

	ASTSwitchStmt *SwitchStmt = As<ASTSwitchStmt>(Body->getContent()[0]);
	EXPECT_EQ(SwitchStmt->getStmtKind(), ASTStmtKind::STMT_SWITCH);

	ASTRuleStmt *Case0 = SwitchStmt->getCases()[0];
	EXPECT_EQ(As<ASTNumberValue>(Case0->getRule())->getValue(), "1");
	EXPECT_EQ(As<ASTBlockStmt>(Case0->getStmt())->getContent()[0]->getStmtKind(), ASTStmtKind::STMT_BREAK);

	ASTRuleStmt *Case1 = SwitchStmt->getCases()[1];
	EXPECT_EQ(As<ASTNumberValue>(Case1->getRule())->getValue(), "2");
	EXPECT_TRUE(As<ASTBlockStmt>(Case1->getStmt())->getContent().empty());

	ASTBlockStmt *Default = As<ASTBlockStmt>(SwitchStmt->getDefault());
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


	// Get Body
	ASTFunction *F = static_cast<ASTFunction *>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();

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
	ASTFunction *F = static_cast<ASTFunction *>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();

	ASTLoopStmt *WhileBlock = As<ASTLoopStmt>(Body->getContent()[0]);
	EXPECT_EQ(WhileBlock->getStmtKind(), ASTStmtKind::STMT_LOOP);
	EXPECT_EQ(As<ASTBoolValue>(WhileBlock->getRule())->getValue(), true);
	EXPECT_FALSE(As<ASTBlockStmt>(WhileBlock->getStmt())->isEmpty());
}

TEST_F(ParserTest, ForStmt) {
	llvm::StringRef str = (
		"private void func(int a) {\n"
		"  for int b = 1, int c = 2; a < 10; b++, --c {"
		"  }"
		"}\n");
	ASTModule *Module = Parse("ForStmt", str);



	// Get Body
	ASTFunction *F = static_cast<ASTFunction *>(Module->getNodes()[0]);
	ASTBlockStmt *Body = F->getBody();

	ASTLoopStmt *ForBlock = As<ASTLoopStmt>(Body->getContent()[0]);
	EXPECT_EQ(ForBlock->getStmtKind(), ASTStmtKind::STMT_LOOP);

	ASTBlockStmt *InitStmt = As<ASTBlockStmt>(ForBlock->getInit());
	ASTBlockStmt *LoopStmt = As<ASTBlockStmt>(ForBlock->getLoop());
	ASTBlockStmt *PostStmt = As<ASTBlockStmt>(ForBlock->getPost());

	// int b = 1
	EXPECT_EQ(As<ASTIdentifier>(As<ASTAssignStmt>(InitStmt->getContent()[0])->getSource())->getName(), "b");
	// int c = 2
	EXPECT_EQ(As<ASTIdentifier>(As<ASTAssignStmt>(InitStmt->getContent()[1])->getSource())->getName(), "c");

	// a < 10
	ASTBinaryOp *Cond = As<ASTBinaryOp>(ForBlock->getRule());
	EXPECT_EQ(As<ASTIdentifier>(Cond->getLeftExpr())->getName(), "a");
	EXPECT_EQ(Cond->getOpKind(), ASTBinaryOpKind::OP_BINARY_LT);
	EXPECT_EQ(As<ASTNumberValue>(Cond->getRightExpr())->getValue(), "10");

	// b++
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
}