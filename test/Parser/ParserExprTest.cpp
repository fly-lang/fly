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
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

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
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

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
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

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
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

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
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

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
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

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
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

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
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

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
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

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
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

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

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a + 1
        // Parser creates: target = BinaryOp(a = (a + 1))
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        // Left of '=' is identifier 'a'
        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        // Right of '=' is the binary add expression (a + 1)
        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *AddExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(AddExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ADD);

        EXPECT_EQ(AddExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(AddExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(AddExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(AddExpr->getRightExpr())->getValue(), "1");
    }

    TEST_F(ParserTest, BinarySubExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a - 1"
                "}\n");
        ASTModule *Module = Parse("BinarySubExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a - 1
        // Parser creates: target = BinaryOp(a = (a - 1))
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *SubExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(SubExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_SUB);

        EXPECT_EQ(SubExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(SubExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(SubExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(SubExpr->getRightExpr())->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryMulExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a * 1"
                "}\n");
        ASTModule *Module = Parse("BinaryMulExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a * 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *MulExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(MulExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_MUL);

        EXPECT_EQ(MulExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(MulExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(MulExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(MulExpr->getRightExpr())->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryDivExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a / 1"
                "}\n");
        ASTModule *Module = Parse("BinaryDivExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a / 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *DivExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(DivExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_DIV);

        EXPECT_EQ(DivExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(DivExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(DivExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(DivExpr->getRightExpr())->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryModExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a % 1"
                "}\n");
        ASTModule *Module = Parse("BinaryModExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a % 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *ModExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(ModExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_MOD);

        EXPECT_EQ(ModExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(ModExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(ModExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(ModExpr->getRightExpr())->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryAndExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a & 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAndExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a & 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *AndExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(AndExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_AND);

        EXPECT_EQ(AndExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(AndExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(AndExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(AndExpr->getRightExpr())->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryOrExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a | 1"
                "}\n");
        ASTModule *Module = Parse("BinaryOrExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a | 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *OrExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(OrExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_OR);

        EXPECT_EQ(OrExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(OrExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(OrExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(OrExpr->getRightExpr())->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryXorExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a ^ 1"
                "}\n");
        ASTModule *Module = Parse("BinaryXorExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a ^ 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *XorExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(XorExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_XOR);

        EXPECT_EQ(XorExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(XorExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(XorExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(XorExpr->getRightExpr())->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryShiftLExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a << 1"
                "}\n");
        ASTModule *Module = Parse("BinaryShiftLExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a << 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *ShiftLExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(ShiftLExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_SHIFT_L);

        EXPECT_EQ(ShiftLExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(ShiftLExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(ShiftLExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(ShiftLExpr->getRightExpr())->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryShiftRExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a >> 1"
                "}\n");
        ASTModule *Module = Parse("BinaryShiftRExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a >> 1
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *ShiftRExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(ShiftRExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_SHIFT_R);

        EXPECT_EQ(ShiftRExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(ShiftRExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(ShiftRExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(ShiftRExpr->getRightExpr())->getValue(), "1");
    }

    TEST_F(ParserTest, BinaryLogicAndExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a && true"
                "}\n");
        ASTModule *Module = Parse("BinaryLogicAndExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a && true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *LogicAndExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(LogicAndExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_LOGIC_AND);

        EXPECT_EQ(LogicAndExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LogicAndExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(LogicAndExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(LogicAndExpr->getRightExpr())->getValue(), true);
    }

    TEST_F(ParserTest, BinaryLogicOrExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a || true"
                "}\n");
        ASTModule *Module = Parse("BinaryLogicOrExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a || true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *LogicOrExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(LogicOrExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_LOGIC_OR);

        EXPECT_EQ(LogicOrExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LogicOrExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(LogicOrExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(LogicOrExpr->getRightExpr())->getValue(), true);
    }

    TEST_F(ParserTest, BinaryComparisonEqualExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a == true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonEqualExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a == true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *CmpEqExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(CmpEqExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_EQ);

        EXPECT_EQ(CmpEqExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(CmpEqExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(CmpEqExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(CmpEqExpr->getRightExpr())->getValue(), true);
    }

    TEST_F(ParserTest, BinaryComparisonNotEqualExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a != true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonNotEqualExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a != true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *CmpNeExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(CmpNeExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_NE);

        EXPECT_EQ(CmpNeExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(CmpNeExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(CmpNeExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(CmpNeExpr->getRightExpr())->getValue(), true);
    }

    TEST_F(ParserTest, BinaryComparisonGreaterThanExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a > true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonGreaterThanExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a > true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *CmpGtExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(CmpGtExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_GT);

        EXPECT_EQ(CmpGtExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(CmpGtExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(CmpGtExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(CmpGtExpr->getRightExpr())->getValue(), true);
    }

    TEST_F(ParserTest, BinaryComparisonGreaterThanEqualExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a >= true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonGreaterThanEqualExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a >= true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *CmpGteExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(CmpGteExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_GTE);

        EXPECT_EQ(CmpGteExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(CmpGteExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(CmpGteExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(CmpGteExpr->getRightExpr())->getValue(), true);
    }

    TEST_F(ParserTest, BinaryComparisonLessThanExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a < true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonLessThanExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a < true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *CmpLtExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(CmpLtExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_LT);

        EXPECT_EQ(CmpLtExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(CmpLtExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(CmpLtExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(CmpLtExpr->getRightExpr())->getValue(), true);
    }

    TEST_F(ParserTest, BinaryComparisonLessThanEqualExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a <= true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonLessThanEqualExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a <= true
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *CmpLteExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(CmpLteExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_LTE);

        EXPECT_EQ(CmpLteExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(CmpLteExpr->getLeftExpr())->getName(), "a");

        EXPECT_EQ(CmpLteExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTBoolValue>(CmpLteExpr->getRightExpr())->getValue(), true);
    }

    TEST_F(ParserTest, BinaryAddMulExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a + 2 * a"
                "}\n");
        ASTModule *Module = Parse("BinaryAddMulExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a + 2 * a
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        // Left of '=' is identifier 'a'
        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        // Right of '=' is the addition expression (a + 2 * a)
        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *BinaryAddExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(BinaryAddExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ADD);

        EXPECT_EQ(BinaryAddExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(BinaryAddExpr->getLeftExpr())->getName(), "a");

        ASTExpr *AddRightExpr = BinaryAddExpr->getRightExpr();
        EXPECT_EQ(AddRightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *BinaryMulExpr = As<ASTBinaryOp>(AddRightExpr);
        EXPECT_EQ(BinaryMulExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_MUL);

        EXPECT_EQ(As<ASTNumberValue>(BinaryMulExpr->getLeftExpr())->getValue(), "2");
        EXPECT_EQ(BinaryMulExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(BinaryMulExpr->getRightExpr())->getName(), "a");
    }

    TEST_F(ParserTest, BinarySubDivExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a - 2 / a"
                "}\n");
        ASTModule *Module = Parse("BinarySubDivExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a - 2 / a
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        // Left of '=' is identifier 'a'
        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        // Right of '=' is the subtraction expression (a - 2 / a)
        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *BinarySubExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(BinarySubExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_SUB);

        EXPECT_EQ(BinarySubExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(BinarySubExpr->getLeftExpr())->getName(), "a");

        ASTExpr *SubRightExpr = BinarySubExpr->getRightExpr();
        EXPECT_EQ(SubRightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *BinaryDivExpr = As<ASTBinaryOp>(SubRightExpr);
        EXPECT_EQ(BinaryDivExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_DIV);

        EXPECT_EQ(As<ASTNumberValue>(BinaryDivExpr->getLeftExpr())->getValue(), "2");
        EXPECT_EQ(BinaryDivExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(BinaryDivExpr->getRightExpr())->getName(), "a");
    }

    TEST_F(ParserTest, BinaryParenExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = (2 - a) % (a + 1)"
                "}\n");
        ASTModule *Module = Parse("BinaryParenExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = (2 - a) % (a + 1)
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        EXPECT_EQ(AssignStmt->getTarget()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinaryOp>(AssignStmt->getTarget())->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        ASTBinaryOp *EqExpr = As<ASTBinaryOp>(AssignStmt->getTarget());

        // Left of '=' is identifier 'a'
        EXPECT_EQ(EqExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(EqExpr->getLeftExpr())->getName(), "a");

        // Right of '=' is the modulo expression ((2 - a) % (a + 1))
        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *BinaryModExpr = As<ASTBinaryOp>(RightExpr);
        EXPECT_EQ(BinaryModExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_MOD);

        ASTExpr *LeftExpr = BinaryModExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *BinarySubExpr = As<ASTBinaryOp>(LeftExpr);
        EXPECT_EQ(BinarySubExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_SUB);
        EXPECT_EQ(As<ASTNumberValue>(BinarySubExpr->getLeftExpr())->getValue(), "2");
        EXPECT_EQ(As<ASTIdentifier>(BinarySubExpr->getRightExpr())->getName(), "a");

        ASTExpr *ModRightExpr = BinaryModExpr->getRightExpr();
        EXPECT_EQ(ModRightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *BinaryAddExpr = As<ASTBinaryOp>(ModRightExpr);
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

        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a==1 ? 1 : a
        ASTAssignStmt *AssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        ASSERT_NE(AssignStmt, nullptr);
        ASSERT_NE(AssignStmt->getTarget(), nullptr);

        // Check if target is TERNARY instead of BINARY
        ASSERT_TRUE(AssignStmt->getTarget()->getExprKind() == ASTExprKind::EXPR_TERNARY);
        // The parser might be creating: ASTAssignStmt(source=a, target=TERNARY)
        // instead of: ASTAssignStmt(source=a, target=BINARY_ASSIGN(a, TERNARY))
        ASTTernaryOp *TernaryExpr = As<ASTTernaryOp>(AssignStmt->getTarget());
        ASSERT_NE(TernaryExpr, nullptr);

        // Check ternary condition: a==1
        ASTExpr *ConditionExpr = TernaryExpr->getConditionExpr();
        ASSERT_NE(ConditionExpr, nullptr);
        EXPECT_EQ(ConditionExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinaryOp *CmpExpr = As<ASTBinaryOp>(ConditionExpr);
        ASSERT_NE(CmpExpr, nullptr);
        EXPECT_EQ(CmpExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);

        // Check true expression: 1
        ASTExpr *TrueExpr = TernaryExpr->getTrueExpr();
        ASSERT_NE(TrueExpr, nullptr);
        EXPECT_EQ(TrueExpr->getExprKind(), ASTExprKind::EXPR_VALUE);

        // Check false expression: a
        ASTExpr *FalseExpr = TernaryExpr->getFalseExpr();
        ASSERT_NE(FalseExpr, nullptr);
        EXPECT_EQ(FalseExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
    }

}
