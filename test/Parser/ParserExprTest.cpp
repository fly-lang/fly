//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "ParserTest.h"
#include "AST/ASTModule.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTAssignStmt.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTOp.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTExprStmt.h"

#include "llvm/ADT/StringMap.h"

#include <CodeGen/CodeGenExpr.h>

namespace {

    using namespace fly;

    TEST_F(ParserTest, UnaryExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  ++a\n"
                "  a++\n"
                "  --a\n"
                "  a--\n"
                "}\n");
        ASTModule *Module = Parse("UnaryExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // ++a
        auto *a1Stmt = As<ASTExprStmt>(Body->getContent()[0]);
        auto *a1Unary = As<ASTUnaryOp>(a1Stmt->getExpr());
        EXPECT_EQ(a1Unary->getOpKind(), ASTUnaryOpKind::OP_UNARY_PRE_INCR);
        EXPECT_EQ(As<ASTIdentifier>(a1Unary->getExpr())->getName(), "a");

        // a++
        auto *a2Stmt = As<ASTExprStmt>(Body->getContent()[1]);
        auto *a2Unary = As<ASTUnaryOp>(a2Stmt->getExpr());
        EXPECT_EQ(a2Unary->getOpKind(), ASTUnaryOpKind::OP_UNARY_POST_INCR);
        EXPECT_EQ(As<ASTIdentifier>(a2Unary->getExpr())->getName(), "a");

        // --a
        auto *a3Stmt = As<ASTExprStmt>(Body->getContent()[2]);
        auto *a3Unary = As<ASTUnaryOp>(a3Stmt->getExpr());
        EXPECT_EQ(a3Unary->getOpKind(), ASTUnaryOpKind::OP_UNARY_PRE_DECR);
        EXPECT_EQ(As<ASTIdentifier>(a3Unary->getExpr())->getName(), "a");

        // a--
        auto *a4Stmt = As<ASTExprStmt>(Body->getContent()[3]);
        auto *a4Unary = As<ASTUnaryOp>(a4Stmt->getExpr());
        EXPECT_EQ(a4Unary->getOpKind(), ASTUnaryOpKind::OP_UNARY_POST_DECR);
        EXPECT_EQ(As<ASTIdentifier>(a4Unary->getExpr())->getName(), "a");
    }

