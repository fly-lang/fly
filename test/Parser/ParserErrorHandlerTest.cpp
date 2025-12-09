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

namespace {

    using namespace fly;

TEST_F(ParserTest, HandleFail_err0) {
    llvm::StringRef str = (
                           "void err0() {\n"
                           "  fail\n"
                           "  return false\n"
                           "}\n"
                           "void main() {\n"
                           "  handle err0()\n"
                           "  bool b = false\n"
                           "  error err0 = handle { b = err0() }\n"
                           "}\n");
    ASTModule *Module = Parse("FunctionFail_err0", str);
    ASSERT_FALSE(HasErrorOccurred());

    ASTFunction *err0 = As<ASTFunction>(Module->getNodes()[0]);
    ASSERT_TRUE(err0 != nullptr);
    ASSERT_FALSE(err0->getBody()->getContent().empty());
    ASTFailStmt *Stmt0 = As<ASTFailStmt>(err0->getBody()->getContent()[0]);
    ASSERT_TRUE(Stmt0->getExpr() == nullptr);
}

TEST_F(ParserTest, HandleFail_err1) {
    llvm::StringRef str = (
                           "int err1() {\n"
                           "  fail 404\n"
                           "  return 0\n"
                           "}\n"
                           "void main() {\n"
                           "  int i = 0\n"
                           "  error err1 = handle { i = err1() }\n"
                           "}\n");
    ASTModule *Module = Parse("FunctionFail_err1", str);
    ASSERT_FALSE(HasErrorOccurred());

    ASTFunction *err1 = As<ASTFunction>(Module->getNodes()[0]);
    ASSERT_TRUE(err1 != nullptr);
    ASSERT_FALSE(err1->getBody()->getContent().empty());
    ASTFailStmt *Stmt1 = As<ASTFailStmt>(err1->getBody()->getContent()[0]);
    ASTNumberValue *Val2 = As<ASTNumberValue>(Stmt1->getExpr());
    ASSERT_EQ(Val2->getValue(), "404");
}

TEST_F(ParserTest, HandleFail_err2) {
    llvm::StringRef str = (
                           "string err2() {\n"
                           "  fail \"Error\"\n"
                           "  return \"\"\n"
                           "}\n"
                           "void main() {\n"
                           "  string s = \"\"\n"
                           "  error err2 = handle { s = err2() }\n"
                           "}\n");
    ASTModule *Module = Parse("FunctionFail_err2", str);
    ASSERT_FALSE(HasErrorOccurred());

    ASTFunction *err2 = As<ASTFunction>(Module->getNodes()[0]);
    ASSERT_TRUE(err2 != nullptr);
    ASSERT_FALSE(err2->getBody()->getContent().empty());
    ASTFailStmt *Stmt2 = As<ASTFailStmt>(err2->getBody()->getContent()[0]);
    ASTStringValue *Val3 = As<ASTStringValue>(Stmt2->getExpr());
    ASSERT_EQ(Val3->getValue(), "Error");
}

TEST_F(ParserTest, HandleFail_void) {
    llvm::StringRef str = (
                           "void err0() {\n"
                           "  fail\n"
                           "}\n"
                           "void main() {\n"
                           "  handle err0()\n"
                           "}\n");
    ASTModule *Module = Parse("HandleFail_void", str);
    ASSERT_FALSE(HasErrorOccurred());

    ASTFunction *main = As<ASTFunction>(Module->getNodes()[0]);
    ASSERT_TRUE(main != nullptr);
    ASSERT_FALSE(main->getBody()->getContent().empty());

    ASTHandleStmt *HandleStmt = As<ASTHandleStmt>(main->getBody()->getContent()[0]);
    ASSERT_TRUE(HandleStmt->getErrorHandler() == nullptr);
    ASTBlockStmt *Handle = HandleStmt->getHandle();
    ASSERT_FALSE(Handle->getContent().empty());
    ASTExprStmt *ExprStmt = As<ASTExprStmt>(Handle->getContent()[0]);
    ASSERT_TRUE(As<ASTCall>(ExprStmt->getExpr())->getArgs().empty());
}

}