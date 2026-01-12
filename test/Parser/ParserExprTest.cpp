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
        auto *a1Unary = As<ASTUnary>(a1Stmt->getExpr());
        EXPECT_EQ(a1Unary->getOpKind(), ASTUnaryKind::OP_UNARY_PRE_INCR);
        EXPECT_EQ(As<ASTIdentifier>(a1Unary->getExpr())->getName(), "a");

        // a++
        auto *a2Stmt = As<ASTExprStmt>(Body->getContent()[1]);
        auto *a2Unary = As<ASTUnary>(a2Stmt->getExpr());
        EXPECT_EQ(a2Unary->getOpKind(), ASTUnaryKind::OP_UNARY_POST_INCR);
        EXPECT_EQ(As<ASTIdentifier>(a2Unary->getExpr())->getName(), "a");

        // --a
        auto *a3Stmt = As<ASTExprStmt>(Body->getContent()[2]);
        auto *a3Unary = As<ASTUnary>(a3Stmt->getExpr());
        EXPECT_EQ(a3Unary->getOpKind(), ASTUnaryKind::OP_UNARY_PRE_DECR);
        EXPECT_EQ(As<ASTIdentifier>(a3Unary->getExpr())->getName(), "a");

        // a--
        auto *a4Stmt = As<ASTExprStmt>(Body->getContent()[3]);
        auto *a4Unary = As<ASTUnary>(a4Stmt->getExpr());
        EXPECT_EQ(a4Unary->getOpKind(), ASTUnaryKind::OP_UNARY_POST_DECR);
        EXPECT_EQ(As<ASTIdentifier>(a4Unary->getExpr())->getName(), "a");
    }

    TEST_F(ParserTest, UnarySideExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a++ + ++a\n"
                "}\n");
        ASTModule *Module = Parse("UnaryExpr", str);


        // Get Body
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[0]);
        ASTBlockStmt *Body = Func->getBody();

        // a = a++ + ++a
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        auto *AssignBinaryExpr = As<ASTBinary>(ExprStmt->getExpr());
        EXPECT_EQ(AssignBinaryExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);

        // Left side of assignment is 'a'
        EXPECT_EQ(AssignBinaryExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(AssignBinaryExpr->getLeftExpr())->getName(), "a");

        // Right side of assignment is the ADD expression: a++ + ++a
        ASTExpr *RightExpr = AssignBinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        auto *AddExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(AddExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_ADD);

        // Left side of ADD is a++ (post-increment)
        ASTExpr *AddLeftExpr = AddExpr->getLeftExpr();
        EXPECT_EQ(AddLeftExpr->getExprKind(), ASTExprKind::EXPR_UNARY);
        EXPECT_EQ(As<ASTUnary>(AddLeftExpr)->getOpKind(), ASTUnaryKind::OP_UNARY_POST_INCR);
        EXPECT_EQ(As<ASTIdentifier>(As<ASTUnary>(AddLeftExpr)->getExpr())->getName(), "a");

        // Right side of ADD is ++a (pre-increment)
        ASTExpr *AddRightExpr = AddExpr->getRightExpr();
        EXPECT_EQ(AddRightExpr->getExprKind(), ASTExprKind::EXPR_UNARY);
        EXPECT_EQ(As<ASTUnary>(AddRightExpr)->getOpKind(), ASTUnaryKind::OP_UNARY_PRE_INCR);
        EXPECT_EQ(As<ASTIdentifier>(As<ASTUnary>(AddRightExpr)->getExpr())->getName(), "a");
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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN_ADD);
        auto *BinaryExpr = As<ASTBinary>(ExprStmt->getExpr());

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN_SUB);
        auto *BinaryExpr = As<ASTBinary>(ExprStmt->getExpr());

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN_MUL);
        auto *BinaryExpr = As<ASTBinary>(ExprStmt->getExpr());

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN_DIV);
        auto *BinaryExpr = As<ASTBinary>(ExprStmt->getExpr());

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        auto *BinaryExpr = As<ASTBinary>(ExprStmt->getExpr());
        EXPECT_EQ(BinaryExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN_MOD);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        auto *BinaryExpr = As<ASTBinary>(ExprStmt->getExpr());
        EXPECT_EQ(BinaryExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN_AND);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        auto *BinaryExpr = As<ASTBinary>(ExprStmt->getExpr());
        EXPECT_EQ(BinaryExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN_OR);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        auto *BinaryExpr = As<ASTBinary>(ExprStmt->getExpr());
        EXPECT_EQ(BinaryExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN_XOR);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN_SHIFT_L);
        auto *BinaryExpr = As<ASTBinary>(ExprStmt->getExpr());

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN_SHIFT_R);
        auto *BinaryExpr = As<ASTBinary>(ExprStmt->getExpr());

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        // Left of '=' is identifier 'a'
        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        // Right of '=' is the binary add expression (a + 1)
        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *AddExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(AddExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_ADD);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *SubExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(SubExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_SUB);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *MulExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(MulExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_MUL);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *DivExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(DivExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_DIV);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *ModExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(ModExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_MOD);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *AndExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(AndExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_AND);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *OrExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(OrExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_OR);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *XorExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(XorExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_XOR);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *ShiftLExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(ShiftLExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_SHIFT_L);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *ShiftRExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(ShiftRExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_SHIFT_R);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *LogicAndExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(LogicAndExpr->getOpKind(), ASTBinaryKind::OP_BINARY_LOGIC_AND);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *LogicOrExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(LogicOrExpr->getOpKind(), ASTBinaryKind::OP_BINARY_LOGIC_OR);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *CmpEqExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(CmpEqExpr->getOpKind(), ASTBinaryKind::OP_BINARY_COMPARE_EQ);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *CmpNeExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(CmpNeExpr->getOpKind(), ASTBinaryKind::OP_BINARY_COMPARE_NE);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *CmpGtExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(CmpGtExpr->getOpKind(), ASTBinaryKind::OP_BINARY_COMPARE_GT);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *CmpGteExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(CmpGteExpr->getOpKind(), ASTBinaryKind::OP_BINARY_COMPARE_GTE);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *CmpLtExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(CmpLtExpr->getOpKind(), ASTBinaryKind::OP_BINARY_COMPARE_LT);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *CmpLteExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(CmpLteExpr->getOpKind(), ASTBinaryKind::OP_BINARY_COMPARE_LTE);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        // Left of '=' is identifier 'a'
        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        // Right of '=' is the addition expression (a + 2 * a)
        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *BinaryAddExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(BinaryAddExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_ADD);

        EXPECT_EQ(BinaryAddExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(BinaryAddExpr->getLeftExpr())->getName(), "a");

        ASTExpr *AddRightExpr = BinaryAddExpr->getRightExpr();
        EXPECT_EQ(AddRightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *BinaryMulExpr = As<ASTBinary>(AddRightExpr);
        EXPECT_EQ(BinaryMulExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_MUL);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        // Left of '=' is identifier 'a'
        ASTExpr *LeftExpr = EqExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(LeftExpr)->getName(), "a");

        // Right of '=' is the subtraction expression (a - 2 / a)
        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *BinarySubExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(BinarySubExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_SUB);

        EXPECT_EQ(BinarySubExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(BinarySubExpr->getLeftExpr())->getName(), "a");

        ASTExpr *SubRightExpr = BinarySubExpr->getRightExpr();
        EXPECT_EQ(SubRightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *BinaryDivExpr = As<ASTBinary>(SubRightExpr);
        EXPECT_EQ(BinaryDivExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_DIV);

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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        EXPECT_EQ(ExprStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_BINARY);
        EXPECT_EQ(As<ASTBinary>(ExprStmt->getExpr())->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
        ASTBinary *EqExpr = As<ASTBinary>(ExprStmt->getExpr());

        // Left of '=' is identifier 'a'
        EXPECT_EQ(EqExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
        EXPECT_EQ(As<ASTIdentifier>(EqExpr->getLeftExpr())->getName(), "a");

        // Right of '=' is the modulo expression ((2 - a) % (a + 1))
        ASTExpr *RightExpr = EqExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *BinaryModExpr = As<ASTBinary>(RightExpr);
        EXPECT_EQ(BinaryModExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_MOD);

        ASTExpr *LeftExpr = BinaryModExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *BinarySubExpr = As<ASTBinary>(LeftExpr);
        EXPECT_EQ(BinarySubExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_SUB);
        EXPECT_EQ(As<ASTNumberValue>(BinarySubExpr->getLeftExpr())->getValue(), "2");
        EXPECT_EQ(As<ASTIdentifier>(BinarySubExpr->getRightExpr())->getName(), "a");

        ASTExpr *ModRightExpr = BinaryModExpr->getRightExpr();
        EXPECT_EQ(ModRightExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *BinaryAddExpr = As<ASTBinary>(ModRightExpr);
        EXPECT_EQ(BinaryAddExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ARITH_ADD);
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
        auto *ExprStmt = As<ASTExprStmt>(Body->getContent()[0]);
        ASSERT_NE(ExprStmt, nullptr);
        ASSERT_NE(ExprStmt->getExpr(), nullptr);

        // Check if target is TERNARY instead of BINARY
        ASSERT_TRUE(ExprStmt->getExpr()->getExprKind() == ASTExprKind::EXPR_TERNARY);
        // The parser might be creating: ASTExprStmt(expr=TERNARY)
        // instead of: ASTExprStmt(expr=BINARY_ASSIGN(a, TERNARY))
        ASTTernary *TernaryExpr = As<ASTTernary>(ExprStmt->getExpr());
        ASSERT_NE(TernaryExpr, nullptr);

        // Check ternary condition: a==1
        ASTExpr *ConditionExpr = TernaryExpr->getConditionExpr();
        ASSERT_NE(ConditionExpr, nullptr);
        EXPECT_EQ(ConditionExpr->getExprKind(), ASTExprKind::EXPR_BINARY);
        ASTBinary *CmpExpr = As<ASTBinary>(ConditionExpr);
        ASSERT_NE(CmpExpr, nullptr);
        EXPECT_EQ(CmpExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);

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
