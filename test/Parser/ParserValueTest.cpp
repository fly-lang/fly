//===--------------------------------------------------------------------------------------------------------------===//
// test/Parser/ParserValueTest.cpp - Parser value tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBinary.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTDeclStmt.h"
#include "AST/ASTFunction.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTModule.h"
#include "AST/ASTType.h"
#include "AST/ASTValue.h"
#include "ParserTest.h"

namespace {

    using namespace fly;

    // ─── helpers ────────────────────────────────────────────────────────────

    // Return the right-hand ASTValue of the N-th declaration statement in Body.
    static ASTValue *rhsValue(ASTBlockStmt *Body, size_t N) {
        auto *Stmt = static_cast<ASTDeclStmt *>(Body->getContent()[N]);
        auto *Assign = static_cast<ASTBinary *>(Stmt->getExpr());
        return static_cast<ASTValue *>(Assign->getRightExpr());
    }

    // ─── bool ───────────────────────────────────────────────────────────────

    TEST_F(ParserTest, ValueBoolTrue) {
        llvm::StringRef src =
            "func() {\n"
            "  bool a = true\n"
            "}\n";
        ASTModule *M = Parse("ValueBoolTrue", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *Val = As<ASTBoolValue>(rhsValue(Body, 0));
        ASSERT_TRUE(Val != nullptr);
        EXPECT_EQ(Val->getValueKind(), ASTValueKind::VAL_BOOL);
        EXPECT_EQ(Val->getValue(), true);
        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueBoolFalse) {
        llvm::StringRef src =
            "func() {\n"
            "  bool a = false\n"
            "}\n";
        ASTModule *M = Parse("ValueBoolFalse", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *Val = As<ASTBoolValue>(rhsValue(Body, 0));
        ASSERT_TRUE(Val != nullptr);
        EXPECT_EQ(Val->getValueKind(), ASTValueKind::VAL_BOOL);
        EXPECT_EQ(Val->getValue(), false);
        EXPECT_FALSE(HasErrorOccurred());
    }

    // ─── null / unset ────────────────────────────────────────────────────────

    TEST_F(ParserTest, ValueNull) {
        llvm::StringRef src =
            "func() {\n"
            "  Type a = null\n"
            "}\n";
        ASTModule *M = Parse("ValueNull", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *Val = As<ASTNullValue>(rhsValue(Body, 0));
        ASSERT_TRUE(Val != nullptr);
        EXPECT_EQ(Val->getValueKind(), ASTValueKind::VAL_NULL);
        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueUnset) {
        llvm::StringRef src =
            "func() {\n"
            "  EnumType e = unset\n"
            "}\n";
        ASTModule *M = Parse("ValueUnset", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *Val = As<ASTUnsetValue>(rhsValue(Body, 0));
        ASSERT_TRUE(Val != nullptr);
        EXPECT_EQ(Val->getValueKind(), ASTValueKind::VAL_UNSET);
        EXPECT_FALSE(HasErrorOccurred());
    }

    // ─── integer numeric forms ───────────────────────────────────────────────

    TEST_F(ParserTest, ValueIntegerDecimal) {
        llvm::StringRef src =
            "func() {\n"
            "  int a = 0\n"
            "  int b = 42\n"
            "  int c = 1000000\n"
            "}\n";
        ASTModule *M = Parse("ValueIntegerDecimal", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *a = As<ASTNumberValue>(rhsValue(Body, 0));
        ASSERT_TRUE(a != nullptr);
        EXPECT_EQ(a->getValueKind(), ASTValueKind::VAL_NUMBER);
        EXPECT_EQ(a->getValue(), "0");

        auto *b = As<ASTNumberValue>(rhsValue(Body, 1));
        ASSERT_TRUE(b != nullptr);
        EXPECT_EQ(b->getValue(), "42");

        auto *c = As<ASTNumberValue>(rhsValue(Body, 2));
        ASSERT_TRUE(c != nullptr);
        EXPECT_EQ(c->getValue(), "1000000");

        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueIntegerHex) {
        llvm::StringRef src =
            "func() {\n"
            "  int a = 0xFF\n"
            "  int b = 0x0\n"
            "  int c = 0xDEADBEEF\n"
            "}\n";
        ASTModule *M = Parse("ValueIntegerHex", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 0))->getValue(), "0xFF");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 1))->getValue(), "0x0");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 2))->getValue(), "0xDEADBEEF");
        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueIntegerBinary) {
        llvm::StringRef src =
            "func() {\n"
            "  int a = 0b0\n"
            "  int b = 0b1010\n"
            "  int c = 0b11111111\n"
            "}\n";
        ASTModule *M = Parse("ValueIntegerBinary", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 0))->getValue(), "0b0");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 1))->getValue(), "0b1010");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 2))->getValue(), "0b11111111");
        EXPECT_FALSE(HasErrorOccurred());
    }

    // ─── float numeric forms ─────────────────────────────────────────────────

    TEST_F(ParserTest, ValueFloatBasic) {
        llvm::StringRef src =
            "func() {\n"
            "  float a = 0.0\n"
            "  float b = 1.5\n"
            "  double c = 3.14159\n"
            "}\n";
        ASTModule *M = Parse("ValueFloatBasic", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 0))->getValue(), "0.0");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 1))->getValue(), "1.5");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 2))->getValue(), "3.14159");
        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueFloatScientific) {
        llvm::StringRef src =
            "func() {\n"
            "  double a = 1.0e10\n"
            "  double b = 1.5e-3\n"
            "  double c = 6.022E23\n"
            "}\n";
        ASTModule *M = Parse("ValueFloatScientific", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 0))->getValue(), "1.0e10");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 1))->getValue(), "1.5e-3");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 2))->getValue(), "6.022E23");
        EXPECT_FALSE(HasErrorOccurred());
    }

    // ─── char literals ───────────────────────────────────────────────────────

    TEST_F(ParserTest, ValueCharEmpty) {
        llvm::StringRef src =
            "func() {\n"
            "  byte a = ''\n"
            "}\n";
        ASTModule *M = Parse("ValueCharEmpty", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *Val = As<ASTStringValue>(rhsValue(Body, 0));
        ASSERT_TRUE(Val != nullptr);
        EXPECT_EQ(Val->getValueKind(), ASTValueKind::VAL_STRING);
        EXPECT_EQ(Val->getValue(), "");
        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueCharSingle) {
        llvm::StringRef src =
            "func() {\n"
            "  byte a = 'a'\n"
            "  byte b = 'Z'\n"
            "  byte c = '0'\n"
            "}\n";
        ASTModule *M = Parse("ValueCharSingle", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        EXPECT_EQ(As<ASTStringValue>(rhsValue(Body, 0))->getValue(), "a");
        EXPECT_EQ(As<ASTStringValue>(rhsValue(Body, 1))->getValue(), "Z");
        EXPECT_EQ(As<ASTStringValue>(rhsValue(Body, 2))->getValue(), "0");
        EXPECT_FALSE(HasErrorOccurred());
    }

    // ─── string literals ─────────────────────────────────────────────────────

    TEST_F(ParserTest, ValueStringEmpty) {
        llvm::StringRef src =
            "func() {\n"
            "  string a = \"\"\n"
            "}\n";
        ASTModule *M = Parse("ValueStringEmpty", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *Val = As<ASTStringValue>(rhsValue(Body, 0));
        ASSERT_TRUE(Val != nullptr);
        EXPECT_EQ(Val->getValueKind(), ASTValueKind::VAL_STRING);
        EXPECT_EQ(Val->getValue(), "");
        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueStringContent) {
        llvm::StringRef src =
            "func() {\n"
            "  string a = \"hello\"\n"
            "  string b = \"fly lang\"\n"
            "  string c = \"1234\"\n"
            "}\n";
        ASTModule *M = Parse("ValueStringContent", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        EXPECT_EQ(As<ASTStringValue>(rhsValue(Body, 0))->getValue(), "hello");
        EXPECT_EQ(As<ASTStringValue>(rhsValue(Body, 1))->getValue(), "fly lang");
        EXPECT_EQ(As<ASTStringValue>(rhsValue(Body, 2))->getValue(), "1234");
        EXPECT_FALSE(HasErrorOccurred());
    }

    // ─── array values ────────────────────────────────────────────────────────

    TEST_F(ParserTest, ValueArrayEmpty) {
        llvm::StringRef src =
            "func() {\n"
            "  int[] a = {}\n"
            "}\n";
        ASTModule *M = Parse("ValueArrayEmpty", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *Val = As<ASTArrayValue>(rhsValue(Body, 0));
        ASSERT_TRUE(Val != nullptr);
        EXPECT_EQ(Val->getValueKind(), ASTValueKind::VAL_ARRAY);
        EXPECT_TRUE(Val->empty());
        EXPECT_EQ(Val->size(), 0u);
        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueArrayIntegers) {
        llvm::StringRef src =
            "func() {\n"
            "  int[] a = {1, 2, 3}\n"
            "}\n";
        ASTModule *M = Parse("ValueArrayIntegers", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *Val = As<ASTArrayValue>(rhsValue(Body, 0));
        ASSERT_TRUE(Val != nullptr);
        EXPECT_EQ(Val->size(), 3u);
        EXPECT_EQ(As<ASTNumberValue>(Val->getValues()[0])->getValue(), "1");
        EXPECT_EQ(As<ASTNumberValue>(Val->getValues()[1])->getValue(), "2");
        EXPECT_EQ(As<ASTNumberValue>(Val->getValues()[2])->getValue(), "3");
        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueArrayBools) {
        llvm::StringRef src =
            "func() {\n"
            "  bool[] a = {true, false, true}\n"
            "}\n";
        ASTModule *M = Parse("ValueArrayBools", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *Val = As<ASTArrayValue>(rhsValue(Body, 0));
        ASSERT_TRUE(Val != nullptr);
        EXPECT_EQ(Val->size(), 3u);
        EXPECT_EQ(As<ASTBoolValue>(Val->getValues()[0])->getValue(), true);
        EXPECT_EQ(As<ASTBoolValue>(Val->getValues()[1])->getValue(), false);
        EXPECT_EQ(As<ASTBoolValue>(Val->getValues()[2])->getValue(), true);
        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueArrayStrings) {
        llvm::StringRef src =
            "func() {\n"
            "  string[] a = {\"hello\", \"world\", \"\"}\n"
            "}\n";
        ASTModule *M = Parse("ValueArrayStrings", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *Val = As<ASTArrayValue>(rhsValue(Body, 0));
        ASSERT_TRUE(Val != nullptr);
        EXPECT_EQ(Val->size(), 3u);
        EXPECT_EQ(As<ASTStringValue>(Val->getValues()[0])->getValue(), "hello");
        EXPECT_EQ(As<ASTStringValue>(Val->getValues()[1])->getValue(), "world");
        EXPECT_EQ(As<ASTStringValue>(Val->getValues()[2])->getValue(), "");
        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueArrayNested) {
        llvm::StringRef src =
            "func() {\n"
            "  int[][] a = {{1, 2}, {3, 4}}\n"
            "}\n";
        ASTModule *M = Parse("ValueArrayNested", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *Outer = As<ASTArrayValue>(rhsValue(Body, 0));
        ASSERT_TRUE(Outer != nullptr);
        EXPECT_EQ(Outer->size(), 2u);

        auto *Row0 = As<ASTArrayValue>(Outer->getValues()[0]);
        ASSERT_TRUE(Row0 != nullptr);
        EXPECT_EQ(Row0->size(), 2u);
        EXPECT_EQ(As<ASTNumberValue>(Row0->getValues()[0])->getValue(), "1");
        EXPECT_EQ(As<ASTNumberValue>(Row0->getValues()[1])->getValue(), "2");

        auto *Row1 = As<ASTArrayValue>(Outer->getValues()[1]);
        ASSERT_TRUE(Row1 != nullptr);
        EXPECT_EQ(Row1->size(), 2u);
        EXPECT_EQ(As<ASTNumberValue>(Row1->getValues()[0])->getValue(), "3");
        EXPECT_EQ(As<ASTNumberValue>(Row1->getValues()[1])->getValue(), "4");

        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueArrayMixed) {
        llvm::StringRef src =
            "func() {\n"
            "  int[] a = {0, 0xFF, 0b1010}\n"
            "}\n";
        ASTModule *M = Parse("ValueArrayMixed", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *Val = As<ASTArrayValue>(rhsValue(Body, 0));
        ASSERT_TRUE(Val != nullptr);
        EXPECT_EQ(Val->size(), 3u);
        EXPECT_EQ(As<ASTNumberValue>(Val->getValues()[0])->getValue(), "0");
        EXPECT_EQ(As<ASTNumberValue>(Val->getValues()[1])->getValue(), "0xFF");
        EXPECT_EQ(As<ASTNumberValue>(Val->getValues()[2])->getValue(), "0b1010");
        EXPECT_FALSE(HasErrorOccurred());
    }

    // ─── octal prefix (Python style 0o) ─────────────────────────────────────

    TEST_F(ParserTest, ValueIntegerOctal) {
        llvm::StringRef src =
            "func() {\n"
            "  int a = 0o0\n"
            "  int b = 0o7\n"
            "  int c = 0o755\n"
            "  int d = 0O644\n"
            "}\n";
        ASTModule *M = Parse("ValueIntegerOctal", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 0))->getValue(), "0o0");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 1))->getValue(), "0o7");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 2))->getValue(), "0o755");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 3))->getValue(), "0O644");
        EXPECT_FALSE(HasErrorOccurred());
    }

    // ─── digit separator _ ───────────────────────────────────────────────────

    TEST_F(ParserTest, ValueIntegerDigitSeparator) {
        llvm::StringRef src =
            "func() {\n"
            "  int a = 1_000\n"
            "  int b = 1_000_000\n"
            "  int c = 0xFF_FF\n"
            "  int d = 0b1010_1010\n"
            "  int e = 0o7_5_5\n"
            "}\n";
        ASTModule *M = Parse("ValueIntegerDigitSeparator", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 0))->getValue(), "1_000");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 1))->getValue(), "1_000_000");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 2))->getValue(), "0xFF_FF");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 3))->getValue(), "0b1010_1010");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 4))->getValue(), "0o7_5_5");
        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueFloatDigitSeparator) {
        llvm::StringRef src =
            "func() {\n"
            "  double a = 1_000.5\n"
            "  double b = 3.141_592\n"
            "}\n";
        ASTModule *M = Parse("ValueFloatDigitSeparator", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 0))->getValue(), "1_000.5");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 1))->getValue(), "3.141_592");
        EXPECT_FALSE(HasErrorOccurred());
    }

    // ─── imaginary suffix j / complex type ──────────────────────────────────

    TEST_F(ParserTest, ValueImaginary) {
        llvm::StringRef src =
            "func() {\n"
            "  complex a = 3.14j\n"
            "  complex b = 1.0J\n"
            "  complex c = 0.0j\n"
            "}\n";
        ASTModule *M = Parse("ValueImaginary", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 0))->getValue(), "3.14j");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 1))->getValue(), "1.0J");
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 2))->getValue(), "0.0j");
        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueComplexType) {
        llvm::StringRef src =
            "func() {\n"
            "  complex a = 2.5j\n"
            "}\n";
        ASTModule *M = Parse("ValueComplexType", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();
        auto *Stmt = static_cast<ASTDeclStmt *>(Body->getContent()[0]);
        auto *Var = Stmt->getLocalVar();

        EXPECT_TRUE(HasBuiltinType(Var->getType(), ASTBuiltinTypeKind::TYPE_COMPLEX));
        EXPECT_EQ(As<ASTNumberValue>(rhsValue(Body, 0))->getValue(), "2.5j");
        EXPECT_FALSE(HasErrorOccurred());
    }

    // ─── struct values ───────────────────────────────────────────────────────

    TEST_F(ParserTest, ValueStructSingle) {
        llvm::StringRef src =
            "func() {\n"
            "  Type s = {x = 42}\n"
            "}\n";
        ASTModule *M = Parse("ValueStructSingle", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *Val = As<ASTStructValue>(rhsValue(Body, 0));
        ASSERT_TRUE(Val != nullptr);
        EXPECT_EQ(Val->getValueKind(), ASTValueKind::VAL_STRUCT);
        EXPECT_EQ(Val->size(), 1u);
        auto It = Val->getValues().find("x");
        ASSERT_NE(It, Val->getValues().end());
        EXPECT_EQ(As<ASTNumberValue>(It->second)->getValue(), "42");
        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueStructMultiple) {
        llvm::StringRef src =
            "func() {\n"
            "  Type s = {x = 1, y = 2, z = 3}\n"
            "}\n";
        ASTModule *M = Parse("ValueStructMultiple", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *Val = As<ASTStructValue>(rhsValue(Body, 0));
        ASSERT_TRUE(Val != nullptr);
        EXPECT_EQ(Val->size(), 3u);
        EXPECT_EQ(As<ASTNumberValue>(Val->getValues().find("x")->second)->getValue(), "1");
        EXPECT_EQ(As<ASTNumberValue>(Val->getValues().find("y")->second)->getValue(), "2");
        EXPECT_EQ(As<ASTNumberValue>(Val->getValues().find("z")->second)->getValue(), "3");
        EXPECT_FALSE(HasErrorOccurred());
    }

    TEST_F(ParserTest, ValueStructMixedValues) {
        llvm::StringRef src =
            "func() {\n"
            "  Type s = {name = \"fly\", count = 0, flag = true}\n"
            "}\n";
        ASTModule *M = Parse("ValueStructMixedValues", src);
        auto *Body = As<ASTFunction>(M->getNodes()[0])->getBody();

        auto *Val = As<ASTStructValue>(rhsValue(Body, 0));
        ASSERT_TRUE(Val != nullptr);
        EXPECT_EQ(Val->size(), 3u);
        EXPECT_EQ(As<ASTStringValue>(Val->getValues().find("name")->second)->getValue(), "fly");
        EXPECT_EQ(As<ASTNumberValue>(Val->getValues().find("count")->second)->getValue(), "0");
        EXPECT_EQ(As<ASTBoolValue>(Val->getValues().find("flag")->second)->getValue(), true);
        EXPECT_FALSE(HasErrorOccurred());
    }

} // namespace
