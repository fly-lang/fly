//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "ParserTest.h"
#include "Frontend/FrontendAction.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTParam.h"
#include "AST/ASTLoopBlock.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTLoopBlock.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTClassMethod.h"

#include "llvm/ADT/StringMap.h"

namespace {

    using namespace fly;

    TEST_F(ParserTest, UnaryExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  ++a"
                "  a++"
                "  --a"
                "  a--"
                "  a = ++a + 1"
                "}\n");
        ASTNode *Node = Parse("UnaryExpr", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        // ++a
        ASTExprStmt *a1Stmt = (ASTExprStmt *) Body->getContent()[0];
        ASTUnaryGroupExpr *a1Unary = (ASTUnaryGroupExpr *) a1Stmt->getExpr();
        EXPECT_EQ(a1Unary->getOperatorKind(), ASTUnaryOperatorKind::ARITH_INCR);
        EXPECT_EQ(a1Unary->getOptionKind(), ASTUnaryOptionKind::UNARY_PRE);
        EXPECT_EQ(((ASTVarRefExpr *) a1Unary->getFirst())->getVarRef()->getName(), "a");

        // a++
        ASTExprStmt *a2Stmt = (ASTExprStmt *) Body->getContent()[1];
        ASTUnaryGroupExpr *a2Unary = (ASTUnaryGroupExpr *) a2Stmt->getExpr();
        EXPECT_EQ(a2Unary->getOperatorKind(), ASTUnaryOperatorKind::ARITH_INCR);
        EXPECT_EQ(a2Unary->getOptionKind(), ASTUnaryOptionKind::UNARY_POST);
        EXPECT_EQ(((ASTVarRefExpr *) a2Unary->getFirst())->getVarRef()->getName(), "a");

        // --a
        ASTExprStmt *a3Stmt = (ASTExprStmt *) Body->getContent()[2];
        ASTUnaryGroupExpr *a3Unary = (ASTUnaryGroupExpr *) a3Stmt->getExpr();
        EXPECT_EQ(a3Unary->getOperatorKind(), ASTUnaryOperatorKind::ARITH_DECR);
        EXPECT_EQ(a3Unary->getOptionKind(), ASTUnaryOptionKind::UNARY_PRE);
        EXPECT_EQ(((ASTVarRefExpr *) a3Unary->getFirst())->getVarRef()->getName(), "a");

        // a--
        ASTExprStmt *a4Stmt = (ASTExprStmt *) Body->getContent()[3];
        ASTUnaryGroupExpr *a4Unary = (ASTUnaryGroupExpr *) a4Stmt->getExpr();
        EXPECT_EQ(a4Unary->getOperatorKind(), ASTUnaryOperatorKind::ARITH_DECR);
        EXPECT_EQ(a4Unary->getOptionKind(), ASTUnaryOptionKind::UNARY_POST);
        EXPECT_EQ(((ASTVarRefExpr *) a4Unary->getFirst())->getVarRef()->getName(), "a");

        // a = ++a + 1
        const ASTVarStmt *a5Var = (ASTVarStmt *) Body->getContent()[4];
        EXPECT_EQ(a5Var->getExpr()->getExprKind(), ASTExprKind::EXPR_GROUP);
        ASTBinaryGroupExpr *Group = (ASTBinaryGroupExpr *) a5Var->getExpr();
        EXPECT_EQ(Group->getOperatorKind(), ASTBinaryOperatorKind::ARITH_ADD);
        EXPECT_EQ(Group->getOptionKind(), ASTBinaryOptionKind::BINARY_ARITH);
        const ASTUnaryGroupExpr *E1 = (ASTUnaryGroupExpr *) Group->getFirst();
        EXPECT_EQ(E1->getOperatorKind(), ASTUnaryOperatorKind::ARITH_INCR);
        EXPECT_EQ(E1->getOptionKind(), ASTUnaryOptionKind::UNARY_PRE);
        ASTValueExpr *ValueExpr = (ASTValueExpr *) Group->getSecond();
        EXPECT_EQ(ValueExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(ValueExpr->getValue()->print(), "1");
    }

    TEST_F(ParserTest, IntBinaryArithOperation) {
        llvm::StringRef str = ("int func() {\n"
                               "  int a = 2\n"
                               "  int b = a + b / a - b\n"
                               "  return a\n"
                               "}\n");
        ASTNode *Node = Parse("IntBinaryArithOperation", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *(Node->getNameSpace()->getFunctions().begin())->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        // Test: int a = 2

        const ASTVarStmt *aVar = (ASTVarStmt *) Body->getContent()[0];
        EXPECT_EQ(aVar->getVarRef()->getDef()->getName(), "a");
        EXPECT_EQ(aVar->getExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) aVar->getExpr())->getValue()->print(), "2");

        // Test: int b = a + b / a - b

        const ASTVarStmt *bVar = (ASTVarStmt *) Body->getContent()[1];
        EXPECT_EQ(bVar->getVarRef()->getDef()->getName(), "b");
        EXPECT_EQ(bVar->getExpr()->getExprKind(), ASTExprKind::EXPR_GROUP);
        EXPECT_EQ(((ASTGroupExpr *) bVar->getExpr())->getGroupKind(), ASTExprGroupKind::GROUP_BINARY);
        ASTBinaryGroupExpr *bGroup = (ASTBinaryGroupExpr *) bVar->getExpr();

        // int b = G1 - b
        const ASTBinaryGroupExpr *G1 = (ASTBinaryGroupExpr *) bGroup->getFirst();
        EXPECT_EQ(bGroup->getOperatorKind(), ASTBinaryOperatorKind::ARITH_SUB);
        EXPECT_EQ(((ASTVarRefExpr *) bGroup->getSecond())->getVarRef()->getName(), "b");

        // G1 = a + G2
        EXPECT_EQ(((ASTVarRefExpr *) G1->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G1->getOperatorKind(), ASTBinaryOperatorKind::ARITH_ADD);
        const ASTBinaryGroupExpr *G2 = (ASTBinaryGroupExpr *) G1->getSecond();

        // G2 = b / a
        const ASTVarRefExpr *b = (ASTVarRefExpr *) G2->getFirst();
        EXPECT_EQ(G2->getOperatorKind(), ASTBinaryOperatorKind::ARITH_DIV);
        const ASTVarRefExpr *a = (ASTVarRefExpr *) G2->getSecond();

    }

    TEST_F(ParserTest, FloatBinaryArithOperation) {
        llvm::StringRef str = ("float func() {\n"
                               "  float a = 1.0"
                               "  float b = a * b - a / b"
                               "  return b\n"
                               "}\n");
        ASTNode *Node = Parse("FloatBinaryArithOperation", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        // Test: float a -= 1.0
        const ASTVarStmt *aVar = (ASTVarStmt *) Body->getContent()[0];
        EXPECT_EQ(aVar->getVarRef()->getDef()->getName(), "a");
        EXPECT_EQ(aVar->getExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) aVar->getExpr())->getValue()->print(), "1.0");

        // Test: int b = a * b - a / b
        const ASTVarStmt *bVar = (ASTVarStmt *) Body->getContent()[1];
        EXPECT_EQ(bVar->getVarRef()->getDef()->getName(), "b");
        EXPECT_EQ(bVar->getExpr()->getExprKind(), ASTExprKind::EXPR_GROUP);
        EXPECT_EQ(((ASTGroupExpr *) bVar->getExpr())->getGroupKind(), ASTExprGroupKind::GROUP_BINARY);
        ASTBinaryGroupExpr *bGroup = (ASTBinaryGroupExpr *) bVar->getExpr();

        // int b = G1 - G2
        const ASTBinaryGroupExpr *G1 = (ASTBinaryGroupExpr *) bGroup->getFirst();
        const ASTBinaryGroupExpr *G2 = (ASTBinaryGroupExpr *) bGroup->getSecond();
        EXPECT_EQ(bGroup->getOperatorKind(), ASTBinaryOperatorKind::ARITH_SUB);

        // G1 = a * b
        EXPECT_EQ(((ASTVarRefExpr *) G1->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G1->getOperatorKind(), ASTBinaryOperatorKind::ARITH_MUL);
        EXPECT_EQ(((ASTVarRefExpr *) G1->getSecond())->getVarRef()->getName(), "b");

        // G2 = a / b
        EXPECT_EQ(((ASTVarRefExpr *) G2->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G2->getOperatorKind(), ASTBinaryOperatorKind::ARITH_DIV);
        EXPECT_EQ(((ASTVarRefExpr *) G2->getSecond())->getVarRef()->getName(), "b");

    }

    TEST_F(ParserTest, BoolBinaryLogicOperation) {
        llvm::StringRef str = ("bool func() {\n"
                               "  bool a = true"
                               "  bool b = a || false && a == true\n"
                               "  return b\n"
                               "}\n");
        ASTNode *Node = Parse("BoolBinaryLogicOperation", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        // Test: bool a = true
        const ASTVarStmt *aVar = (ASTVarStmt *) Body->getContent()[0];
        EXPECT_EQ(aVar->getVarRef()->getDef()->getName(), "a");
        EXPECT_EQ(aVar->getExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) aVar->getExpr())->getValue()->print(), "true");

        // Test: bool b = a || false && a == true
        const ASTVarStmt *bVar = (ASTVarStmt *) Body->getContent()[1];
        EXPECT_EQ(bVar->getVarRef()->getDef()->getName(), "b");
        EXPECT_EQ(bVar->getExpr()->getExprKind(), ASTExprKind::EXPR_GROUP);
        EXPECT_EQ(((ASTGroupExpr *) bVar->getExpr())->getGroupKind(), ASTExprGroupKind::GROUP_BINARY);
        ASTBinaryGroupExpr *bGroup = (ASTBinaryGroupExpr *) bVar->getExpr();

        // int b = G1 == true
        const ASTBinaryGroupExpr *G1 = (ASTBinaryGroupExpr *) bGroup->getFirst();
        EXPECT_EQ(bGroup->getOperatorKind(), ASTBinaryOperatorKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) bGroup->getSecond())->getValue()->print(), "true");

