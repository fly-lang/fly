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
#include "AST/ASTImport.h"
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

    TEST_F(ParserTest, NullTypeVarReturn) {
        llvm::StringRef str = ("Type func() {\n"
                               "  Type t = null"
                               "  return t\n"
                               "}"
                               "public class Type {}\n"
							   "\n");
        ASTModule *Module = Parse("TypeDefaultVarReturn", str);



        // Get Body
        ASTFunction *F = As<ASTFunction>(Module->getNodes()[0]);
        EXPECT_EQ(F->getReturnType()->getTypeKind(), ASTTypeKind::TYPE_NAMED);
        ASTBlockStmt *Body = F->getBody();
        ASSERT_FALSE(Body->getContent().empty());

        // Test: Type t = null
        ASTAssignStmt *varStmt = As<ASTAssignStmt>(Body->getContent()[0]);

    	ASTIdentifier * Left = As<ASTIdentifier>(varStmt->getSource());
        EXPECT_EQ(Left->getName(), "t");
    	EXPECT_TRUE(Left->getType()->isClass());
    	EXPECT_TRUE(As<ASTValue>(varStmt->getTarget())->isNull());

        ASTReturnStmt *Ret = As<ASTReturnStmt>(Body->getContent()[1]);
        ASTIdentifier *RetRef = As<ASTIdentifier>(Ret->getExpr());
        EXPECT_EQ(RetRef->getName(), "t");

    	ASTClass *TypeClass = As<ASTClass>(Module->getNodes()[1]);
    	ASSERT_TRUE(TypeClass != nullptr);
    	EXPECT_EQ(TypeClass->getName(), "Type");
    	EXPECT_TRUE(HasModifier(TypeClass->getModifiers(), ASTModifierKind::MOD_PUBLIC));
    }

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
    	// The RHS is a member expression Test.B. Check the member (last part) first.
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

    TEST_F(ParserTest, Struct) {
        llvm::StringRef str = (
            "public struct Test {\n"
            "  int a\n"
            "  public int b = 2\n"
            "  const int c = 0\n"
            "}\n"
            "void func1() {\n"
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
    	EXPECT_TRUE(As<ASTValue>(aVar->getExpr())->isDefault());
    	EXPECT_EQ(As<ASTNumberValue>(bVar->getExpr())->getValue(), "2");
		EXPECT_EQ(As<ASTNumberValue>(cVar->getExpr())->getValue(), "0");

    	ASTFunction *Function = As<ASTFunction>(Module->getNodes()[1]);
    	ASSERT_TRUE(Function != nullptr);
		EXPECT_EQ(Function->getName(), "func1");

    	ASTAssignStmt *assign1 = As<ASTAssignStmt>(Function->getBody()->getContent()[0]);
		ASTIdentifier *assign1_src = As<ASTIdentifier>(assign1->getSource());
		ASSERT_TRUE(assign1_src != nullptr);
		EXPECT_EQ(assign1_src->getName(), "t");
		ASTCall *newCall = As<ASTCall>(assign1->getTarget());
		ASSERT_TRUE(newCall != nullptr);
		EXPECT_EQ(newCall->getExprKind(), ASTExprKind::EXPR_CALL);
		EXPECT_EQ(newCall->getName(), "Test");

    	ASTAssignStmt *assign2 = As<ASTAssignStmt>(Function->getBody()->getContent()[1]);
    	ASTIdentifier *assign2_src = As<ASTIdentifier>(assign2->getSource());
    	ASSERT_TRUE(assign2_src != nullptr);
    	EXPECT_EQ(assign2_src->getName(), "x");
    	ASTValue *assign2_target = As<ASTValue>(assign2->getTarget());
         ASSERT_TRUE(assign2_target != nullptr);
         ASSERT_TRUE(assign2_target->isStruct());


    }

    TEST_F(ParserTest, Class) {
        llvm::StringRef str = (
        	"public class Test {\n"
			"  int a = 1\n"
			"  private int b = 1\n"
			"  public int a() { return a }\n"
			"  protected int b() { return 2 }\n"
			"  private int c() { return 3 }\n"
			"  const int d() { return 0 }\n"
			"}\n"
			"void func() {\n"
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
        EXPECT_TRUE(dMethod->getModifiers().empty());
        EXPECT_TRUE(HasModifier(dMethod->getModifiers(), ASTModifierKind::MOD_CONSTANT));

        // Verify method return types and bodies
        EXPECT_TRUE(HasBuiltinType(aMethod->getReturnType(), ASTBuiltinTypeKind::TYPE_INT));
        EXPECT_TRUE(HasBuiltinType(bMethod->getReturnType(), ASTBuiltinTypeKind::TYPE_INT));
        EXPECT_TRUE(HasBuiltinType(cMethod->getReturnType(), ASTBuiltinTypeKind::TYPE_INT));
        EXPECT_TRUE(HasBuiltinType(dMethod->getReturnType(), ASTBuiltinTypeKind::TYPE_INT));

        // a() { return a }
        {
            ASTBlockStmt *MBody = aMethod->getBody();
            ASSERT_FALSE(MBody->getContent().empty());
            ASTReturnStmt *Ret = As<ASTReturnStmt>(MBody->getContent()[0]);
            ASSERT_TRUE(Ret != nullptr);
            EXPECT_EQ(Ret->getStmtKind(), ASTStmtKind::STMT_RETURN);
            EXPECT_EQ(Ret->getExpr()->getExprKind(), ASTExprKind::EXPR_IDENTIFIER);
            EXPECT_EQ(As<ASTIdentifier>(Ret->getExpr())->getName(), "a");
        }

        // b() { return 2 }
        {
            ASTBlockStmt *MBody = bMethod->getBody();
            ASSERT_FALSE(MBody->getContent().empty());
            ASTReturnStmt *Ret = As<ASTReturnStmt>(MBody->getContent()[0]);
            ASSERT_TRUE(Ret != nullptr);
            EXPECT_EQ(Ret->getStmtKind(), ASTStmtKind::STMT_RETURN);
            EXPECT_EQ(Ret->getExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
            EXPECT_EQ(As<ASTNumberValue>(Ret->getExpr())->getValue(), "2");
        }

        // c() { return 3 }
        {
            ASTBlockStmt *MBody = cMethod->getBody();
            ASSERT_FALSE(MBody->getContent().empty());
            ASTReturnStmt *Ret = As<ASTReturnStmt>(MBody->getContent()[0]);
            ASSERT_TRUE(Ret != nullptr);
            EXPECT_EQ(Ret->getStmtKind(), ASTStmtKind::STMT_RETURN);
            EXPECT_EQ(Ret->getExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
            EXPECT_EQ(As<ASTNumberValue>(Ret->getExpr())->getValue(), "3");
        }

        // d() { return 0 }
        {
            ASTBlockStmt *MBody = dMethod->getBody();
            ASSERT_FALSE(MBody->getContent().empty());
            ASTReturnStmt *Ret = As<ASTReturnStmt>(MBody->getContent()[0]);
            ASSERT_TRUE(Ret != nullptr);
            EXPECT_EQ(Ret->getStmtKind(), ASTStmtKind::STMT_RETURN);
            EXPECT_EQ(Ret->getExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
            EXPECT_EQ(As<ASTNumberValue>(Ret->getExpr())->getValue(), "0");
        }

        // Verify we can call a method on an instance in function
        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[1]);
        ASSERT_TRUE(Func != nullptr);
        ASTBlockStmt *Body = Func->getBody();
        ASSERT_FALSE(Body->getContent().empty());
        ASTAssignStmt *assign = As<ASTAssignStmt>(Body->getContent()[0]);
         ASTIdentifier *assign_src = As<ASTIdentifier>(assign->getSource());
         ASSERT_TRUE(assign_src != nullptr);
         EXPECT_EQ(assign_src->getName(), "t");
         ASTCall *newCall = As<ASTCall>(assign->getTarget());
         ASSERT_TRUE(newCall != nullptr);
         EXPECT_EQ(newCall->getExprKind(), ASTExprKind::EXPR_CALL);
         EXPECT_EQ(newCall->getName(), "Test");
    }

    TEST_F(ParserTest, StructExtendStruct) {
        llvm::StringRef str = ("public struct Case : Test {\n"
                               "}\n");
        ASTModule *Module = Parse("CaseStruct", str);

        llvm::StringRef str2 = (
                "struct Test {\n"
                "}\n");
        ASTModule *Module2 = Parse("TestStruct", str2);



        // Verify the Case struct has Test as a base
        ASTClass *Case = As<ASTClass>(Module->getNodes()[0]);
        ASSERT_TRUE(Case != nullptr);
        EXPECT_EQ(Case->getName(), "Case");
        const auto &Bases = Case->getBases();
        EXPECT_EQ(Bases.size(), 1);
        ASTType *Base0 = Bases[0];
        ASSERT_TRUE(Base0 != nullptr);
        // Base should be a named type with name 'Test'
        EXPECT_EQ(Base0->getTypeKind(), ASTTypeKind::TYPE_NAMED);
        ASTNamedType *Named = As<ASTNamedType>(Base0);
        ASSERT_TRUE(Named != nullptr);
        const auto &Names = Named->getNames();
        ASSERT_TRUE(!Names.empty());
        EXPECT_EQ(Names[0]->getName(), "Test");
    }

    TEST_F(ParserTest, ClassExtendClass) {
         llvm::StringRef str = (
	         "public class Case : Test {\n"
	         "  int b\n"
	         "  void f() {}\n"
	         "}\n"
	         "class Test {\n"
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
        EXPECT_TRUE(HasBuiltinType(fMethod->getReturnType(), ASTBuiltinTypeKind::TYPE_VOID));
        // method body should be present and empty
        ASTBlockStmt *FBody = fMethod->getBody();
        ASSERT_TRUE(FBody != nullptr);
        EXPECT_EQ(FBody->getContent().size(), 0);
     }

    TEST_F(ParserTest, EnumExtendEnum) {
        llvm::StringRef str = ("public enum Option : Enum {\n"
                               "  B C\n"
                               "}\n");
        ASTModule *Module = Parse("Option", str);

        llvm::StringRef str2 = (
                "enum Enum {\n"
                "  A\n"
                "}\n");
        ASTModule *Module2 = Parse("Enum", str2);

        // Verify Option enum exists and extends Enum
        ASTEnum *Opt = As<ASTEnum>(Module->getNodes()[0]);
        ASSERT_TRUE(Opt != nullptr);
        EXPECT_EQ(Opt->getName(), "Option");
        EXPECT_TRUE(HasModifier(Opt->getModifiers(), ASTModifierKind::MOD_PUBLIC));
        // entries B, C
        EXPECT_EQ(Opt->getNodes().size(), 2);
        ASTEnumEntry *B = As<ASTEnumEntry>(Opt->getNodes()[0]);
        ASTEnumEntry *C = As<ASTEnumEntry>(Opt->getNodes()[1]);
        ASSERT_TRUE(B != nullptr);
        ASSERT_TRUE(C != nullptr);
        EXPECT_EQ(B->getName(), "B");
        EXPECT_EQ(C->getName(), "C");

        // Check super classes (should include named type 'Enum')
        const auto &Supers = Opt->getBases();
        EXPECT_EQ(Supers.size(), 1);
        ASTType *Base0 = Supers[0];
        ASSERT_TRUE(Base0 != nullptr);
        EXPECT_EQ(Base0->getTypeKind(), ASTTypeKind::TYPE_NAMED);
        ASTNamedType *Named = As<ASTNamedType>(Base0);
        ASSERT_TRUE(Named != nullptr);
        const auto &Names = Named->getNames();
        ASSERT_TRUE(!Names.empty());
        EXPECT_EQ(Names[0]->getName(), "Enum");
    }

    TEST_F(ParserTest, ClassExtendAll) {
        llvm::StringRef str = ("public class Test : Class Struct Interface {\n"
                               "}\n");
        ASTModule *Module = Parse("Test", str);

        llvm::StringRef str2 = (
                "class Class {\n"
                "}\n");
        ASTModule *Module2 = Parse("Class", str2);

        llvm::StringRef str3 = (
                "struct Struct {\n"
                "}\n");
        ASTModule *Module3 = Parse("Struct", str3);

        llvm::StringRef str4 = (
                "interface Interface {\n"
                "}\n");
        ASTModule *Module4 = Parse("Interface", str4);

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
