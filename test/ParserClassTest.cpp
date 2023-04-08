//===--------------------------------------------------------------------------------------------------------------===//
// test/IdentifierParserTest.cpp - Identifier Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "ParserTest.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTBlock.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarAssign.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTParams.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"

namespace {

    using namespace fly;

    TEST_F(ParserTest, ClassEmpty) {
        llvm::StringRef str = ("public class Test {}\n");
        ASTNode *Node = Parse("ClassEmpty", str);
        ASSERT_TRUE(isSuccess());

        EXPECT_FALSE(Node->getClass() == nullptr);
        EXPECT_TRUE(Node->getNameSpace()->getClasses().size() == 1);
        ASTClass &Class = *Node->getNameSpace()->getClasses().begin()->second;
        EXPECT_EQ(Class.getScopes()->getVisibility(), ASTVisibilityKind::V_PUBLIC);
        const auto &NSClassess = Node->getContext().getDefaultNameSpace()->getClasses();
        const auto &ClassTest = NSClassess.find("Test");
        ASSERT_TRUE(ClassTest != NSClassess.end());
    }

    TEST_F(ParserTest, Enum) {
        llvm::StringRef str = ("public enum Test {\n"
                               "  A B C\n"
                               "}\n");
        ASTNode *Node = Parse("TestStruct", str, false);

        EXPECT_FALSE(Node->getClass()->getVars().empty());
        EXPECT_EQ(Node->getClass()->getVars().size(), 4); // A B C enum
        ASTClassVar *VarA = Node->getClass()->getVars().find("A")->getValue();
        ASTClassVar *VarB = Node->getClass()->getVars().find("B")->getValue();
        ASTClassVar *VarC = Node->getClass()->getVars().find("C")->getValue();
        ASTClassVar *VarEnum = Node->getClass()->getVars().find("enum")->getValue();
        EXPECT_EQ(VarA->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_TRUE(VarA->getScopes()->isConstant());
        EXPECT_EQ(VarB->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_TRUE(VarB->getScopes()->isConstant());
        EXPECT_EQ(VarC->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_TRUE(VarC->getScopes()->isConstant());
        EXPECT_EQ(VarEnum->getScopes()->getVisibility(), ASTVisibilityKind::V_PRIVATE);
        EXPECT_TRUE(VarEnum->getScopes()->isConstant());

        llvm::StringRef str2 = (
                "void main() {\n"
                "  Test a = Test.A\n"
                "  a = Test.B"
                "  Test c = a"
                "}\n");
        ASTNode *Node2 = Parse("func", str2);
        ASSERT_TRUE(isSuccess());

        ASTFunction *main = *Node2->getFunctions().find("main")->getValue().begin()->second.begin();
        const ASTBlock *Body = main->getBody();
        ASTLocalVar *aVar = ((ASTLocalVar *) Body->getContent()[0]);
        ASTVarRefExpr *aExpr = (ASTVarRefExpr *) aVar->getExpr();
        ASTClassVar *A = (ASTClassVar *) aExpr->getVarRef()->getDef();
    }

    TEST_F(ParserTest, Struct) {
        llvm::StringRef str = ("public struct Test {\n"
                               "  int a\n"
                               "  public int b = 2\n"
                               "  const int c = 0\n"
                               "}\n");
        ASTNode *Node = Parse("TestStruct", str, false);

        EXPECT_FALSE(Node->getClass()->getVars().empty());
        EXPECT_EQ(Node->getClass()->getVars().size(), 3);
        ASTClassVar *aVar = Node->getClass()->getVars().find("a")->getValue();
        ASTClassVar *bVar = Node->getClass()->getVars().find("b")->getValue();
        ASTClassVar *cVar = Node->getClass()->getVars().find("c")->getValue();
        EXPECT_EQ(aVar->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_FALSE(aVar->getScopes()->isConstant());
        EXPECT_EQ(bVar->getScopes()->getVisibility(), ASTVisibilityKind::V_PUBLIC);
        EXPECT_FALSE(bVar->getScopes()->isConstant());
        EXPECT_EQ(cVar->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_TRUE(cVar->getScopes()->isConstant());

        llvm::StringRef str2 = (
                "void func1() {\n"
                "  Test t = new Test()"
                "  t.a = 3"
                "  t.b = t.c"
                "}\n");
        ASTNode *Node1 = Parse("func1", str2, false);
        ASTFunction *func1 = *Node1->getFunctions().find("func1")->getValue().begin()->second.begin();
        const ASTBlock *Body1 = func1->getBody();
        ASTLocalVar *tVar1 = ((ASTLocalVar *) Body1->getContent()[0]);

        llvm::StringRef str3 = (
                "void func2() {\n"
                "  Test t = { a = 3, b = 1}"
                "}\n");
        ASTNode *Node2 = Parse("func2", str3);
        ASTFunction *func2 = *Node2->getFunctions().find("func2")->getValue().begin()->second.begin();
        const ASTBlock *Body2 = func2->getBody();
        ASTLocalVar *tVar2 = ((ASTLocalVar *) Body2->getContent()[0]);

        ASSERT_TRUE(isSuccess());
    }

    TEST_F(ParserTest, Class) {
        llvm::StringRef str = ("public class Test {\n"
                               "  int a = 1\n"
                               "  private int b = 1\n"
                               "  public int a() { return a }\n"
                               "  protected int b() { return 2 }\n"
                               "  private int c() { return 3 }\n"
                               "  const int d() { return 0 }\n"
                               "}\n");
        ASTNode *Node = Parse("TestClass", str, false);

        EXPECT_EQ(Node->getClass()->getVars().size(), 2);
        EXPECT_EQ(Node->getClass()->getMethods().size(), 4);
        ASTClassVar *aVar = Node->getClass()->getVars().find("a")->getValue();
        ASTClassVar *bVar = Node->getClass()->getVars().find("b")->getValue();
        ASTClassFunction *aMethod = *Node->getClass()->getMethods().find("a")->getValue().begin()->second.begin();
        ASTClassFunction *bMethod = *Node->getClass()->getMethods().find("b")->getValue().begin()->second.begin();
        ASTClassFunction *cMethod = *Node->getClass()->getMethods().find("c")->getValue().begin()->second.begin();
        ASTClassFunction *dMethod = *Node->getClass()->getMethods().find("d")->getValue().begin()->second.begin();
        EXPECT_EQ(aVar->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_EQ(bVar->getScopes()->getVisibility(), ASTVisibilityKind::V_PRIVATE);
        EXPECT_EQ(aMethod->getScopes()->getVisibility(), ASTVisibilityKind::V_PUBLIC);
        EXPECT_EQ(bMethod->getScopes()->getVisibility(), ASTVisibilityKind::V_PROTECTED);
        EXPECT_EQ(cMethod->getScopes()->getVisibility(), ASTVisibilityKind::V_PRIVATE);
        EXPECT_EQ(dMethod->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_TRUE(dMethod->getScopes()->isConstant());

        llvm::StringRef str2 = (
                "void func() {\n"
                "  Test t = new Test()\n"
                "  t.a()\n"
                "  t.d()\n"
                "}\n");
        ASTNode *Node2 = Parse("Identifier", str2);

        ASSERT_TRUE(isSuccess());

    }

    TEST_F(ParserTest, StructExtendStruct) {
        llvm::StringRef str = ("public struct Case : Test {\n"
                               "  int b\n"
                               "}\n");
        ASTNode *Node = Parse("CaseStruct", str, false);

        llvm::StringRef str2 = (
                "struct Test {\n"
                "  int a\n"
                "}\n");
        ASTNode *Node2 = Parse("TestStruct", str2);

        ASSERT_TRUE(isSuccess());

        EXPECT_EQ(Node->getClass()->getVars().size(), 2);
        ASTClassVar &aVar = *Node->getClass()->getVars().find("a")->getValue();
        ASTClassVar &bVar = *Node->getClass()->getVars().find("b")->getValue();
        EXPECT_EQ(aVar.getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_EQ(bVar.getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_FALSE(aVar.getScopes()->isConstant());
        EXPECT_FALSE(bVar.getScopes()->isConstant());
    }

    TEST_F(ParserTest, ClassExtendClass) {
        llvm::StringRef str = ("public class Case : Test {\n"
                               "  int b\n"
                               "  void f() {}\n"
                               "}\n");
        ASTNode *Node = Parse("CaseClass", str, false);

        llvm::StringRef str2 = (
                "class Test {\n"
                "  int a\n"
                "  private void f1() {}\n"
                "  protected void f2() {}\n"
                "  public void f3() {}\n"
                "  void f() {}\n"
                "}\n");
        ASTNode *Node2 = Parse("TestClass", str2);

        ASSERT_TRUE(isSuccess());

        EXPECT_EQ(Node2->getClass()->getVars().size(), 1);
        ASTClassVar &aVar = *Node2->getClass()->getVars().find("a")->getValue();
        EXPECT_EQ(Node2->getClass()->getMethods().size(), 4);
        ASTClassFunction *f1Method = *Node2->getClass()->getMethods().find("f1")->getValue().begin()->second.begin();
        ASTClassFunction *f2Method = *Node2->getClass()->getMethods().find("f2")->getValue().begin()->second.begin();
        ASTClassFunction *f3Method = *Node2->getClass()->getMethods().find("f3")->getValue().begin()->second.begin();
        ASTClassFunction *fMethod1 = *Node2->getClass()->getMethods().find("f")->getValue().begin()->second.begin();

        EXPECT_EQ(Node->getClass()->getVars().size(), 1);
        ASTClassVar &bVar = *Node->getClass()->getVars().find("b")->getValue();
        EXPECT_EQ(Node->getClass()->getMethods().size(), 4);
        ASTClassFunction *fMethod = *Node->getClass()->getMethods().find("f")->getValue().begin()->second.begin();
        EXPECT_EQ(fMethod->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_FALSE(fMethod->getScopes()->isConstant());
    }

    TEST_F(ParserTest, ClassExtendStruct) {
        llvm::StringRef str = ("public class Case : Test {\n"
                               "  int b\n"
                               "}\n");
        ASTNode *Node = Parse("ClassCase", str, false);

        llvm::StringRef str2 = (
                "struct Test {\n"
                "  int a\n"
                "}\n");
        ASTNode *Node2 = Parse("StructTest", str2);

        ASSERT_TRUE(isSuccess());

        EXPECT_EQ(Node2->getClass()->getVars().size(), 1);
        ASTClassVar &a_Var = *Node2->getClass()->getVars().find("a")->getValue();
        EXPECT_EQ(a_Var.getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);

        EXPECT_EQ(Node->getClass()->getVars().size(), 2);
        ASTClassVar &aVar = *Node->getClass()->getVars().find("a")->getValue();
        EXPECT_EQ(aVar.getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASTClassVar &bVar = *Node->getClass()->getVars().find("b")->getValue();
        EXPECT_EQ(bVar.getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
    }

    TEST_F(ParserTest, ClassExtendInterface) {
        llvm::StringRef str = ("public class Class : Interface {\n"
                               "  int a() { return 1 }\n"
                               "}\n");
        ASTNode *Node = Parse("Class", str, false);

        llvm::StringRef str2 = (
                "interface Interface {\n"
                "  int a()\n"
                "}\n");
        ASTNode *Node2 = Parse("Interface", str2);

        ASSERT_TRUE(isSuccess());

        EXPECT_EQ(Node2->getClass()->getMethods().size(), 1);
        EXPECT_EQ(Node->getClass()->getMethods().size(), 1);
        ASTClassFunction *aMethod = *Node->getClass()->getMethods().find("a")->getValue().begin()->second.begin();
        EXPECT_EQ(aMethod->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_FALSE(aMethod->getScopes()->isConstant());
    }

    TEST_F(ParserTest, EnumExtendEnum) {
        llvm::StringRef str = ("public enum Option : Enum {\n"
                               "  B C\n"
                               "}\n");
        ASTNode *Node = Parse("Option", str, false);

        llvm::StringRef str2 = (
                "enum Enum {\n"
                "  A\n"
                "}\n");
        ASTNode *Node2 = Parse("Enum", str2);

        ASSERT_TRUE(isSuccess());

        EXPECT_EQ(Node2->getClass()->getVars().size(), 2); // A enum
        ASTClassVar *Var_A = Node2->getClass()->getVars().find("A")->getValue();
        EXPECT_EQ(Node->getClass()->getVars().size(), 4); // A B C enum
        ASTClassVar *VarA = Node->getClass()->getVars().find("A")->getValue();
        ASTClassVar *VarB = Node->getClass()->getVars().find("B")->getValue();
        ASTClassVar *VarC = Node->getClass()->getVars().find("C")->getValue();
    }

    TEST_F(ParserTest, ClassExtendAll) {
        llvm::StringRef str = ("public class Test : Class Struct Interface {\n"
//                               "  int a() { return a }\n"
                               "}\n");
        ASTNode *Node = Parse("Test", str, false);

        llvm::StringRef str2 = (
                "class Class {\n"
                "  int a() { return 1 }"
                "}\n");
        ASTNode *Node2 = Parse("Class", str2, false);

        llvm::StringRef str3 = (
                "struct Struct {\n"
                "  int a"
                "}\n");
        ASTNode *Node3 = Parse("Struct", str3, false);

        llvm::StringRef str4 = (
                "interface Interface {\n"
                "  int a()"
                "}\n");
        ASTNode *Node4 = Parse("Interface", str4);

        ASSERT_TRUE(isSuccess());

        EXPECT_EQ(Node->getClass()->getMethods().size(), 1);
        ASTClassFunction *aMethod = *Node->getClass()->getMethods().find("a")->getValue().begin()->second.begin();
        EXPECT_EQ(aMethod->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_FALSE(aMethod->getScopes()->isConstant());

        EXPECT_EQ(Node->getClass()->getVars().size(), 1); // A enum
        ASTClassVar *VarA = Node->getClass()->getVars().find("a")->getValue();
        EXPECT_EQ(VarA->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
    }
}