    TEST_F(ParserTest, UnarySideExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a++ + ++a"
                "}\n");
        ASTModule *Module = Parse("UnaryExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a++ + ++a
        auto *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ADD);
        auto *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_UNARY);
        EXPECT_EQ(As<ASTUnaryOp>(LeftExpr)->getOpKind(), ASTUnaryOpKind::OP_UNARY_POST_INCR);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_UNARY);
        EXPECT_EQ(As<ASTUnaryOp>(RightExpr)->getOpKind(), ASTUnaryOpKind::OP_UNARY_PRE_INCR);
    }

    TEST_F(ParserTest, BinaryAssignAddExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a += 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignAddExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a += 1
        auto *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN_ADD);
        auto *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryAssignSubExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a -= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignSubExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a -= 1
        auto *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN_SUB);
        auto *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryAssignMulExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a *= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignMulExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a *= 1
        auto *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN_MUL);
        auto *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryAssignDivExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a /= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignDivExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a /= 1
        auto *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN_DIV);
        auto *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryAssignModExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a %= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignModExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a %= 1
        auto *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN_MOD);
        auto *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryAssignAndExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a &= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignAndExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a &= 1
        auto *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN_AND);
        auto *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryAssignOrExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a |= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignOrExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a |= 1
        auto *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN_OR);
        auto *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryAssignXorExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a ^= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignXorExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a ^= 1
        auto *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN_XOR);
        auto *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryAssignShiftLExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a <<= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignShiftLExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a <<= 1
        auto *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN_SHIFT_L);
        auto *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryAssignShiftRExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a >>= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignShiftRExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a <<= 1
        auto *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN_SHIFT_R);
        auto *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryAddExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a + 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAddExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a + 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ADD);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinarySubExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a - 1"
                "}\n");
        ASTModule *Module = Parse("BinarySubExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a - 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_SUB);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryMulExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a * 1"
                "}\n");
        ASTModule *Module = Parse("BinaryMulExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a * 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_MUL);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryDivExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a / 1"
                "}\n");
        ASTModule *Module = Parse("BinaryDivExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a / 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_DIV);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryModExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a % 1"
                "}\n");
        ASTModule *Module = Parse("BinaryModExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a % 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_MOD);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryAndExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a & 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAndExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a & 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_AND);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryOrExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a | 1"
                "}\n");
        ASTModule *Module = Parse("BinaryOrExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a | 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_OR);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryXorExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a ^ 1"
                "}\n");
        ASTModule *Module = Parse("BinaryXorExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a | 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_XOR);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryShiftLExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a << 1"
                "}\n");
        ASTModule *Module = Parse("BinaryShiftLExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a << 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_SHIFT_L);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryShiftRExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a >> 1"
                "}\n");
        ASTModule *Module = Parse("BinaryShiftRExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a <<= 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_SHIFT_R);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryLogicAndExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a && true"
                "}\n");
        ASTModule *Module = Parse("BinaryLogicAndExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a && true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_LOGIC_AND);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(RightExpr)->getValue(), true);
    }

    TEST_F(ParserTest, BinaryLogicOrExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a || true"
                "}\n");
        ASTModule *Module = Parse("BinaryLogicOrExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a || true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_LOGIC_OR);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(RightExpr)->getValue(), true);
    }

    TEST_F(ParserTest, BinaryComparisonEqualExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a == true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonEqualExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a == true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_EQ);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(RightExpr)->getValue(), true);
    }

    TEST_F(ParserTest, BinaryComparisonNotEqualExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a != true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonNotEqualExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a != true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_NE);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(RightExpr)->getValue(), true);
    }

    TEST_F(ParserTest, BinaryComparisonGreaterThanExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a > true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonGreaterThanExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a > true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_GT);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(RightExpr)->getValue(), true);
    }

    TEST_F(ParserTest, BinaryComparisonGreaterThanEqualExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a >= true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonGreaterThanEqualExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a >= true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_GTE);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(RightExpr)->getValue(), true);
    }

    TEST_F(ParserTest, BinaryComparisonLessThanExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a < true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonLessThanExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a < true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_LT);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(RightExpr)->getValue(), true);
    }

    TEST_F(ParserTest, BinaryComparisonLessThanEqualExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a <= true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonLessThanEqualExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a <= true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_LTE);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(RightExpr)->getValue(), true);
    }

    TEST_F(ParserTest, BinaryAddMulExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a + 2 * a"
                "}\n");
        ASTModule *Module = Parse("BinaryAddMulExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a + 2 * a
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ADD);
        ASTBinaryOp *BinaryAddExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryAddExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryAddExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *BinaryMulExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(BinaryMulExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_MUL);

        EXPECT_EQ(As<ASTNumberValue>(BinaryMulExpr->getLeftExpr())->getValue(), "2");
        EXPECT_EQ(BinaryMulExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
    }

    TEST_F(ParserTest, BinarySubDivExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a - 2 / a"
                "}\n");
        ASTModule *Module = Parse("BinarySubDivExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a - 2 / a
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_SUB);
        ASTBinaryOp *BinaryAddExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryAddExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryAddExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *BinaryMulExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(BinaryMulExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_DIV);

        EXPECT_EQ(As<ASTNumberValue>(BinaryMulExpr->getLeftExpr())->getValue(), "2");
        EXPECT_EQ(BinaryMulExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
    }

    TEST_F(ParserTest, BinaryParenExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = (2 - a) % (a + 1)"
                "}\n");
        ASTModule *Module = Parse("BinaryParenExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = (2 - a) % (a + 1)
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_MOD);
        ASTBinaryOp *BinaryModExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = BinaryModExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *BinarySubExpr = As<ASTBinaryOp>(LeftExpr);
        EXPECT_EQ(BinarySubExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_SUB);
        EXPECT_EQ(As<ASTNumberValue>(BinarySubExpr->getLeftExpr())->getValue(), "2");
        EXPECT_EQ(As<ASTIdentifier>(BinarySubExpr->getRightExpr())->getName(), "a");

        ASTExpr *RightExpr = BinaryModExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *BinaryAddExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(BinaryAddExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ADD);
        EXPECT_EQ(As<ASTIdentifier>(BinaryAddExpr->getLeftExpr())->getName(), "a");
        EXPECT_EQ(As<ASTNumberValue>(BinaryAddExpr->getRightExpr())->getValue(), "1");
    }

    TEST_F(ParserTest, TernaryExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a==1 ? 1 : a"
                "}\n");
        ASTModule *Module = Parse("TernaryExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a <= true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_TERNARY);
        ASTTernaryOp *TernaryExpr = As<ASTTernaryOp>(AssignStmt->getTarget());

        ASTExpr *ConditionExpr = TernaryExpr->getConditionExpr();
        EXPECT_EQ(ConditionExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *BinaryExpr = As<ASTBinaryOp>(ConditionExpr);

        ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

        ASTExpr *RightExpr = BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(RightExpr)->getValue(), "1");

        ASTExpr *TrueExpr = TernaryExpr->getTrueExpr();
        EXPECT_EQ(TrueExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(TrueExpr)->getValue(), false);

        ASTExpr *FalseExpr = TernaryExpr->getFalseExpr();
        EXPECT_EQ(FalseExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(FalseExpr)->getName(), "a");
    }

}
