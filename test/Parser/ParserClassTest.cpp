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
#include "AST/ASTClass.h"
#include "AST/ASTFunction.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTMethod.h"
#include "AST/ASTModule.h"
#include "AST/ASTValue.h"
#include "ParserTest.h"

#include <AST/ASTDeclStmt.h>
#include <AST/ASTFailStmt.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTMember.h>
#include <AST/ASTName.h>
#include <AST/ASTReturnStmt.h>

namespace {

    using namespace fly;

    TEST_F(ParserTest, NullTypeVarReturn) {
        llvm::StringRef str = ("public class Type {}\n"
                               "func() {\n"
                               "  Type t = null\n"
                               "  return\n"
                               "}\n");
        ASTModule *Module = Parse("TypeDefaultVarReturn", str);

        // Verify Type class is defined first
    	ASTClass *TypeClass = As<ASTClass>(Module->getNodes()[0]);
    	ASSERT_TRUE(TypeClass != nullptr);
    	EXPECT_EQ(TypeClass->getName(), "Type");
    	EXPECT_TRUE(HasModifier(TypeClass->getModifiers(), ASTModifierKind::MOD_PUBLIC));

        // Get Function Body (now at index 1 since Type class is at index 0)
        ASTFunction *F = As<ASTFunction>(Module->getNodes()[1]);
        ASTBlockStmt *Body = F->getBody();
        ASSERT_FALSE(Body->getContent().empty());

        // Test: Type t = null
        auto *varStmt = As<ASTDeclStmt>(Body->getContent()[0]);
        EXPECT_EQ(varStmt->getLocalVar()->getName(), "t");
        auto *assignExpr = As<ASTBinary>(varStmt->getExpr());
        EXPECT_EQ(assignExpr->getBinaryKind(), ASTBinaryKind::OP_BINARY_ASSIGN);

    	auto *Left = As<ASTIdentifier>(assignExpr->getLeftExpr());
        EXPECT_EQ(Left->getName(), "t");
    	// Type resolution happens during Sema phase, not parsing
    	EXPECT_TRUE(As<ASTValue>(assignExpr->getRightExpr())->isNull());

        ASTReturnStmt *Ret = As<ASTReturnStmt>(Body->getContent()[1]);
        EXPECT_EQ(Ret->getStmtKind(), ASTStmtKind::STMT_RETURN);
    }

