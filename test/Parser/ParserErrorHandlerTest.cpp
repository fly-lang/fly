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
#include "AST/ASTValue.h"
#include "AST/ASTFunction.h"
#include <AST/ASTBlockStmt.h>
#include <AST/ASTExprStmt.h>
#include <AST/ASTFailStmt.h>
#include <AST/ASTHandleStmt.h>
#include <AST/ASTCall.h>
#include <AST/ASTAssignStmt.h>
#include <AST/ASTIdentifier.h>

namespace {

    using namespace fly;

TEST_F(ParserTest, HandleFail_err0) {
    llvm::StringRef str = (
                           "void main() {\n"
                           "  error err0 handle { fail true }\n"
                           "}\n");
    ASTModule *Module = Parse("FunctionFail_err0", str);
    ASSERT_FALSE(HasErrorOccurred());

    // Check main function
    ASTFunction *main = As<ASTFunction>(Module->getNodes()[0]);
    ASSERT_TRUE(main != nullptr);
    EXPECT_EQ(main->getName(), "main");
    ASTBlockStmt *Body = main->getBody();
    ASSERT_FALSE(Body->getContent().empty());

    // Check error err0 handle { fail } statement
    auto *HandleStmt = As<ASTHandleStmt>(Body->getContent()[0]);
    ASSERT_TRUE(HandleStmt != nullptr);
    ASSERT_TRUE(HandleStmt->getErrorHandler() != nullptr);
    EXPECT_EQ(As<ASTIdentifier>(HandleStmt->getErrorHandler())->getName(), "err0");

    ASTBlockStmt *HandleBlock = HandleStmt->getHandle();
    ASSERT_TRUE(HandleBlock != nullptr);
    ASSERT_FALSE(HandleBlock->getContent().empty());

    auto *FailStmt = As<ASTFailStmt>(HandleBlock->getContent()[0]);
    ASSERT_TRUE(FailStmt != nullptr);
	ASSERT_TRUE(FailStmt->getExpr() != nullptr);
	auto *Val = As<ASTBoolValue>(FailStmt->getExpr());
	ASSERT_TRUE(Val != nullptr);
	EXPECT_EQ(Val->getValue(), true);
}

TEST_F(ParserTest, HandleFail_err1) {
    llvm::StringRef str = (
                           "void main() {\n"
                           "  error err1 handle { fail 404 }\n"
                           "}\n");
    ASTModule *Module = Parse("FunctionFail_err1", str);
    ASSERT_FALSE(HasErrorOccurred());

    // Check main function
    ASTFunction *main = As<ASTFunction>(Module->getNodes()[0]);
    ASSERT_TRUE(main != nullptr);
    EXPECT_EQ(main->getName(), "main");
    ASTBlockStmt *Body = main->getBody();
    ASSERT_FALSE(Body->getContent().empty());

    // Check error err1 handle { fail 404 } statement
    auto *HandleStmt = As<ASTHandleStmt>(Body->getContent()[0]);
    ASSERT_TRUE(HandleStmt != nullptr);
    ASSERT_TRUE(HandleStmt->getErrorHandler() != nullptr);
    EXPECT_EQ(As<ASTIdentifier>(HandleStmt->getErrorHandler())->getName(), "err1");

    ASTBlockStmt *HandleBlock = HandleStmt->getHandle();
    ASSERT_TRUE(HandleBlock != nullptr);
    ASSERT_FALSE(HandleBlock->getContent().empty());

    auto *FailStmt = As<ASTFailStmt>(HandleBlock->getContent()[0]);
    ASSERT_TRUE(FailStmt != nullptr);
    ASSERT_TRUE(FailStmt->getExpr() != nullptr);
    auto *Val = As<ASTNumberValue>(FailStmt->getExpr());
    ASSERT_TRUE(Val != nullptr);
    EXPECT_EQ(Val->getValue(), "404");
}

TEST_F(ParserTest, HandleFail_err2) {
    llvm::StringRef str = (
                           "void main() {\n"
                           "  error err2 handle { fail \"Error\" }\n"
                           "}\n");
    ASTModule *Module = Parse("FunctionFail_err2", str);
    ASSERT_FALSE(HasErrorOccurred());

    // Check main function
    ASTFunction *main = As<ASTFunction>(Module->getNodes()[0]);
    ASSERT_TRUE(main != nullptr);
    EXPECT_EQ(main->getName(), "main");
    ASTBlockStmt *Body = main->getBody();
    ASSERT_FALSE(Body->getContent().empty());

    // Check error err2 handle { fail "Error" } statement
    auto *HandleStmt = As<ASTHandleStmt>(Body->getContent()[0]);
    ASSERT_TRUE(HandleStmt != nullptr);
    ASSERT_TRUE(HandleStmt->getErrorHandler() != nullptr);
    EXPECT_EQ(As<ASTIdentifier>(HandleStmt->getErrorHandler())->getName(), "err2");

    ASTBlockStmt *HandleBlock = HandleStmt->getHandle();
    ASSERT_TRUE(HandleBlock != nullptr);
    ASSERT_FALSE(HandleBlock->getContent().empty());

    auto *FailStmt = As<ASTFailStmt>(HandleBlock->getContent()[0]);
    ASSERT_TRUE(FailStmt != nullptr);
    ASSERT_TRUE(FailStmt->getExpr() != nullptr);
    auto *Val = As<ASTStringValue>(FailStmt->getExpr());
    ASSERT_TRUE(Val != nullptr);
    EXPECT_EQ(Val->getValue(), "Error");
}

TEST_F(ParserTest, HandleFail_void) {
    llvm::StringRef str = (
                           "void main() {\n"
                           "  handle { fail }\n"
                           "}\n");
    ASTModule *Module = Parse("HandleFail_void", str);
    ASSERT_FALSE(HasErrorOccurred());

    // Check main function
    ASTFunction *main = As<ASTFunction>(Module->getNodes()[0]);
    ASSERT_TRUE(main != nullptr);
    EXPECT_EQ(main->getName(), "main");
    ASSERT_FALSE(main->getBody()->getContent().empty());

    // Check handle { fail } statement
    ASTHandleStmt *HandleStmt = As<ASTHandleStmt>(main->getBody()->getContent()[0]);
    ASSERT_TRUE(HandleStmt != nullptr);
    EXPECT_TRUE(HandleStmt->getErrorHandler() == nullptr);

    ASTBlockStmt *Handle = HandleStmt->getHandle();
    ASSERT_TRUE(Handle != nullptr);
    ASSERT_FALSE(Handle->getContent().empty());

    auto *FailStmt = As<ASTFailStmt>(Handle->getContent()[0]);
    ASSERT_TRUE(FailStmt != nullptr);
    EXPECT_TRUE(FailStmt->getExpr() == nullptr);
}

}