        // G1 = G2 && a
        const ASTBinaryGroupExpr *G2 = (ASTBinaryGroupExpr *) G1->getFirst();
        EXPECT_EQ(G1->getOperatorKind(), ASTBinaryOperatorKind::LOGIC_AND);
        EXPECT_EQ(((ASTValueExpr *) G2->getSecond())->getValue()->print(), "false");

        // G2 = a || false
        EXPECT_EQ(((ASTVarRefExpr *) G2->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G2->getOperatorKind(), ASTBinaryOperatorKind::LOGIC_OR);
        EXPECT_EQ(((ASTValueExpr *) G2->getSecond())->getValue()->print(), "false");

    }

    TEST_F(ParserTest, LongBinaryArithOperation) {
        llvm::StringRef str = ("long func() {\n"
                               "  long a = 1\n"
                               "  long b = (a + b) / (a - b)\n"
                               "  return b\n"
                               "}\n");
        ASTNode *Node = Parse("LongBinaryArithOperation", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        // Test: long a = 1

        const ASTVarStmt *aVar = (ASTVarStmt *) Body->getContent()[0];
        EXPECT_EQ(aVar->getVarRef()->getDef()->getName(), "a");
        EXPECT_EQ(aVar->getExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) aVar->getExpr())->getValue()->print(), "1");

