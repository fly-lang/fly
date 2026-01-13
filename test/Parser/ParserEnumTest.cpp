//===--------------------------------------------------------------------------------------------------------------===//
// test/IdentifierParserTest.cpp - Identifier Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTAttribute.h"
#include "AST/ASTBinary.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTCall.h"
#include "AST/ASTEnum.h"
#include "AST/ASTEnumValue.h"
#include "AST/ASTFunction.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTModule.h"
#include "ParserTest.h"

#include <AST/ASTDeclStmt.h>
#include <AST/ASTExprStmt.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTMember.h>
#include <AST/ASTName.h>

namespace {

    using namespace fly;

    TEST_F(ParserTest, Enum) {
        llvm::StringRef str = ("public enum Test {\n"
                               "  A, B, C\n"
                               "}\n");
        ASTModule *Module = Parse("TestEnum", str);

        llvm::StringRef str2 = (
                "void main() {\n"
                "  Test a = Test.A\n"
                "  a = Test.B\n"
                "  Test c = a\n"
                "}\n");
        ASTModule *Module2 = Parse("func", str2);

    	// Verify enum node exists and has expected name
    	ASTEnum *E = As<ASTEnum>(Module->getNodes()[0]);
    	ASSERT_TRUE(E != nullptr);
    	EXPECT_EQ(E->getName(), "Test");
    	EXPECT_TRUE(HasModifier(E->getModifiers(), ASTModifierKind::MOD_PUBLIC));
    	EXPECT_TRUE(!E->getNodes().empty());
    	EXPECT_EQ(E->getNodes().size(), 3);
    	ASTEnumValue *A = As<ASTEnumValue>(E->getNodes()[0]);
		ASTEnumValue *B = As<ASTEnumValue>(E->getNodes()[1]);
		ASTEnumValue *C = As<ASTEnumValue>(E->getNodes()[2]);
		ASSERT_TRUE(A != nullptr);
    	ASSERT_TRUE(B != nullptr);
    	ASSERT_TRUE(C != nullptr);

    	// Verify usage in main
    	ASTFunction *main = As<ASTFunction>(Module2->getNodes()[0]);
     	ASTBlockStmt *Body = main->getBody();
		ASSERT_FALSE(Body->getContent().empty());

		// First statement: Test a = Test.A
		auto *aExprStmt = As<ASTDeclStmt>(Body->getContent()[0]);
		EXPECT_EQ(aExprStmt->getLocalVar()->getName(), "a");
		auto *AssignBinaryExpr = As<ASTBinary>(aExprStmt->getExpr());
		EXPECT_EQ(AssignBinaryExpr->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);

		// Left side is 'a'
		auto *src = As<ASTIdentifier>(AssignBinaryExpr->getLeftExpr());
		ASSERT_TRUE(src != nullptr);
		EXPECT_EQ(src->getName(), "a");
		EXPECT_EQ(AssignBinaryExpr->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

		// Right side is Test.A (member expression)
		auto *Test_a = As<ASTMember>(AssignBinaryExpr->getRightExpr());
		ASSERT_TRUE(Test_a != nullptr);
		EXPECT_EQ(Test_a->getName(), "A");
		EXPECT_EQ(Test_a->getExprKind(), ASTExprKind::EXPR_MEMBER);
		EXPECT_EQ(As<ASTIdentifier>(Test_a->getParent())->getName(), "Test");
		EXPECT_EQ(Test_a->getParent()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

     // Verify second statement: a = Test.B
    	auto *aExprStmt2 = As<ASTExprStmt>(Body->getContent()[1]);
    	auto *AssignBinaryExpr2 = As<ASTBinary>(aExprStmt2->getExpr());
    	EXPECT_EQ(AssignBinaryExpr2->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);

    	// Left side is 'a'
    	auto *src2 = As<ASTIdentifier>(AssignBinaryExpr2->getLeftExpr());
    	ASSERT_TRUE(src2 != nullptr);
    	EXPECT_EQ(src2->getName(), "a");
    	EXPECT_EQ(AssignBinaryExpr2->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

    	// Right side is Test.B (member expression)
    	auto *Test_b = As<ASTMember>(AssignBinaryExpr2->getRightExpr());
    	ASSERT_TRUE(Test_b != nullptr);
    	EXPECT_EQ(Test_b->getName(), "B");
    	EXPECT_EQ(Test_b->getExprKind(), ASTExprKind::EXPR_MEMBER);
    	EXPECT_EQ(As<ASTIdentifier>(Test_b->getParent())->getName(), "Test");
    	EXPECT_EQ(Test_b->getParent()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

    	// Verify third statement: Test c = a
    	auto *aExprStmt3 = As<ASTDeclStmt>(Body->getContent()[2]);
    	EXPECT_EQ(aExprStmt3->getLocalVar()->getName(), "c");
    	auto *AssignBinaryExpr3 = As<ASTBinary>(aExprStmt3->getExpr());
    	EXPECT_EQ(AssignBinaryExpr3->getOpKind(), ASTBinaryKind::OP_BINARY_ASSIGN);

    	// Left side is 'c'
    	auto *src3 = As<ASTIdentifier>(AssignBinaryExpr3->getLeftExpr());
    	ASSERT_TRUE(src3 != nullptr);
    	EXPECT_EQ(src3->getName(), "c");
    	EXPECT_EQ(AssignBinaryExpr3->getLeftExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

    	// Right side is 'a' (identifier)
    	auto *target3 = As<ASTIdentifier>(AssignBinaryExpr3->getRightExpr());
    	ASSERT_TRUE(target3 != nullptr);
    	EXPECT_EQ(target3->getName(), "a");
    }

    TEST_F(ParserTest, EnumExtendEnum) {
        llvm::StringRef str = ("public enum Option : Enum {\n"
                               "  A, B, C\n"
                               "}\n");
        ASTModule *Module = Parse("Option", str);

        // Verify Option enum exists and extends Enum
        ASTEnum *Opt = As<ASTEnum>(Module->getNodes()[0]);
        ASSERT_TRUE(Opt != nullptr);
        EXPECT_EQ(Opt->getName(), "Option");
        EXPECT_TRUE(HasModifier(Opt->getModifiers(), ASTModifierKind::MOD_PUBLIC));
        // entries A, B, C
        EXPECT_EQ(Opt->getNodes().size(), 3);
    	ASTEnumValue *A = As<ASTEnumValue>(Opt->getNodes()[0]);
        ASTEnumValue *B = As<ASTEnumValue>(Opt->getNodes()[1]);
        ASTEnumValue *C = As<ASTEnumValue>(Opt->getNodes()[2]);
    	ASSERT_TRUE(A != nullptr);
        ASSERT_TRUE(B != nullptr);
        ASSERT_TRUE(C != nullptr);
        EXPECT_EQ(A->getName(), "A");
        EXPECT_EQ(B->getName(), "B");
    	EXPECT_EQ(C->getName(), "C");

        // Check super classes (should include named type 'Enum')
        const auto &Bases = Opt->getBases();
        EXPECT_EQ(Bases.size(), 1);
        ASTType *Base0 = Bases[0];
        ASSERT_TRUE(Base0 != nullptr);
        EXPECT_EQ(Base0->getTypeKind(), ASTTypeKind::TYPE_NAMED);
        ASTNamedType *Named = As<ASTNamedType>(Base0);
        ASSERT_TRUE(Named != nullptr);
        const auto &Names = Named->getNames();
        ASSERT_TRUE(!Names.empty());
        EXPECT_EQ(Names[0]->getName(), "Enum");
    }
}
