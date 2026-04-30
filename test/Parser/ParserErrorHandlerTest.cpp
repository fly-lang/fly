//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunction.h"
#include "AST/ASTModule.h"
#include "AST/ASTValue.h"
#include "ParserTest.h"

#include <AST/ASTBlockStmt.h>
#include <AST/ASTCall.h>
#include <AST/ASTFailStmt.h>
#include <AST/ASTHandleStmt.h>

namespace {

	using namespace fly;

	TEST_F(ParserTest, HandleFail_err0) {
		llvm::StringRef str = ("main() {\n"
							   "  handle { fail true }\n"
							   "}\n");
		ASTModule *Module = Parse("FunctionFail_err0", str);
		ASSERT_FALSE(HasErrorOccurred());

		// Check main function
		ASTFunction *main = As<ASTFunction>(Module->getNodes()[0]);
		ASSERT_TRUE(main != nullptr);
		EXPECT_EQ(main->getName(), "main");
		ASTBlockStmt *Body = main->getBody();
		ASSERT_FALSE(Body->getContent().empty());

		// Check error handle { fail } statement
		auto *HandleStmt = As<ASTHandleStmt>(Body->getContent()[0]);
		ASSERT_TRUE(HandleStmt != nullptr);

		ASTBlockStmt *HandleBlock = HandleStmt->getHandle();
		ASSERT_TRUE(HandleBlock != nullptr);
		ASSERT_FALSE(HandleBlock->getContent().empty());

		auto *FailStmt = As<ASTFailStmt>(HandleBlock->getContent()[0]);
		ASSERT_TRUE(FailStmt != nullptr);
		ASSERT_TRUE(FailStmt->getFirstExpr() != nullptr);
		auto *Val = As<ASTBoolValue>(FailStmt->getFirstExpr());
		ASSERT_TRUE(Val != nullptr);
		EXPECT_EQ(Val->getValue(), true);
	}

	TEST_F(ParserTest, HandleFail_err1) {
		llvm::StringRef str = ("main() {\n"
							   "  handle { fail 404 }\n"
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

		ASTBlockStmt *HandleBlock = HandleStmt->getHandle();
		ASSERT_TRUE(HandleBlock != nullptr);
		ASSERT_FALSE(HandleBlock->getContent().empty());

		auto *FailStmt = As<ASTFailStmt>(HandleBlock->getContent()[0]);
		ASSERT_TRUE(FailStmt != nullptr);
		ASSERT_TRUE(FailStmt->getFirstExpr() != nullptr);
		auto *Val = As<ASTNumberValue>(FailStmt->getFirstExpr());
		ASSERT_TRUE(Val != nullptr);
		EXPECT_EQ(Val->getValue(), "404");
	}

	TEST_F(ParserTest, HandleFail_err2) {
		llvm::StringRef str = ("main() {\n"
							   "  handle { fail \"Error\" }\n"
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

		ASTBlockStmt *HandleBlock = HandleStmt->getHandle();
		ASSERT_TRUE(HandleBlock != nullptr);
		ASSERT_FALSE(HandleBlock->getContent().empty());

		auto *FailStmt = As<ASTFailStmt>(HandleBlock->getContent()[0]);
		ASSERT_TRUE(FailStmt != nullptr);
		ASSERT_TRUE(FailStmt->getFirstExpr() != nullptr);
		auto *Val = As<ASTStringValue>(FailStmt->getFirstExpr());
		ASSERT_TRUE(Val != nullptr);
		EXPECT_EQ(Val->getValue(), "Error");
	}

	TEST_F(ParserTest, HandleFail_void) {
		llvm::StringRef str = ("main() {\n"
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

		ASTBlockStmt *Handle = HandleStmt->getHandle();
		ASSERT_TRUE(Handle != nullptr);
		ASSERT_FALSE(Handle->getContent().empty());

		auto *FailStmt = As<ASTFailStmt>(Handle->getContent()[0]);
		ASSERT_TRUE(FailStmt != nullptr);
		EXPECT_TRUE(FailStmt->getFirstExpr() == nullptr);
	}

	TEST_F(ParserTest, HandleFail_err3) {
		llvm::StringRef str = (
							   "main() {\n"
							   "  handle { fail 404, \"Not Found\" }\n"
							   "}\n");
		ASTModule *Module = Parse("FunctionFail_err3", str);
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

		ASTBlockStmt *HandleBlock = HandleStmt->getHandle();
		ASSERT_TRUE(HandleBlock != nullptr);
		ASSERT_FALSE(HandleBlock->getContent().empty());

		auto *FailStmt = As<ASTFailStmt>(HandleBlock->getContent()[0]);
		ASSERT_TRUE(FailStmt != nullptr);

		ASSERT_TRUE(FailStmt->getFirstExpr() != nullptr);
		auto *Val = As<ASTStringValue>(FailStmt->getFirstExpr());
		ASSERT_TRUE(Val != nullptr);
		EXPECT_EQ(Val->getValue(), "404");

		ASSERT_TRUE(FailStmt->getSecondExpr() != nullptr);
		auto *Str = As<ASTStringValue>(FailStmt->getSecondExpr());
		ASSERT_TRUE(Str != nullptr);
		EXPECT_EQ(Str->getValue(), "Not Found");
	}

	TEST_F(ParserTest, HandleFail_err4) {
		llvm::StringRef str = (
							   "main() {\n"
							   "  handle { fail new Err() }\n"
							   "}\n");
		ASTModule *Module = Parse("FunctionFail_err4", str);
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

		ASTBlockStmt *HandleBlock = HandleStmt->getHandle();
		ASSERT_TRUE(HandleBlock != nullptr);
		ASSERT_FALSE(HandleBlock->getContent().empty());

		auto *FailStmt = As<ASTFailStmt>(HandleBlock->getContent()[0]);
		ASSERT_TRUE(FailStmt != nullptr);
		ASSERT_TRUE(FailStmt->getFirstExpr() != nullptr);
	}

} // namespace