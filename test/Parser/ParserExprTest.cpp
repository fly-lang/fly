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
#include "AST/ASTAssignmentStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTOpExpr.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTExprStmt.h"

#include "llvm/ADT/StringMap.h"

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
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // ++a
        ASTExprStmt *a1Stmt = (ASTExprStmt *) Body->getContent()[0];
        ASTUnaryOpExpr *a1Unary = (ASTUnaryOpExpr *) a1Stmt->getExpr();
        EXPECT_EQ(a1Unary->getOpKind(), ASTUnaryOpExprKind::OP_UNARY_PRE_INCR);
        EXPECT_EQ(((ASTVarRefExpr *) a1Unary->getExpr())->getVarRef()->getName(), "a");

        // a++
        ASTExprStmt *a2Stmt = (ASTExprStmt *) Body->getContent()[1];
        ASTUnaryOpExpr *a2Unary = (ASTUnaryOpExpr *) a2Stmt->getExpr();
        EXPECT_EQ(a2Unary->getOpKind(), ASTUnaryOpExprKind::OP_UNARY_POST_INCR);
        EXPECT_EQ(((ASTVarRefExpr *) a2Unary->getExpr())->getVarRef()->getName(), "a");

        // --a
        ASTExprStmt *a3Stmt = (ASTExprStmt *) Body->getContent()[2];
        ASTUnaryOpExpr *a3Unary = (ASTUnaryOpExpr *) a3Stmt->getExpr();
        EXPECT_EQ(a3Unary->getOpKind(), ASTUnaryOpExprKind::OP_UNARY_PRE_DECR);
        EXPECT_EQ(((ASTVarRefExpr *) a3Unary->getExpr())->getVarRef()->getName(), "a");

        // a--
        ASTExprStmt *a4Stmt = (ASTExprStmt *) Body->getContent()[3];
        ASTUnaryOpExpr *a4Unary = (ASTUnaryOpExpr *) a4Stmt->getExpr();
        EXPECT_EQ(a4Unary->getOpKind(), ASTUnaryOpExprKind::OP_UNARY_POST_DECR);
        EXPECT_EQ(((ASTVarRefExpr *) a4Unary->getExpr())->getVarRef()->getName(), "a");
    }

    TEST_F(ParserTest, UnarySideExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a++ + ++a"
                "}\n");
        ASTModule *Module = Parse("UnaryExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a++ + ++a
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_ADD);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) LeftExpr)->getOpExprKind(), ASTOpExprKind::OP_UNARY);
        EXPECT_EQ(((ASTUnaryOpExpr *) LeftExpr)->getOpKind(), ASTUnaryOpExprKind::OP_UNARY_POST_INCR);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) RightExpr)->getOpExprKind(), ASTOpExprKind::OP_UNARY);
        EXPECT_EQ(((ASTUnaryOpExpr *) RightExpr)->getOpKind(), ASTUnaryOpExprKind::OP_UNARY_PRE_INCR);
    }

    TEST_F(ParserTest, BinaryAssignAddExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a += 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignAddExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a += 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_ASSIGN_ADD);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryAssignSubExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a -= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignSubExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a -= 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_ASSIGN_SUB);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryAssignMulExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a *= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignMulExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a *= 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_ASSIGN_MUL);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryAssignDivExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a /= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignDivExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a /= 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_ASSIGN_DIV);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryAssignModExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a %= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignModExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a %= 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_ASSIGN_MOD);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryAssignAndExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a &= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignAndExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a &= 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_ASSIGN_AND);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryAssignOrExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a |= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignOrExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a |= 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_ASSIGN_OR);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryAssignXorExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a ^= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignXorExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a ^= 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_ASSIGN_XOR);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryAssignShiftLExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a <<= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignShiftLExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a <<= 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_ASSIGN_SHIFT_L);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryAssignShiftRExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a >>= 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAssignShiftRExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a <<= 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_ASSIGN_SHIFT_R);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryAddExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a + 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAddExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a + 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_ADD);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinarySubExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a - 1"
                "}\n");
        ASTModule *Module = Parse("BinarySubExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a - 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_SUB);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryMulExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a * 1"
                "}\n");
        ASTModule *Module = Parse("BinaryMulExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a * 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_MUL);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryDivExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a / 1"
                "}\n");
        ASTModule *Module = Parse("BinaryDivExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a / 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_DIV);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryModExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a % 1"
                "}\n");
        ASTModule *Module = Parse("BinaryModExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a % 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_MOD);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryAndExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a & 1"
                "}\n");
        ASTModule *Module = Parse("BinaryAndExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a & 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_AND);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryOrExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a | 1"
                "}\n");
        ASTModule *Module = Parse("BinaryOrExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a | 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_OR);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryXorExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a ^ 1"
                "}\n");
        ASTModule *Module = Parse("BinaryXorExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a | 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_XOR);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryShiftLExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a << 1"
                "}\n");
        ASTModule *Module = Parse("BinaryShiftLExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a << 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_SHIFT_L);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryShiftRExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a >> 1"
                "}\n");
        ASTModule *Module = Parse("BinaryShiftRExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a << 1
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_SHIFT_R);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
    }

    TEST_F(ParserTest, BinaryLogicAndExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a && true"
                "}\n");
        ASTModule *Module = Parse("BinaryLogicAndExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a && true
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_AND_LOG);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "true");
    }

    TEST_F(ParserTest, BinaryLogicOrExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a || true"
                "}\n");
        ASTModule *Module = Parse("BinaryLogicOrExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a || true
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_OR_LOG);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "true");
    }

    TEST_F(ParserTest, BinaryComparisonEqualExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a == true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonEqualExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a == true
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_EQ);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "true");
    }

    TEST_F(ParserTest, BinaryComparisonNotEqualExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a != true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonNotEqualExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a != true
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_NE);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "true");
    }

    TEST_F(ParserTest, BinaryComparisonGreaterThanExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a > true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonGreaterThanExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a > true
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_GT);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "true");
    }

    TEST_F(ParserTest, BinaryComparisonGreaterThanEqualExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a >= true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonGreaterThanEqualExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a >= true
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_GTE);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "true");
    }

    TEST_F(ParserTest, BinaryComparisonLessThanExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a < true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonLessThanExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a < true
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_LT);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "true");
    }

    TEST_F(ParserTest, BinaryComparisonLessThanEqualExpr) {
        llvm::StringRef str = (
                "void func(bool a) {\n"
                "  a = a <= true"
                "}\n");
        ASTModule *Module = Parse("BinaryComparisonLessThanEqualExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a <= true
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_LTE);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "true");
    }

    TEST_F(ParserTest, BinaryAddMulExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a + 2 * a"
                "}\n");
        ASTModule *Module = Parse("BinaryAddMulExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a + 2 * a
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_ADD);
        ASTBinaryOpExpr *BinaryAddExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryAddExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryAddExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_OP);
        ASTBinaryOpExpr *BinaryMulExpr = (ASTBinaryOpExpr *) RightExpr;
        EXPECT_EQ(BinaryMulExpr->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_MUL);
        
        EXPECT_EQ(((ASTValueExpr *) BinaryMulExpr->getLeftExpr())->getValue()->print(), "2");
        EXPECT_EQ(BinaryMulExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VAR_REF);
    }

    TEST_F(ParserTest, BinarySubDivExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a - 2 / a"
                "}\n");
        ASTModule *Module = Parse("BinarySubDivExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a - 2 / a
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_SUB);
        ASTBinaryOpExpr *BinaryAddExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryAddExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryAddExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_OP);
        ASTBinaryOpExpr *BinaryMulExpr = (ASTBinaryOpExpr *) RightExpr;
        EXPECT_EQ(BinaryMulExpr->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_DIV);

        EXPECT_EQ(((ASTValueExpr *) BinaryMulExpr->getLeftExpr())->getValue()->print(), "2");
        EXPECT_EQ(BinaryMulExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VAR_REF);
    }

    TEST_F(ParserTest, BinaryParenExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = (2 - a) % (a + 1)"
                "}\n");
        ASTModule *Module = Parse("BinaryParenExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = (2 - a) % (a + 1)
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) AssignmentStmt->getExpr())->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_MOD);
        ASTBinaryOpExpr *BinaryModExpr = (ASTBinaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *LeftExpr = BinaryModExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_OP);
        ASTBinaryOpExpr *BinarySubExpr = (ASTBinaryOpExpr *) LeftExpr;
        EXPECT_EQ(BinarySubExpr->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_SUB);
        EXPECT_EQ(((ASTValueExpr *) BinarySubExpr->getLeftExpr())->getValue()->print(), "2");
        EXPECT_EQ(((ASTVarRefExpr *) BinarySubExpr->getRightExpr())->getVarRef()->getDef()->getName(), "a");

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryModExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_OP);
        ASTBinaryOpExpr *BinaryAddExpr = (ASTBinaryOpExpr *) RightExpr;
        EXPECT_EQ(BinaryAddExpr->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_ADD);
        EXPECT_EQ(((ASTVarRefExpr *) BinaryAddExpr->getLeftExpr())->getVarRef()->getDef()->getName(), "a");
        EXPECT_EQ(((ASTValueExpr *) BinaryAddExpr->getRightExpr())->getValue()->print(), "1");
    }

    TEST_F(ParserTest, TernaryExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  a = a==1 ? 1 : a"
                "}\n");
        ASTModule *Module = Parse("TernaryExpr", str);
        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        const ASTBlockStmt *Body = Func->getBody();

        // a = a <= true
        const ASTAssignmentStmt *AssignmentStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(AssignmentStmt->getExpr()->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) AssignmentStmt->getExpr())->getOpExprKind(), ASTOpExprKind::OP_TERNARY);
        ASTTernaryOpExpr *TernaryExpr = (ASTTernaryOpExpr *) AssignmentStmt->getExpr();

        const ASTExpr *ConditionExpr = TernaryExpr->getConditionExpr();
        EXPECT_EQ(ConditionExpr->getExprKind(), ASTExprKind::EXPR_OP);
        EXPECT_EQ(((ASTOpExpr *) ConditionExpr)->getOpExprKind(), ASTOpExprKind::OP_BINARY);
        EXPECT_EQ(((ASTBinaryOpExpr *) ConditionExpr)->getOpKind(), ASTBinaryOpExprKind::OP_BINARY_EQ);
        ASTBinaryOpExpr *BinaryExpr = (ASTBinaryOpExpr *) ConditionExpr;

        const ASTExpr *LeftExpr = BinaryExpr->getLeftExpr();
        EXPECT_EQ(LeftExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);

        const ASTExpr *RightExpr = (ASTValueExpr *) BinaryExpr->getRightExpr();
        EXPECT_EQ(RightExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) RightExpr)->getValue()->print(), "1");
        
        const ASTExpr *TrueExpr = TernaryExpr->getTrueExpr();
        EXPECT_EQ(TrueExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) TrueExpr)->getValue()->print(), "1");

        const ASTExpr *FalseExpr = (ASTValueExpr *) TernaryExpr->getFalseExpr();
        EXPECT_EQ(FalseExpr->getExprKind(), ASTExprKind::EXPR_VAR_REF);
        EXPECT_EQ(((ASTVarRefExpr *) FalseExpr)->getVarRef()->getName(), "a");

    }

}