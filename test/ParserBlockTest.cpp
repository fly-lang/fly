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
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTParam.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTClassMethod.h"

#include "llvm/ADT/StringMap.h"

namespace {

    using namespace fly;

    TEST_F(ParserTest, IfElsifElseStmt) {
        llvm::StringRef str = (
                         "void func(int a, int b) {\n"
                         "  if (a == 1) {"
                         "    return"
                         "  } elsif (a == 2) {"
                         "    b = 1"
                         "  } else {"
                         "    b = 2"
                         "  }"
                         "}\n");
        ASTModule *Module = Parse("IfElsifElseStmt", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Module->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        // If
        ASTIfBlock *IfBlock = (ASTIfBlock *) Body->getContent()[0];
        EXPECT_EQ(IfBlock->getBlockKind(), ASTBlockKind::BLOCK_IF);
        ASTBinaryGroupExpr *IfCond = (ASTBinaryGroupExpr *) IfBlock->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) IfCond->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(IfCond->getOperatorKind(), ASTBinaryOperatorKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) IfCond->getSecond())->getValue()->print(), "1");
        EXPECT_TRUE(((ASTEmptyExpr *)((ASTReturnStmt *) IfBlock->getContent()[0])->getExpr())->getExprKind() == ASTExprKind::EXPR_EMPTY);
        EXPECT_FALSE(IfBlock->getElsifBlocks().empty());
        EXPECT_TRUE(IfBlock->getElseBlock());

        // Elsif
        ASTElsifBlock *ElsifBlock = IfBlock->getElsifBlocks()[0];
        ASTBinaryGroupExpr *ElsifCond = (ASTBinaryGroupExpr *) ElsifBlock->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) ElsifCond->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(ElsifCond->getOperatorKind(), ASTBinaryOperatorKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) ElsifCond->getSecond())->getValue()->print(), "2");
        EXPECT_EQ(((ASTVarStmt *) ElsifBlock->getContent()[0])->getVarRef()->getName(), "b");

        // Else
        ASTElseBlock *ElseBlock = IfBlock->getElseBlock();
        EXPECT_EQ(ElseBlock->getBlockKind(), ASTBlockKind::BLOCK_ELSE);
        EXPECT_EQ(((ASTVarStmt *)ElseBlock->getContent()[0])->getVarRef()->getName(), "b");

    }

    TEST_F(ParserTest, IfElsifElseInlineStmt) {
        llvm::StringRef str = (
                         "void func(int a) {\n"
                         "  if (a == 1) return"
                         "  elsif a == 2 a = 1"
                         "  else a = 2"
                         "}\n");
        ASTModule *Module = Parse("IfElsifElseInlineStmt", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Module->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        // if
        ASTIfBlock *IfBlock = (ASTIfBlock *) Body->getContent()[0];
        EXPECT_EQ(IfBlock->getBlockKind(), ASTBlockKind::BLOCK_IF);
        ASTBinaryGroupExpr *IfCond = (ASTBinaryGroupExpr *) IfBlock->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) IfCond->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(IfCond->getOperatorKind(), ASTBinaryOperatorKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) IfCond->getSecond())->getValue()->print(), "1");
        EXPECT_TRUE(((ASTEmptyExpr *)((ASTReturnStmt *) IfBlock->getContent()[0])->getExpr())->getExprKind() == ASTExprKind::EXPR_EMPTY);

        EXPECT_FALSE(IfBlock->getElsifBlocks().empty());
        EXPECT_TRUE(IfBlock->getElseBlock());

        // Elsif
        ASTElsifBlock *ElsifBlock = IfBlock->getElsifBlocks()[0];
        EXPECT_EQ(ElsifBlock->getBlockKind(), ASTBlockKind::BLOCK_ELSIF);
        ASTBinaryGroupExpr *ElsifCond = (ASTBinaryGroupExpr *) ElsifBlock->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) ElsifCond->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(ElsifCond->getOperatorKind(), ASTBinaryOperatorKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) ElsifCond->getSecond())->getValue()->print(), "2");

        // Else
        ASTElseBlock *ElseBlock = IfBlock->getElseBlock();
        EXPECT_EQ(ElseBlock->getBlockKind(), ASTBlockKind::BLOCK_ELSE);
        EXPECT_EQ(((ASTVarStmt *) ElseBlock->getContent()[0])->getVarRef()->getName(), "a");

    }

    TEST_F(ParserTest, SwitchCaseDefaultStmt) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  switch (a) {"
                "    case 1:"
                "      break"
                "    case 2:"
                "    default:"
                "      return"
                "  }"
                "}\n");
        ASTModule *Module = Parse("SwitchCaseDefaultStmt", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Module->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        ASTSwitchBlock *SwitchBlock = (ASTSwitchBlock *) Body->getContent()[0];
        EXPECT_EQ(SwitchBlock->getBlockKind(), ASTBlockKind::BLOCK_SWITCH);
        EXPECT_EQ(((ASTValueExpr *) SwitchBlock->getCases()[0]->getExpr())->getValue()->print(), "1");
        EXPECT_EQ(SwitchBlock->getCases()[0]->getContent()[0]->getKind(), ASTStmtKind::STMT_BREAK);
        EXPECT_EQ(((ASTValueExpr *) SwitchBlock->getCases()[1]->getExpr())->getValue()->print(), "2");
        EXPECT_TRUE((ASTExprStmt *) SwitchBlock->getCases()[1]->getContent().empty());
        EXPECT_EQ(SwitchBlock->getDefault()->getBlockKind(), ASTBlockKind::BLOCK_SWITCH_DEFAULT);
        EXPECT_EQ((SwitchBlock->getDefault()->getContent()[0])->getKind(), ASTStmtKind::STMT_RETURN);

    }

    TEST_F(ParserTest, WhileStmt) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  while (a==1) {"
                "    a++"
                "  }\n"
                "}\n");
        ASTModule *Module = Parse("WhileStmt", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Module->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();
        ASTWhileBlock *WhileBlock = (ASTWhileBlock *) Body->getContent()[0];
        EXPECT_EQ(WhileBlock->getBlockKind(), ASTBlockKind::BLOCK_WHILE);
        EXPECT_FALSE(WhileBlock->getCondition() == nullptr);
        EXPECT_FALSE(WhileBlock->isEmpty());

        const ASTBinaryGroupExpr *Cond = (ASTBinaryGroupExpr *) WhileBlock->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) Cond->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(Cond->getOperatorKind(), ASTBinaryOperatorKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) Cond->getSecond())->getValue()->print(), "1");

    }

    TEST_F(ParserTest, WhileValueStmt) {
        llvm::StringRef str = ("void func(int a) {\n"
                               "  while true a++\n"
                               "}\n");
        ASTModule *Module = Parse("WhileValueStmt", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Module->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();
        ASTWhileBlock *WhileBlock = (ASTWhileBlock *) Body->getContent()[0];
        EXPECT_EQ(WhileBlock->getBlockKind(), ASTBlockKind::BLOCK_WHILE);
        EXPECT_EQ(((ASTValueExpr *) WhileBlock->getCondition())->getValue()->print(), "true");
        EXPECT_FALSE(WhileBlock->isEmpty());
    }

    TEST_F(ParserTest, ForStmt) {
        llvm::StringRef str = (
                "private void func(int a) {\n"
                "  for int b = 1, int c = 2; b < 10; b++, --c {"
                "  }"
                "}\n");
        ASTModule *Module = Parse("ForStmt", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Module->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();
        ASTForBlock *ForBlock = (ASTForBlock *) Body->getContent()[0];
        EXPECT_EQ(ForBlock->getBlockKind(), ASTBlockKind::BLOCK_FOR);

        // int b =1
        EXPECT_EQ(((ASTLocalVar *) ForBlock->getContent()[0])->getName(), "b");
        // int c = 2
        EXPECT_EQ(((ASTLocalVar *) ForBlock->getContent()[1])->getName(), "c");

        // b < 10
        ASTBinaryGroupExpr *Cond = (ASTBinaryGroupExpr *) ForBlock->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) Cond->getFirst())->getVarRef()->getName(), "b");
        EXPECT_EQ(Cond->getOperatorKind(), ASTBinaryOperatorKind::COMP_LT);
        EXPECT_EQ(((ASTValueExpr *) Cond->getSecond())->getValue()->print(), "10");

        // b++
        ASTExprStmt * ExprStmt1 = (ASTExprStmt *) ForBlock->getPost()->getContent()[0];
        EXPECT_EQ(((ASTUnaryGroupExpr *) ExprStmt1->getExpr())->getFirst()->getVarRef()->getName(), "b");
        EXPECT_EQ(((ASTUnaryGroupExpr *) ExprStmt1->getExpr())->getOperatorKind(), ASTUnaryOperatorKind::ARITH_INCR);

        // c--
        ASTExprStmt * ExprStmt2 = (ASTExprStmt *) ForBlock->getPost()->getContent()[1];
        EXPECT_EQ(((ASTUnaryGroupExpr *) ExprStmt2->getExpr())->getFirst()->getVarRef()->getName(), "c");
        EXPECT_EQ(((ASTUnaryGroupExpr *) ExprStmt2->getExpr())->getOperatorKind(), ASTUnaryOperatorKind::ARITH_DECR);

        EXPECT_TRUE(ForBlock->getLoop()->isEmpty());
    }
}