        // Test: long b = (a + b) / (a - b)

        const ASTVarStmt *bVar = (ASTVarStmt *) Body->getContent()[1];
        EXPECT_EQ(bVar->getVarRef()->getDef()->getName(), "b");
        EXPECT_EQ(bVar->getExpr()->getExprKind(), ASTExprKind::EXPR_GROUP);
        EXPECT_EQ(((ASTGroupExpr *) bVar->getExpr())->getGroupKind(), ASTExprGroupKind::GROUP_BINARY);
        ASTBinaryGroupExpr *bGroup = (ASTBinaryGroupExpr *) bVar->getExpr();

        // long b = G1 / G2
        const ASTBinaryGroupExpr *G1 = (ASTBinaryGroupExpr *) bGroup->getFirst();
        const ASTBinaryGroupExpr *G2 = (ASTBinaryGroupExpr *) bGroup->getSecond();
        EXPECT_EQ(bGroup->getOperatorKind(), ASTBinaryOperatorKind::ARITH_DIV);

        // G1 = a + b
        EXPECT_EQ(((ASTVarRefExpr *) G1->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G1->getOperatorKind(), ASTBinaryOperatorKind::ARITH_ADD);
        EXPECT_EQ(((ASTVarRefExpr *) G1->getSecond())->getVarRef()->getName(), "b");

        // G2 = a - b
        EXPECT_EQ(((ASTVarRefExpr *) G2->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G2->getOperatorKind(), ASTBinaryOperatorKind::ARITH_SUB);
        EXPECT_EQ(((ASTVarRefExpr *) G2->getSecond())->getVarRef()->getName(), "b");

    }

    TEST_F(ParserTest, CondTernaryOperation) {
        llvm::StringRef str = ("int func(int a) {\n"
                               "  return a==1 ? 1 : a\n"
                               "}\n");
        ASTNode *Node = Parse("CondTernaryOperation", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        ASTParam *a = F->getParams()->getList()[0];
        const ASTBlock *Body = F->getBody();

        ASTReturnStmt *Ret = (ASTReturnStmt *) Body->getContent()[0];
        ASTTernaryGroupExpr *Expr = (ASTTernaryGroupExpr *) Ret->getExpr();
        ASTBinaryGroupExpr *Comp = ((ASTBinaryGroupExpr *) Expr->getFirst());
        EXPECT_EQ(Comp->getOperatorKind(), ASTBinaryOperatorKind::COMP_EQ);
        EXPECT_EQ(((ASTVarRefExpr *) Comp->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(((ASTValueExpr *) Comp->getSecond())->getValue()->print(), "1");
        EXPECT_EQ(((ASTValueExpr *) Expr->getSecond())->getValue()->print(), "1");
        EXPECT_EQ(((ASTVarRefExpr *) Expr->getThird())->getVarRef()->getName(), "a");

    }
}