    TEST_F(ParserTest, Struct) {
        llvm::StringRef str = (
            "public struct Test {\n"
            "  int a\n"
            "  public int b = 2\n"
            "  const int c = 0\n"
            "}\n"
            "func1() {\n"
            "  Test t = new Test()\n"
            "  Test x = { a = 3, b = 1 }\n"
            "}\n");
    	ASTModule *Module = Parse("StructTest", str);

    	ASTClass *TestStruct = As<ASTClass>(Module->getNodes()[0]);
    	ASSERT_TRUE(TestStruct != nullptr);
    	EXPECT_EQ(TestStruct->getName(), "Test");
    	EXPECT_TRUE(HasModifier(TestStruct->getModifiers(), ASTModifierKind::MOD_PUBLIC));
    	EXPECT_EQ(TestStruct->getNodes().size(), 3);
    	ASTAttribute *aVar = As<ASTAttribute>(TestStruct->getNodes()[0]);
    	ASTAttribute *bVar = As<ASTAttribute>(TestStruct->getNodes()[1]);
    	ASTAttribute *cVar = As<ASTAttribute>(TestStruct->getNodes()[2]);
    	EXPECT_EQ(aVar->getName(), "a");
    	EXPECT_EQ(bVar->getName(), "b");
    	EXPECT_EQ(cVar->getName(), "c");
    	EXPECT_TRUE(aVar->getModifiers().empty());
    	EXPECT_TRUE(HasModifier(bVar->getModifiers(), ASTModifierKind::MOD_PUBLIC));
    	EXPECT_TRUE(HasModifier(cVar->getModifiers(), ASTModifierKind::MOD_CONSTANT));
    	// Field 'a' has no initializer, so getExpr() might be null or return a default value
    	EXPECT_TRUE(aVar->getExpr() == nullptr);
    	EXPECT_EQ(As<ASTNumberValue>(bVar->getExpr())->getValue(), "2");
		EXPECT_EQ(As<ASTNumberValue>(cVar->getExpr())->getValue(), "0");

    	ASTFunction *Function = As<ASTFunction>(Module->getNodes()[1]);
    	ASSERT_TRUE(Function != nullptr);
		EXPECT_EQ(Function->getName(), "func1");

    	// Test t = new Test()
    	auto *assignExpr1 = As<ASTDeclStmt>(Function->getBody()->getContent()[0]);
    	EXPECT_EQ(assignExpr1->getLocalVar()->getName(), "t");
    	auto *assign1 = As<ASTBinary>(assignExpr1->getExpr());
    	EXPECT_EQ(assign1->getBinaryKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
		auto *assign1_src = As<ASTIdentifier>(assign1->getLeftExpr());
		ASSERT_TRUE(assign1_src != nullptr);
		EXPECT_EQ(assign1_src->getName(), "t");
		auto *newCall = As<ASTCall>(assign1->getRightExpr());
		ASSERT_TRUE(newCall != nullptr);
		EXPECT_EQ(newCall->getExprKind(), ASTExprKind::EXPR_CALL);
		EXPECT_EQ(newCall->getName(), "Test");

    	// Test x = { a = 3, b = 1 }
    	auto *assignExpr2 = As<ASTDeclStmt>(Function->getBody()->getContent()[1]);
    	EXPECT_EQ(assignExpr2->getLocalVar()->getName(), "x");
    	auto *assign2 = As<ASTBinary>(assignExpr2->getExpr());
    	EXPECT_EQ(assign2->getBinaryKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
    	auto *assign2_src = As<ASTIdentifier>(assign2->getLeftExpr());
    	ASSERT_TRUE(assign2_src != nullptr);
    	EXPECT_EQ(assign2_src->getName(), "x");
    	ASTStructValue *assign2_target = As<ASTStructValue>(assign2->getRightExpr());
         ASSERT_TRUE(assign2_target != nullptr);
         ASSERT_TRUE(assign2_target->isStruct());

         // Check the struct value contains the expected fields: { a = 3, b = 1 }
         const auto &structValues = assign2_target->getValues();
         EXPECT_EQ(structValues.size(), 2);

         // Check field 'a' = 3
         auto aIt = structValues.find("a");
         ASSERT_TRUE(aIt != structValues.end());
         auto *aValue = As<ASTNumberValue>(aIt->second);
         ASSERT_TRUE(aValue != nullptr);
         EXPECT_EQ(aValue->getValue(), "3");

         // Check field 'b' = 1
         auto bIt = structValues.find("b");
         ASSERT_TRUE(bIt != structValues.end());
         auto *bValue = As<ASTNumberValue>(bIt->second);
         ASSERT_TRUE(bValue != nullptr);
         EXPECT_EQ(bValue->getValue(), "1");
    }

    TEST_F(ParserTest, Class) {
        llvm::StringRef str = (
        	"public class Test {\n"
			"  int a = 1\n"
			"  private int b = 1\n"
			"  public a() { fail a }\n"
			"  protected b() { fail 2 }\n"
			"  private c() { fail 3 }\n"
			"  const d() { fail 0 }\n"
			"}\n"
			"func() {\n"
			"  Test t = new Test()\n"
			"  t.a()\n"
			"}\n");
        ASTModule *Module = Parse("TestClass", str);


        ASTClass *TestClass = As<ASTClass>(Module->getNodes()[0]);
        ASSERT_TRUE(TestClass != nullptr);
        EXPECT_EQ(TestClass->getName(), "Test");
        EXPECT_TRUE(HasModifier(TestClass->getModifiers(), ASTModifierKind::MOD_PUBLIC));

        // Attributes: a, b (direct index access into class nodes)
        // Order in source: attribute a, attribute b, method a, method b, method c, method d
        ASSERT_TRUE(TestClass->getNodes().size() >= 6);
        ASTAttribute *aVar = As<ASTAttribute>(TestClass->getNodes()[0]);
        ASTAttribute *bVar = As<ASTAttribute>(TestClass->getNodes()[1]);
        ASSERT_TRUE(aVar != nullptr);
        ASSERT_TRUE(bVar != nullptr);
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_TRUE(aVar->getModifiers().empty());
        EXPECT_TRUE(HasModifier(bVar->getModifiers(), ASTModifierKind::MOD_PRIVATE));
        // attribute initializers
        EXPECT_TRUE(As<ASTNumberValue>(aVar->getExpr())->getValue() == "1");
        EXPECT_TRUE(As<ASTNumberValue>(bVar->getExpr())->getValue() == "1");

        // Methods: a, b, c, d (direct index access)
        ASTMethod *aMethod = As<ASTMethod>(TestClass->getNodes()[2]);
        ASTMethod *bMethod = As<ASTMethod>(TestClass->getNodes()[3]);
        ASTMethod *cMethod = As<ASTMethod>(TestClass->getNodes()[4]);
        ASTMethod *dMethod = As<ASTMethod>(TestClass->getNodes()[5]);
        ASSERT_TRUE(aMethod != nullptr);
        ASSERT_TRUE(bMethod != nullptr);
        ASSERT_TRUE(cMethod != nullptr);
        ASSERT_TRUE(dMethod != nullptr);

        EXPECT_TRUE(HasModifier(aMethod->getModifiers(), ASTModifierKind::MOD_PUBLIC));
        EXPECT_TRUE(HasModifier(bMethod->getModifiers(), ASTModifierKind::MOD_PROTECTED));
        EXPECT_TRUE(HasModifier(cMethod->getModifiers(), ASTModifierKind::MOD_PRIVATE));
        EXPECT_TRUE(HasModifier(dMethod->getModifiers(), ASTModifierKind::MOD_CONSTANT));

        // Verify method return types are void and bodies contain fail statements

        // a() { fail a }
        {
            ASTBlockStmt *MBody = aMethod->getBody();
            ASSERT_FALSE(MBody->getContent().empty());
            ASTFailStmt *Fail = As<ASTFailStmt>(MBody->getContent()[0]);
            ASSERT_TRUE(Fail != nullptr);
            EXPECT_EQ(Fail->getStmtKind(), ASTStmtKind::STMT_FAIL);
            EXPECT_EQ(Fail->getFirstExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
            EXPECT_EQ(As<ASTIdentifier>(Fail->getFirstExpr())->getName(), "a");
        }

        // b() { fail 2 }
        {
            ASTBlockStmt *MBody = bMethod->getBody();
            ASSERT_FALSE(MBody->getContent().empty());
            ASTFailStmt *Fail = As<ASTFailStmt>(MBody->getContent()[0]);
            ASSERT_TRUE(Fail != nullptr);
            EXPECT_EQ(Fail->getStmtKind(), ASTStmtKind::STMT_FAIL);
            EXPECT_EQ(Fail->getFirstExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
            EXPECT_EQ(As<ASTNumberValue>(Fail->getFirstExpr())->getValue(), "2");
        }

        // c() { fail 3 }
        {
            ASTBlockStmt *MBody = cMethod->getBody();
            ASSERT_FALSE(MBody->getContent().empty());
            ASTFailStmt *Fail = As<ASTFailStmt>(MBody->getContent()[0]);
            ASSERT_TRUE(Fail != nullptr);
            EXPECT_EQ(Fail->getStmtKind(), ASTStmtKind::STMT_FAIL);
            EXPECT_EQ(Fail->getFirstExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
            EXPECT_EQ(As<ASTNumberValue>(Fail->getFirstExpr())->getValue(), "3");
        }

        // d() { fail 0 }
        {
            ASTBlockStmt *MBody = dMethod->getBody();
            ASSERT_FALSE(MBody->getContent().empty());
            ASTFailStmt *Fail = As<ASTFailStmt>(MBody->getContent()[0]);
            ASSERT_TRUE(Fail != nullptr);
            EXPECT_EQ(Fail->getStmtKind(), ASTStmtKind::STMT_FAIL);
            EXPECT_EQ(Fail->getFirstExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
            EXPECT_EQ(As<ASTNumberValue>(Fail->getFirstExpr())->getValue(), "0");
        }

        // Verify we can call a method on an instance in function
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[1]);
        ASSERT_TRUE(Func != nullptr);
        ASTBlockStmt *Body = Func->getBody();
        ASSERT_FALSE(Body->getContent().empty());

        // Test t = new Test()
        auto *assignExpr = As<ASTDeclStmt>(Body->getContent()[0]);
        EXPECT_EQ(assignExpr->getLocalVar()->getName(), "t");
        auto *assign = As<ASTBinary>(assignExpr->getExpr());
        EXPECT_EQ(assign->getBinaryKind(), ASTBinaryKind::OP_BINARY_ASSIGN);
         auto *assign_src = As<ASTIdentifier>(assign->getLeftExpr());
         ASSERT_TRUE(assign_src != nullptr);
         EXPECT_EQ(assign_src->getName(), "t");
         auto *newCall = As<ASTCall>(assign->getRightExpr());
         ASSERT_TRUE(newCall != nullptr);
         EXPECT_EQ(newCall->getExprKind(), ASTExprKind::EXPR_CALL);
         EXPECT_EQ(newCall->getName(), "Test");
    }

    TEST_F(ParserTest, ClassExtendClass) {
         llvm::StringRef str = (
	         "public class Case : Test {\n"
	         "  int b\n"
	         "  f() {}\n"
	         "}\n");
         ASTModule *Module = Parse("TestClass", str);

        // Verify Case class
        ASTClass *Case = As<ASTClass>(Module->getNodes()[0]);
        ASSERT_TRUE(Case != nullptr);
        EXPECT_EQ(Case->getName(), "Case");
        EXPECT_TRUE(HasModifier(Case->getModifiers(), ASTModifierKind::MOD_PUBLIC));

        // Bases: should contain named type 'Test'
        const auto &Bases = Case->getBases();
        EXPECT_EQ(Bases.size(), 1);
        ASTType *Base0 = Bases[0];
        ASSERT_TRUE(Base0 != nullptr);
        EXPECT_EQ(Base0->getTypeKind(), ASTTypeKind::TYPE_NAMED);
        ASTNamedType *Named = As<ASTNamedType>(Base0);
        ASSERT_TRUE(Named != nullptr);
        const auto &Names = Named->getNames();
        ASSERT_TRUE(!Names.empty());
        EXPECT_EQ(Names[0]->getName(), "Test");

        // Nodes: attribute 'b' then method 'f'
        ASSERT_TRUE(Case->getNodes().size() >= 2);
        ASTAttribute *bAttr = As<ASTAttribute>(Case->getNodes()[0]);
        ASTMethod *fMethod = As<ASTMethod>(Case->getNodes()[1]);
        ASSERT_TRUE(bAttr != nullptr);
        ASSERT_TRUE(fMethod != nullptr);
        EXPECT_EQ(bAttr->getName(), "b");
        EXPECT_TRUE(bAttr->getModifiers().empty());

        EXPECT_EQ(fMethod->getName(), "f");
        // method body should be present and empty
        ASTBlockStmt *FBody = fMethod->getBody();
        ASSERT_TRUE(FBody != nullptr);
        EXPECT_EQ(FBody->getContent().size(), 0);
     }

    TEST_F(ParserTest, ClassExtendAll) {
        llvm::StringRef str = ("public class Test : Class, Struct, Interface {\n"
                               "}\n");
        ASTModule *Module = Parse("Test", str);

        // Verify the Test class has Class, Struct and Interface as bases
        ASTClass *Test = As<ASTClass>(Module->getNodes()[0]);
        ASSERT_TRUE(Test != nullptr);
        EXPECT_EQ(Test->getName(), "Test");
        const auto &Bases = Test->getBases();
        EXPECT_EQ(Bases.size(), 3);
        // Expected order: Class, Struct, Interface
        const char *ExpectedNames[3] = {"Class", "Struct", "Interface"};
        for (size_t i = 0; i < 3; ++i) {
            ASTType *B = Bases[i];
            ASSERT_TRUE(B != nullptr);
            EXPECT_EQ(B->getTypeKind(), ASTTypeKind::TYPE_NAMED);
            ASTNamedType *Named = As<ASTNamedType>(B);
            ASSERT_TRUE(Named != nullptr);
            const auto &Names = Named->getNames();
            ASSERT_TRUE(!Names.empty());
            EXPECT_EQ(Names[0]->getName(), ExpectedNames[i]);
        }
    }
}
