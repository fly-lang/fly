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
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTClassMethod.h"

namespace {

    using namespace fly;

    TEST_F(ParserTest, GlobalVars) {
        llvm::StringRef str = ("private Obj o");
        ASTModule *Module = Parse("GlobalVars", str);

        ASSERT_TRUE(Resolve());

        ASTGlobalVar *VarO = Module->getGlobalVars().front();

        EXPECT_EQ(VarO->getVisibility(), ASTVisibilityKind::V_PRIVATE);
        EXPECT_FALSE(VarO->isConstant());
        EXPECT_EQ(((ASTIntegerType *) VarO->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_INT);
        EXPECT_EQ(VarO->getName(), "o");
    }

    TEST_F(ParserTest, NullTypeVarReturn) {
        llvm::StringRef str = ("Type func() {\n"
                               "  Type t = null"
                               "  return t\n"
                               "}\n");
        ASTModule *Module = Parse("TypeDefaultVarReturn", str);

        ASSERT_TRUE(Resolve());


        // Get Body
        ASTFunction *F = Module->getFunctions()[0];
        EXPECT_EQ(F->getReturnType()->getKind(), ASTTypeKind::TYPE_IDENTITY);
        const ASTBlockStmt *Body = F->getBody();

        // Test: Type t
        ASTAssignmentStmt *varStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        EXPECT_EQ(varStmt->getVarRef()->getName(), "t");
        ASTIdentityType *ClassType = (ASTIdentityType *) varStmt->getVarRef()->getDef()->getType();
        EXPECT_EQ(ClassType->getKind(), ASTTypeKind::TYPE_IDENTITY);
        EXPECT_EQ(ClassType->getName(), "Type");
        ASSERT_EQ(((ASTNullValue *)((ASTValueExpr *) varStmt->getExpr())->getValue())->print(), "null");

        const ASTReturnStmt *Ret = (ASTReturnStmt *) Body->getContent()[1];
        ASTVarRefExpr *RetRef = (ASTVarRefExpr *) Ret->getExpr();
        EXPECT_EQ(RetRef->getVarRef()->getName(), "t");

    }

    // TEST_F(ParserTest, NullTypeVarReturn) {
    //     llvm::StringRef str = ("Type func() {\n"
    //                            "  Type t = null"
    //                            "  return t\n"
    //                            "}\n");
    //     ASTModule *Module = Parse("TypeDefaultVarReturn", str);
    //
    //     ASSERT_TRUE(Resolve());
    //
    //
    //     // Get Body
    //     ASTFunction *F = *Module->getFunctions().begin()->getValue().begin()->second.begin();
    //     EXPECT_EQ(F->getReturnType()->getKind(), ASTTypeKind::TYPE_IDENTITY);
    //     const ASTBlockStmt *Body = F->getBody();
    //
    //     // Test: Type t
    //     ASTVarStmt *varStmt = (ASTVarStmt *) Body->getContent()[0];
    //     EXPECT_EQ(varStmt->getVarRef()->getName(), "t");
    //     ASTIdentityType *ClassType = (ASTIdentityType *) varStmt->getVarRef()->getDef()->getType();
    //     EXPECT_EQ(ClassType->getKind(), ASTTypeKind::TYPE_IDENTITY);
    //     EXPECT_EQ(ClassType->getName(), "Type");
    //     ASSERT_EQ(((ASTNullValue *)((ASTValueExpr *) varStmt->getExpr())->getValue())->print(), "null");
    //
    //     const ASTReturnStmt *Ret = (ASTReturnStmt *) Body->getContent()[1];
    //     ASTVarRefExpr *RetRef = (ASTVarRefExpr *) Ret->getExpr();
    //     EXPECT_EQ(RetRef->getVarRef()->getName(), "t");
    //
    // }

    TEST_F(ParserTest, ClassEmpty) {
        llvm::StringRef str = ("public class Test {}\n");
        ASTModule *Module = Parse("ClassEmpty", str);
        ASSERT_TRUE(isSuccess());

        EXPECT_FALSE(Module->getIdentity() == nullptr);
        EXPECT_TRUE(Module->getNameSpace()->getIdentities().size() == 1);
        auto &Class = *Module->getNameSpace()->getIdentities().begin()->second;
        EXPECT_EQ(Class.getScopes()->getVisibility(), ASTVisibilityKind::V_PUBLIC);
        const auto &NSClassess = Module->getContext().getDefaultNameSpace()->getIdentities();
        const auto &ClassTest = NSClassess.find("Test");
        ASSERT_TRUE(ClassTest != NSClassess.end());
    }

    TEST_F(ParserTest, Enum) {
        llvm::StringRef str = ("public enum Test {\n"
                               "  A B C\n"
                               "}\n");
        ASTModule *Module = Parse("TestEnum", str, false);

        ASTEnum *Enum = (ASTEnum *) Module->getIdentity();
        EXPECT_FALSE(Enum->getVars().empty());
        EXPECT_EQ(Enum->getVars().size(), 3); // A B C enum
        ASTEnumEntry *VarA = Enum->getVars().find("A")->getValue();
        ASTEnumEntry *VarB = Enum->getVars().find("B")->getValue();
        ASTEnumEntry *VarC = Enum->getVars().find("C")->getValue();
        EXPECT_EQ(VarA->getIndex(), 1);
        EXPECT_EQ(VarB->getIndex(), 2);
        EXPECT_EQ(VarC->getIndex(), 3);

        llvm::StringRef str2 = (
                "void main() {\n"
                "  Test a = Test.A\n"
                "  a = Test.B"
                "  Test c = a"
                "}\n");
        ASTModule *Module2 = Parse("func", str2);
        ASSERT_TRUE(isSuccess());

        ASTFunction *main = *Module2->getFunctions().find("main")->getValue().begin()->second.begin();
        const ASTBlock *Body = main->getBody();
        ASTVarStmt *aVar = ((ASTVarStmt *) Body->getContent()[0]);
        ASTVarRefExpr *aExpr = (ASTVarRefExpr *) aVar->getExpr();
        ASTEnumEntry *A = (ASTEnumEntry *) aExpr->getVarRef()->getDef();
    }

    TEST_F(ParserTest, Struct) {
        llvm::StringRef str = ("public struct Test {\n"
                               "  int a\n"
                               "  public int b = 2\n"
                               "  const int c = 0\n"
                               "}\n");
        ASTModule *Module = Parse("TestStruct", str, false);

        ASTClass *Class = (ASTClass *) Module->getIdentity();
        EXPECT_FALSE(Class->getVars().empty());
        EXPECT_EQ(Class->getVars().size(), 3);
        ASTClassVar *aVar = Class->getVars().find("a")->getValue();
        ASTClassVar *bVar = Class->getVars().find("b")->getValue();
        ASTClassVar *cVar = Class->getVars().find("c")->getValue();
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
        ASTModule *Module1 = Parse("func1", str2, false);
        ASTFunction *func1 = *Module1->getFunctions().find("func1")->getValue().begin()->second.begin();
        const ASTBlock *Body1 = func1->getBody();
        ASTVarStmt *tVar1 = ((ASTVarStmt *) Body1->getContent()[0]);

        llvm::StringRef str3 = (
                "void func2() {\n"
                "  Test t = { a = 3, b = 1}"
                "}\n");
        ASTModule *Module2 = Parse("func2", str3);
        ASTFunction *func2 = *Module2->getFunctions().find("func2")->getValue().begin()->second.begin();
        const ASTBlock *Body2 = func2->getBody();
        ASTVarStmt *tVar2 = ((ASTVarStmt *) Body2->getContent()[0]);

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
        ASTModule *Module = Parse("TestClass", str, false);
        ASTClass *Class = (ASTClass *) Module->getIdentity();

        EXPECT_EQ(Class->getVars().size(), 2);
        EXPECT_EQ(Class->getMethods().size(), 4);
        ASTClassVar *aVar = Class->getVars().find("a")->getValue();
        ASTClassVar *bVar = Class->getVars().find("b")->getValue();
        ASTClassFunction *aMethod = *Class->getMethods().find("a")->getValue().begin()->second.begin();
        ASTClassFunction *bMethod = *Class->getMethods().find("b")->getValue().begin()->second.begin();
        ASTClassFunction *cMethod = *Class->getMethods().find("c")->getValue().begin()->second.begin();
        ASTClassFunction *dMethod = *Class->getMethods().find("d")->getValue().begin()->second.begin();
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
        ASTModule *Module2 = Parse("Identifier", str2);

        ASSERT_TRUE(isSuccess());

    }

    TEST_F(ParserTest, StructExtendStruct) {
        llvm::StringRef str = ("public struct Case : Test {\n"
                               "  int b\n"
                               "}\n");
        ASTModule *Module = Parse("CaseStruct", str, false);

        llvm::StringRef str2 = (
                "struct Test {\n"
                "  int a\n"
                "}\n");
        ASTModule *Module2 = Parse("TestStruct", str2);

        ASSERT_TRUE(isSuccess());
        ASTClass *Class = (ASTClass *) Module->getIdentity();

        EXPECT_EQ(Class->getVars().size(), 2);
        ASTClassVar &aVar = *Class->getVars().find("a")->getValue();
        ASTClassVar &bVar = *Class->getVars().find("b")->getValue();
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
        ASTModule *Module = Parse("CaseClass", str, false);

        llvm::StringRef str2 = (
                "class Test {\n"
                "  int a\n"
                "  private void f1() {}\n"
                "  protected void f2() {}\n"
                "  public void f3() {}\n"
                "  void f() {}\n"
                "}\n");
        ASTModule *Module2 = Parse("TestClass", str2);

        ASSERT_TRUE(isSuccess());
        ASTClass *Class = (ASTClass *) Module->getIdentity();
        ASTClass *Class2 = (ASTClass *) Module2->getIdentity();

        EXPECT_EQ(Class2->getVars().size(), 1);
        ASTClassVar &aVar = *Class2->getVars().find("a")->getValue();
        EXPECT_EQ(Class2->getMethods().size(), 4);
        ASTClassFunction *f1Method = *Class2->getMethods().find("f1")->getValue().begin()->second.begin();
        ASTClassFunction *f2Method = *Class2->getMethods().find("f2")->getValue().begin()->second.begin();
        ASTClassFunction *f3Method = *Class2->getMethods().find("f3")->getValue().begin()->second.begin();
        ASTClassFunction *fMethod1 = *Class2->getMethods().find("f")->getValue().begin()->second.begin();

        EXPECT_EQ(Class->getVars().size(), 1);
        ASTClassVar &bVar = *Class->getVars().find("b")->getValue();
        EXPECT_EQ(Class->getMethods().size(), 4);
        ASTClassFunction *fMethod = *Class->getMethods().find("f")->getValue().begin()->second.begin();
        EXPECT_EQ(fMethod->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_FALSE(fMethod->getScopes()->isConstant());
    }

    TEST_F(ParserTest, ClassExtendStruct) {
        llvm::StringRef str = ("public class Case : Test {\n"
                               "  int b\n"
                               "}\n");
        ASTModule *Module = Parse("ClassCase", str, false);

        llvm::StringRef str2 = (
                "struct Test {\n"
                "  int a\n"
                "}\n");
        ASTModule *Module2 = Parse("StructTest", str2);

        ASSERT_TRUE(isSuccess());
        ASTClass *Class = (ASTClass *) Module->getIdentity();
        ASTClass *Class2 = (ASTClass *) Module2->getIdentity();

        EXPECT_EQ(Class2->getVars().size(), 1);
        ASTClassVar &a_Var = *Class2->getVars().find("a")->getValue();
        EXPECT_EQ(a_Var.getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);

        EXPECT_EQ(Class->getVars().size(), 2);
        ASTClassVar &aVar = *Class->getVars().find("a")->getValue();
        EXPECT_EQ(aVar.getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASTClassVar &bVar = *Class->getVars().find("b")->getValue();
        EXPECT_EQ(bVar.getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
    }

    TEST_F(ParserTest, ClassExtendInterface) {
        llvm::StringRef str = ("public class Class : Interface {\n"
                               "  int a() { return 1 }\n"
                               "}\n");
        ASTModule *Module = Parse("Class", str, false);

        llvm::StringRef str2 = (
                "interface Interface {\n"
                "  int a()\n"
                "}\n");
        ASTModule *Module2 = Parse("Interface", str2);

        ASSERT_TRUE(isSuccess());

        ASTClass *Class = (ASTClass *) Module->getIdentity();
        ASTClass *Class2 = (ASTClass *) Module2->getIdentity();

        EXPECT_EQ(Class2->getMethods().size(), 1);
        EXPECT_EQ(Class->getMethods().size(), 1);
        ASTClassFunction *aMethod = *Class->getMethods().find("a")->getValue().begin()->second.begin();
        EXPECT_EQ(aMethod->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_FALSE(aMethod->getScopes()->isConstant());
    }

    TEST_F(ParserTest, DISABLED_EnumExtendEnum) {
        llvm::StringRef str = ("public enum Option : Enum {\n"
                               "  B C\n"
                               "}\n");
        ASTModule *Module = Parse("Option", str, false);

        llvm::StringRef str2 = (
                "enum Enum {\n"
                "  A\n"
                "}\n");
        ASTModule *Module2 = Parse("Enum", str2);

        ASSERT_TRUE(isSuccess());
        ASTClass *Class = (ASTClass *) Module->getIdentity();
        ASTClass *Class2 = (ASTClass *) Module2->getIdentity();

        EXPECT_EQ(Class2->getVars().size(), 2); // A enum
        ASTClassVar *Var_A = Class2->getVars().find("A")->getValue();
        EXPECT_EQ(Class->getVars().size(), 4); // A B C enum
        ASTClassVar *VarA = Class->getVars().find("A")->getValue();
        ASTClassVar *VarB = Class->getVars().find("B")->getValue();
        ASTClassVar *VarC = Class->getVars().find("C")->getValue();
    }

    TEST_F(ParserTest, ClassExtendAll) {
        llvm::StringRef str = ("public class Test : Class Struct Interface {\n"
//                               "  int a() { return a }\n"
                               "}\n");
        ASTModule *Module = Parse("Test", str, false);

        llvm::StringRef str2 = (
                "class Class {\n"
                "  int a() { return 1 }"
                "}\n");
        ASTModule *Module2 = Parse("Class", str2, false);

        llvm::StringRef str3 = (
                "struct Struct {\n"
                "  int a"
                "}\n");
        ASTModule *Module3 = Parse("Struct", str3, false);

        llvm::StringRef str4 = (
                "interface Interface {\n"
                "  int a()"
                "}\n");
        ASTModule *Module4 = Parse("Interface", str4);

        ASSERT_TRUE(isSuccess());
        ASTClass *Class = (ASTClass *) Module->getIdentity();
        EXPECT_EQ(Class->getMethods().size(), 1);
        ASTClassFunction *aMethod = *Class->getMethods().find("a")->getValue().begin()->second.begin();
        EXPECT_EQ(aMethod->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_FALSE(aMethod->getScopes()->isConstant());

        EXPECT_EQ(Class->getVars().size(), 1); // A enum
        ASTClassVar *VarA = Class->getVars().find("a")->getValue();
        EXPECT_EQ(VarA->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
    }
}