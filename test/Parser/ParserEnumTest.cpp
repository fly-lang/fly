//===--------------------------------------------------------------------------------------------------------------===//
// test/IdentifierParserTest.cpp - Identifier Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "ParserTest.h"
#include "AST/ASTModule.h"
#include "AST/ASTFunction.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTAttribute.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTMethod.h"

#include <AST/ASTMember.h>
#include <AST/ASTName.h>
#include <AST/ASTReturnStmt.h>

namespace {

    using namespace fly;

    TEST_F(ParserTest, Enum) {
        llvm::StringRef str = ("public enum Test {\n"
                               "  A B C\n"
                               "}\n");
        ASTModule *Module = Parse("TestEnum", str);

        llvm::StringRef str2 = (
                "void main() {\n"
                "  Test a = Test.A\n"
                "  a = Test.B"
                "  Test c = a"
                "}\n");
        ASTModule *Module2 = Parse("func", str2);

    	// Verify enum node exists and has expected name
    	ASTEnum *E = As<ASTEnum>(Module->getNodes()[0]);
    	ASSERT_TRUE(E != nullptr);
    	EXPECT_EQ(E->getName(), "Test");
    	EXPECT_TRUE(HasModifier(E->getModifiers(), ASTModifierKind::MOD_PUBLIC));
    	EXPECT_TRUE(!E->getNodes().empty());
    	EXPECT_EQ(E->getNodes().size(), 3);
    	ASTEnumEntry *A = As<ASTEnumEntry>(E->getNodes()[0]);
		ASTEnumEntry *B = As<ASTEnumEntry>(E->getNodes()[1]);
		ASTEnumEntry *C = As<ASTEnumEntry>(E->getNodes()[2]);
		ASSERT_TRUE(A != nullptr);
    	ASSERT_TRUE(B != nullptr);
    	ASSERT_TRUE(C != nullptr);

    	// Verify usage in main
    	ASTFunction *main = As<ASTFunction>(Module2->getNodes()[0]);
     	ASTBlockStmt *Body = main->getBody();
		ASSERT_FALSE(Body->getContent().empty());
		ASTAssignStmt *aAssignStmt = As<ASTAssignStmt>(Body->getContent()[0]);
		ASTIdentifier *src = As<ASTIdentifier>(aAssignStmt->getSource());
		ASSERT_TRUE(src != nullptr);
		EXPECT_EQ(src->getName(), "a");
		// The target is a member expression: Test.A -> represented as ASTMember where the member's
		// name is 'A' and its parent is an identifier 'Test'. Always assert the last part (the
		// member) first, then verify its parent identifier.
		ASTMember * Test_a = As<ASTMember>(aAssignStmt->getTarget());
		ASSERT_TRUE(Test_a != nullptr);
		EXPECT_EQ(Test_a->getName(), "A");
		EXPECT_EQ(Test_a->getExprKind(), ASTExprKind::EXPR_MEMBER);
		EXPECT_EQ(As<ASTIdentifier>(Test_a->getParent())->getName(), "Test");
		EXPECT_EQ(Test_a->getParent()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

     // Verify second assignment: a = Test.B
    	ASTAssignStmt *aAssignStmt2 = As<ASTAssignStmt>(Body->getContent()[1]);
    	ASTIdentifier *src2 = As<ASTIdentifier>(aAssignStmt2->getSource());
    	ASSERT_TRUE(src2 != nullptr);
    	EXPECT_EQ(src2->getName(), "a");
    	ASTMember *Test_b = As<ASTMember>(aAssignStmt2->getTarget());
    	ASSERT_TRUE(Test_b != nullptr);
    	EXPECT_EQ(Test_b->getName(), "B");
    	EXPECT_EQ(Test_b->getExprKind(), ASTExprKind::EXPR_MEMBER);
    	EXPECT_EQ(As<ASTIdentifier>(Test_b->getParent())->getName(), "Test");
    	EXPECT_EQ(Test_b->getParent()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);

    	// Verify third statement: Test c = a
    	ASTAssignStmt *aAssignStmt3 = As<ASTAssignStmt>(Body->getContent()[2]);
    	ASTIdentifier *src3 = As<ASTIdentifier>(aAssignStmt3->getSource());
    	ASSERT_TRUE(src3 != nullptr);
    	EXPECT_EQ(src3->getName(), "c");
    	ASTIdentifier *target3 = As<ASTIdentifier>(aAssignStmt3->getTarget());
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
    	ASTEnumEntry *A = As<ASTEnumEntry>(Opt->getNodes()[0]);
        ASTEnumEntry *B = As<ASTEnumEntry>(Opt->getNodes()[1]);
        ASTEnumEntry *C = As<ASTEnumEntry>(Opt->getNodes()[2]);
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
