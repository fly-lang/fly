//===--------------------------------------------------------------------------------------------------------------===//
// test/CodeGenClassTest.cpp - CodeGen Class tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

// fly
#include "AST/ASTBinary.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTValue.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGenTest.h"

#include <AST/ASTAttribute.h>
#include <AST/ASTDeclStmt.h>
#include <AST/ASTExprStmt.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTMethod.h>
#include <AST/ASTParam.h>
#include <AST/ASTReturnStmt.h>

namespace {

    using namespace fly;

	TEST_F(CodeGenTest, CGClassAttributes) {
        /**
         * Fly code:
         * class TestClass {
         *   int a
         * }
         * void func() {
         *   TestClass test = new TestClass()
         *   test.a = 2
         *   delete test
         * }
         */
        ASTModule *Module = CreateModule();

        // TestClass {
        //   int a
        // }
        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass", TopModifiers,
                                                  SuperClasses);

        // int a
        ASTVar *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, TestClass, IntTypeRef, "a", TopModifiers);

        // int main() {
        //  TestClass test = new TestClass()
        //  test.a = 2
        //  delete test
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // TestClass test = new TestClass()
        ASTType *TestClassType = CreateType(TestClass);
        ASTLocalVar *TestVar = ASTBuilder::CreateLocalVar(SourceLoc, TestClassType, "test", EmptyModifiers);
        ASTDeclStmt *TestDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TestVar);
        ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);
        ASTIdentifier *testIdent = ASTBuilder::CreateIdentifier(TestVar);
        ASTBinary *AssignExpr1 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, testIdent, ConstructorCall);
        TestDeclStmt->setExpr(AssignExpr1);

        //  test.a = 2
    	ASTMember *test_a = ASTBuilder::CreateMember(SourceLoc, aAttribute->getName(), ASTBuilder::CreateIdentifier(TestVar));
        ASTExprStmt * attrStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTValue *value2 = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        ASTBinary *AssignExpr2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, test_a, value2);
        attrStmt->setExpr(AssignExpr2);

        // delete test
        ASTBuilder::CreateDeleteStmt(Body, SourceLoc, ASTBuilder::CreateIdentifier(TestVar));

		// Generate Code
		Generate();
		llvm::Module * M = getModules()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n"
                        "%error = type { i32, ptr, ptr }\n"
                        "%TestClass = type { ptr, i32 }\n"
                        "\n"
                        "@error = external constant %error\n"
                        "@vtable.TestClass = constant [2 x ptr] [ptr null, ptr @TestClass_F9TestClass]\n"
                        "\n"
                        "define linkonce_odr ptr @TestClass.init_ctor(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %2 = load ptr, ptr %1, align 8\n"
                        "  %3 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 0\n"
                        "  store ptr @vtable.TestClass, ptr %3, align 8\n"
                        "  %4 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 1\n"
                        "  store i32 0, ptr %4, align 4\n"
                        "  ret ptr %2\n"
                        "}\n"
                        "\n"
                        "define void @TestClass_F9TestClass(ptr %0, ptr %1) {\n"
                        "entry:\n"
                        "  %2 = alloca ptr, align 8\n"
                        "  %3 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %2, align 8\n"
                        "  store ptr %1, ptr %3, align 8\n"
                        "  ret void\n"
                        "}\n"
                        "\n"
                        "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %3 = load ptr, ptr %1, align 8\n"
                        "  %4 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%TestClass, ptr null, i32 1) to i64))\n"
                        "  %5 = call ptr @TestClass.init_ctor(ptr %4)\n"
                        "  call void @TestClass_F9TestClass(ptr %3, ptr %5)\n"
                        "  store ptr %5, ptr %2, align 8\n"
                        "  %6 = load ptr, ptr %2, align 8\n"
                        "  %7 = getelementptr inbounds %TestClass, ptr %6, i32 0, i32 1\n"
                        "  store i32 2, ptr %7, align 4\n"
                        "  call void @free(ptr %6)\n"
                        "  ret void\n"
                        "}\n"
                        "\n"
                        "declare ptr @malloc(i64)\n"
                        "\n"
                        "declare void @free(ptr)\n");
    }

	TEST_F(CodeGenTest, CGClassGetterMethod) {
        /**th
         * Fly code:
         * class TestClass {
         *   int a
         *   getA(int a) {
         *     a = this.a
         *   }
         * }
         * void func() {
         *   TestClass test = new TestClass()
         *   int x
         *   test.getA(x)
         *   delete test
         * }
         */
        ASTModule *Module = CreateModule();

        // TestClass { int a; getA(int a) { a = this.a } }
        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass", TopModifiers,
                                                  SuperClasses);

        // int a
        ASTVar *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, TestClass, IntTypeRef, "a", TopModifiers);

        // getA(int a) { a = this.a }
        llvm::SmallVector<ASTParam *, 8> getAParams;
        ASTParam *aParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        getAParams.push_back(aParam);
        ASTBlockStmt *MethodBody = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTMethod *getAMethod = ASTBuilder::CreateClassMethod(SourceLoc, TestClass,
                                                               "getA", TopModifiers, getAParams, MethodBody);

        // a = this.a
        ASTMember *thisA = ASTBuilder::CreateMember(SourceLoc, aAttribute->getName(),
                                                     ASTBuilder::CreateIdentifier(SourceLoc, "this"));
        ASTExprStmt *assignStmt = ASTBuilder::CreateExprStmt(MethodBody, SourceLoc);
        ASTIdentifier *aParamIdent = ASTBuilder::CreateIdentifier(aParam);
        ASTBinary *MethodAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                                aParamIdent, thisA);
        assignStmt->setExpr(MethodAssignExpr);

        // void func() { ... }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // TestClass test = new TestClass()
        ASTType *TestClassType = CreateType(TestClass);
        ASTLocalVar *TestVar = ASTBuilder::CreateLocalVar(SourceLoc, TestClassType, "test", EmptyModifiers);
        ASTDeclStmt *TestDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TestVar);
        ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);
        ASTIdentifier *testIdent = ASTBuilder::CreateIdentifier(TestVar);
        ASTBinary *AssignExpr1 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, testIdent, ConstructorCall);
        TestDeclStmt->setExpr(AssignExpr1);

        // int x
        ASTLocalVar *xVar = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "x", EmptyModifiers);
        ASTBuilder::CreateDeclStmt(Body, SourceLoc, xVar);

        // test.getA(x)
        llvm::SmallVector<ASTExpr *, 8> getAArgs;
        getAArgs.push_back(ASTBuilder::CreateIdentifier(xVar));
        ASTCall *getACallExpr = ASTBuilder::CreateCall(SourceLoc, getAMethod->getName(), getAArgs,
                                                        ASTCallKind::CALL_DIRECT,
                                                        ASTBuilder::CreateIdentifier(TestVar));
        ASTExprStmt *callStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        callStmt->setExpr(getACallExpr);

        // delete test
        ASTBuilder::CreateDeleteStmt(Body, SourceLoc, ASTBuilder::CreateIdentifier(TestVar));

		// Generate Code
		Generate();
		llvm::Module * M = getModules()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n"
                        "%error = type { i32, ptr, ptr }\n"
                        "%TestClass = type { ptr, i32 }\n"
                        "\n"
                        "@error = external constant %error\n"
                        "@vtable.TestClass = constant [3 x ptr] [ptr null, ptr @TestClass_F4getA_i, ptr @TestClass_F9TestClass]\n"
                        "\n"
                        "define linkonce_odr ptr @TestClass.init_ctor(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %2 = load ptr, ptr %1, align 8\n"
                        "  %3 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 0\n"
                        "  store ptr @vtable.TestClass, ptr %3, align 8\n"
                        "  %4 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 1\n"
                        "  store i32 0, ptr %4, align 4\n"
                        "  ret ptr %2\n"
                        "}\n"
                        "\n"
                        "define void @TestClass_F4getA_i(ptr %0, ptr %1, ptr %2) {\n"
                        "entry:\n"
                        "  %3 = alloca ptr, align 8\n"
                        "  %4 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %3, align 8\n"
                        "  store ptr %1, ptr %4, align 8\n"
                        "  %5 = load ptr, ptr %4, align 8\n"
                        "  %6 = getelementptr inbounds %TestClass, ptr %5, i32 0, i32 1\n"
                        "  %7 = load i32, ptr %6, align 4\n"
                        "  store i32 %7, ptr %2, align 4\n"
                        "  ret void\n"
                        "}\n"
                        "\n"
                        "define void @TestClass_F9TestClass(ptr %0, ptr %1) {\n"
                        "entry:\n"
                        "  %2 = alloca ptr, align 8\n"
                        "  %3 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %2, align 8\n"
                        "  store ptr %1, ptr %3, align 8\n"
                        "  ret void\n"
                        "}\n"
                        "\n"
                        "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca ptr, align 8\n"
                        "  %3 = alloca i32, align 4\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %4 = load ptr, ptr %1, align 8\n"
                        "  %5 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%TestClass, ptr null, i32 1) to i64))\n"
                        "  %6 = call ptr @TestClass.init_ctor(ptr %5)\n"
                        "  call void @TestClass_F9TestClass(ptr %4, ptr %6)\n"
                        "  store ptr %6, ptr %2, align 8\n"
                        "  store i32 0, ptr %3, align 4\n"
                        "  %7 = load ptr, ptr %2, align 8\n"
                        "  %8 = getelementptr inbounds nuw %TestClass, ptr %7, i32 0, i32 0\n"
                        "  %9 = load ptr, ptr %8, align 8\n"
                        "  %10 = getelementptr ptr, ptr %9, i64 1\n"
                        "  %11 = load ptr, ptr %10, align 8\n"
                        "  call void %11(ptr %4, ptr %7, ptr %3)\n"
                        "  call void @free(ptr %7)\n"
                        "  ret void\n"
                        "}\n"
                        "\n"
                        "declare ptr @malloc(i64)\n"
                        "\n"
                        "declare void @free(ptr)\n");
    }

		TEST_F(CodeGenTest, CGClassSetterMethod) {
        /**
         * Fly code:
         * class TestClass {
         *   int a
         *   setA(const int value) {
         *     this.a = value
         *   }
         * }
         * void func() {
         *   TestClass test = new TestClass()
         *   test.setA(2)
         *   delete test
         * }
         */
        ASTModule *Module = CreateModule();

        // TestClass {
        //   int a
        //   int setA(int a) { this.a = a }
        // }
        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass", TopModifiers,
                                                  SuperClasses);

        // int a
        ASTVar *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, TestClass, IntTypeRef, "a", TopModifiers);

        // int setA(int a) { this.a = a }
    	llvm::SmallVector<ASTParam *, 8> setAParams;
    	llvm::SmallVector<ASTModifier *, 8> ConstModifiers;
    	ConstModifiers.push_back(ASTBuilder::CreateModifier(SourceLoc, ASTModifierKind::MOD_CONSTANT));
    	ASTParam * aParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", ConstModifiers);
    	setAParams.push_back(aParam);
    	ASTBlockStmt *MethodBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	const SourceLocation &Loc = SourceLoc;
    	ASTMember * this_a = ASTBuilder::CreateMember(SourceLoc, aAttribute->getName(), ASTBuilder::CreateIdentifier(Loc, "this"));
    	ASTExprStmt * setAttributeAStmt = ASTBuilder::CreateExprStmt(MethodBody, SourceLoc);
    	ASTIdentifier *aParamIdent = ASTBuilder::CreateIdentifier(aParam);
    	ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, this_a, aParamIdent);
    	setAttributeAStmt->setExpr(AssignExpr);
        ASTFunction *setAMethod = ASTBuilder::CreateClassMethod(SourceLoc, TestClass,
                                                               "setA", TopModifiers, setAParams, MethodBody);

        // int func() {
        //  TestClass test = new TestClass()
        //  test.setA(1)
        //  delete test
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // TestClass test = new TestClass()
        ASTType *TestClassType = CreateType(TestClass);
        ASTLocalVar *TestVar = ASTBuilder::CreateLocalVar(SourceLoc, TestClassType, "test", EmptyModifiers);
        ASTDeclStmt *TestDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TestVar);
        ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);
        ASTIdentifier *testIdent = ASTBuilder::CreateIdentifier(TestVar);
        ASTBinary *AssignExpr2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, testIdent, ConstructorCall);
        TestDeclStmt->setExpr(AssignExpr2);

        // test.setA(1)
    	ASTNumberValue * IntValue = ASTBuilder::CreateNumberValue(SourceLoc, "1");
    	llvm::SmallVector<ASTExpr *, 8> setAArgs;
    	setAArgs.push_back(IntValue);
    	ASTCall *Call = ASTBuilder::CreateCall(SourceLoc, setAMethod->getName(), setAArgs, ASTCallKind::CALL_DIRECT, ASTBuilder::CreateIdentifier(TestVar));
    	ASTExprStmt * ExprStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
    	ExprStmt->setExpr(Call);

        // delete test
        ASTBuilder::CreateDeleteStmt(Body, SourceLoc, ASTBuilder::CreateIdentifier(TestVar));

		// Generate Code
		Generate();
		llvm::Module * M = getModules()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n"
                        "%error = type { i32, ptr, ptr }\n"
                        "%TestClass = type { ptr, i32 }\n"
                        "\n"
                        "@error = external constant %error\n"
                        "@vtable.TestClass = constant [3 x ptr] [ptr null, ptr @TestClass_F4setA_i, ptr @TestClass_F9TestClass]\n"
                        "\n"
                        "define linkonce_odr ptr @TestClass.init_ctor(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %2 = load ptr, ptr %1, align 8\n"
                        "  %3 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 0\n"
                        "  store ptr @vtable.TestClass, ptr %3, align 8\n"
                        "  %4 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 1\n"
                        "  store i32 0, ptr %4, align 4\n"
                        "  ret ptr %2\n"
                        "}\n"
                        "\n"
                        "define void @TestClass_F4setA_i(ptr %0, ptr %1, ptr readonly %2) {\n"
                        "entry:\n"
                        "  %3 = alloca ptr, align 8\n"
                        "  %4 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %3, align 8\n"
                        "  store ptr %1, ptr %4, align 8\n"
                        "  %5 = load ptr, ptr %4, align 8\n"
                        "  %6 = getelementptr inbounds %TestClass, ptr %5, i32 0, i32 1\n"
                        "  %7 = load i32, ptr %2, align 4\n"
                        "  store i32 %7, ptr %6, align 4\n"
                        "  ret void\n"
                        "}\n"
                        "\n"
                        "define void @TestClass_F9TestClass(ptr %0, ptr %1) {\n"
                        "entry:\n"
                        "  %2 = alloca ptr, align 8\n"
                        "  %3 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %2, align 8\n"
                        "  store ptr %1, ptr %3, align 8\n"
                        "  ret void\n"
                        "}\n"
                        "\n"
                        "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %3 = load ptr, ptr %1, align 8\n"
                        "  %4 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%TestClass, ptr null, i32 1) to i64))\n"
                        "  %5 = call ptr @TestClass.init_ctor(ptr %4)\n"
                        "  call void @TestClass_F9TestClass(ptr %3, ptr %5)\n"
                        "  store ptr %5, ptr %2, align 8\n"
                        "  %6 = load ptr, ptr %2, align 8\n"
                        "  %7 = alloca i32, align 4\n"
                        "  store i32 1, ptr %7, align 4\n"
                        "  %8 = getelementptr inbounds nuw %TestClass, ptr %6, i32 0, i32 0\n"
                        "  %9 = load ptr, ptr %8, align 8\n"
                        "  %10 = getelementptr ptr, ptr %9, i64 1\n"
                        "  %11 = load ptr, ptr %10, align 8\n"
                        "  call void %11(ptr %3, ptr %6, ptr %7)\n"
                        "  call void @free(ptr %6)\n"
                        "  ret void\n"
                        "}\n"
                        "\n"
                        "declare ptr @malloc(i64)\n"
                        "\n"
                        "declare void @free(ptr)\n");
    }

	TEST_F(CodeGenTest, CGClassStaticAttributes) {
        /**
         * Fly code:
         * class TestClass {
         *   static int a
         * }
         * void func() {
         *   TestClass.a = 2
         *   int x = TestClass.a
         * }
         */
        ASTModule *Module = CreateModule();

        // TestClass {
        //   static int a
        // }
        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass", TopModifiers,
                                                  SuperClasses);

        // int a
    	llvm::SmallVector<ASTModifier *, 8> Modifiers;
    	Modifiers.push_back(ASTBuilder::CreateModifier(SourceLoc, ASTModifierKind::MOD_STATIC));
        ASTAttribute *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, TestClass, IntTypeRef, "a", Modifiers);

        // int func() {
        //  TestClass.a = 2
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        //  TestClass.a = 2
        ASTExprStmt * attrStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTValue *value2Expr = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        ASTMember *attrMember = ASTBuilder::CreateMember(SourceLoc, "a", ASTBuilder::CreateIdentifier(SourceLoc, "TestClass"));
        ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, attrMember, value2Expr);
        attrStmt->setExpr(AssignExpr);

        // int x = TestClass.a
        ASTLocalVar *xVar = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "x", EmptyModifiers);
        ASTDeclStmt *xDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, xVar);
        ASTIdentifier *xIdent = ASTBuilder::CreateIdentifier(xVar);
        ASTMember *attrRead = ASTBuilder::CreateMember(SourceLoc, "a", ASTBuilder::CreateIdentifier(SourceLoc, "TestClass"));
        ASTBinary *xAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, xIdent, attrRead);
        xDeclStmt->setExpr(xAssignExpr);

		// Generate Code
		Generate();
		llvm::Module * M = getModules()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n"
                          "%error = type { i32, ptr, ptr }\n"
                          "%TestClass = type { ptr, i32 }\n"
                          "\n"
                          "@error = external constant %error\n"
                          "@vtable.TestClass = constant [2 x ptr] [ptr null, ptr @TestClass_F9TestClass]\n"
                          "@0 = external global i32\n"
                          "\n"
                          "define linkonce_odr ptr @TestClass.init_ctor(ptr %0) {\n"
                          "entry:\n"
                          "  %1 = alloca ptr, align 8\n"
                          "  store ptr %0, ptr %1, align 8\n"
                          "  %2 = load ptr, ptr %1, align 8\n"
                          "  %3 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 0\n"
                          "  store ptr @vtable.TestClass, ptr %3, align 8\n"
                          "  ret ptr %2\n"
                          "}\n"
                          "\n"
                          "define void @TestClass_F9TestClass(ptr %0, ptr %1) {\n"
                          "entry:\n"
                          "  %2 = alloca ptr, align 8\n"
                          "  %3 = alloca ptr, align 8\n"
                          "  store ptr %0, ptr %2, align 8\n"
                          "  store ptr %1, ptr %3, align 8\n"
                          "  ret void\n"
                          "}\n"
                          "\n"
                          "define void @_F4func(ptr %0) {\n"
                          "entry:\n"
                          "  %1 = alloca ptr, align 8\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store ptr %0, ptr %1, align 8\n"
                          // TestClass.a = 2
                          "  store i32 2, ptr @0, align 4\n"
                          // int x = TestClass.a
                          "  %3 = load i32, ptr @0, align 4\n"
                          "  store i32 %3, ptr %2, align 4\n"
                          "  ret void\n"
                          "}\n"
        );
    }

	TEST_F(CodeGenTest, CGClassStaticMethods) {
        /**
         * Fly code:
         * class TestClass {
         *   static loadA(int a) {
         *     a = 2
         *   }
         * }
         * void func() {
         *   int x
         *   TestClass.loadA(x)
         * }
         */
        ASTModule *Module = CreateModule();

        // class TestClass { static loadA(int a) { a = 2 } }
        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
                                                  TopModifiers, SuperClasses);

        // static loadA(int a) { a = 2 }
        llvm::SmallVector<ASTModifier *, 8> Modifiers;
        Modifiers.push_back(ASTBuilder::CreateModifier(SourceLoc, ASTModifierKind::MOD_STATIC));
        llvm::SmallVector<ASTParam *, 8> loadAParams;
        ASTParam *aParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        loadAParams.push_back(aParam);
        ASTBlockStmt *loadABody = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *loadAMethod = ASTBuilder::CreateClassMethod(SourceLoc, TestClass,
                                                                  "loadA", Modifiers, loadAParams, loadABody);

        // a = 2
        ASTExprStmt *assignStmt = ASTBuilder::CreateExprStmt(loadABody, SourceLoc);
        ASTValue *value2 = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        ASTIdentifier *aIdent = ASTBuilder::CreateIdentifier(aParam);
        ASTBinary *assignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, aIdent, value2);
        assignStmt->setExpr(assignExpr);

        // void func() { int x; TestClass.loadA(x) }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // int x
        ASTLocalVar *xVar = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "x", EmptyModifiers);
        ASTBuilder::CreateDeclStmt(Body, SourceLoc, xVar);

        // TestClass.loadA(x)
        llvm::SmallVector<ASTExpr *, 8> loadAArgs;
        loadAArgs.push_back(ASTBuilder::CreateIdentifier(xVar));
        ASTIdentifier *TestClassIdent = ASTBuilder::CreateIdentifier(SourceLoc, TestClass->getName());
        ASTCall *loadACall = ASTBuilder::CreateCall(SourceLoc, loadAMethod->getName(), loadAArgs,
                                                     ASTCallKind::CALL_DIRECT, TestClassIdent);
        ASTExprStmt *callStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        callStmt->setExpr(loadACall);

        // Generate Code
        Generate();
        llvm::Module * M = getModules()[0];
        std::string output = getOutput(M);

        EXPECT_EQ(output, "\n"
                          "%error = type { i32, ptr, ptr }\n"
                          "%TestClass = type { ptr }\n"
                          "\n"
                          "@error = external constant %error\n"
                          "@vtable.TestClass = constant [3 x ptr] [ptr null, ptr @TestClass_F5loadA_i, ptr @TestClass_F9TestClass]\n"
                          "\n"
                          "define linkonce_odr ptr @TestClass.init_ctor(ptr %0) {\n"
                          "entry:\n"
                          "  %1 = alloca ptr, align 8\n"
                          "  store ptr %0, ptr %1, align 8\n"
                          "  %2 = load ptr, ptr %1, align 8\n"
                          "  %3 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 0\n"
                          "  store ptr @vtable.TestClass, ptr %3, align 8\n"
                          "  ret ptr %2\n"
                          "}\n"
                          "\n"
                          "define void @TestClass_F5loadA_i(ptr %0, ptr %1) {\n"
                          "entry:\n"
                          "  %2 = alloca ptr, align 8\n"
                          "  store ptr %0, ptr %2, align 8\n"
                          "  store i32 2, ptr %1, align 4\n"
                          "  ret void\n"
                          "}\n"
                          "\n"
                          "define void @TestClass_F9TestClass(ptr %0, ptr %1) {\n"
                          "entry:\n"
                          "  %2 = alloca ptr, align 8\n"
                          "  %3 = alloca ptr, align 8\n"
                          "  store ptr %0, ptr %2, align 8\n"
                          "  store ptr %1, ptr %3, align 8\n"
                          "  ret void\n"
                          "}\n"
                          "\n"
                          "define void @_F4func(ptr %0) {\n"
                          "entry:\n"
                          "  %1 = alloca ptr, align 8\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store ptr %0, ptr %1, align 8\n"
                          "  store i32 0, ptr %2, align 4\n"
                          "  %3 = load ptr, ptr %1, align 8\n"
                          "  call void @TestClass_F5loadA_i(ptr %3, ptr %2)\n"
                          "  ret void\n"
                          "}\n"
        );
    }

	TEST_F(CodeGenTest, CGClassExtendsClass) {
        /**
         * Fly code:
         * class BaseClass {
         *   int b
         *   void do() {
         *		b = 1
         *   }
         * }
         * class TestClass : BaseClass {
         *   void undo() {
         *   }
         * }
         * void func() {
         *   BaseClass a = new TestClass()
         *   a.do()
         *   delete a
         * }
         */
    	ASTModule *Module = CreateModule();

    	// BaseClass
    	llvm::SmallVector<ASTType *, 4> BaseSuperClasses;
    	ASTClass *BaseClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "BaseClass",
			TopModifiers, BaseSuperClasses);
    	// int b
    	ASTVar *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, BaseClass, IntTypeRef, "b", TopModifiers);
    	// do()
    	ASTBlockStmt *DoBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *BaseClass_do = ASTBuilder::CreateClassMethod(SourceLoc, BaseClass, "do", TopModifiers, Params, DoBody);

    	// TestClass
    	llvm::SmallVector<ASTType *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(CreateType(BaseClass));
    	ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopModifiers, TestSuperClasses);
    	// undo()
    	ASTBlockStmt *UndoBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *TestClass_undo = ASTBuilder::CreateClassMethod(SourceLoc, TestClass, "undo", TopModifiers, Params, UndoBody);

    	// func() {
    	//   BaseClass a = new TestClass()
    	//   a.do()
    	//   delete a
    	// }
    	ASTBlockStmt *MainBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);

    	ASTType *Base = CreateType(BaseClass);
    	ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(SourceLoc, Base, "a", EmptyModifiers);
    	ASTDeclStmt *aDeclStmt = ASTBuilder::CreateDeclStmt(MainBody, SourceLoc, aVar);
    	ASTIdentifier *aIdent = ASTBuilder::CreateIdentifier(aVar);
    	ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, aIdent, ConstructorCall);
    	aDeclStmt->setExpr(AssignExpr);

    	// a.do()
    	llvm::SmallVector<ASTExpr *, 8> doArgs;
    	ASTCall *doCall = ASTBuilder::CreateCall(SourceLoc, BaseClass_do->getName(), doArgs,
    	                                          ASTCallKind::CALL_DIRECT, ASTBuilder::CreateIdentifier(aVar));
    	ASTExprStmt *doStmt = ASTBuilder::CreateExprStmt(MainBody, SourceLoc);
    	doStmt->setExpr(doCall);

    	// delete a
    	ASTBuilder::CreateDeleteStmt(MainBody, SourceLoc, ASTBuilder::CreateIdentifier(aVar));

    	ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, MainBody);

    	// Generate Code
    	Generate();
		llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M);

	    EXPECT_EQ(output, "\n"
					      "%error = type { i32, ptr, ptr }\n"
					      "%BaseClass = type { ptr, i32 }\n"
					      "%TestClass = type { ptr, %BaseClass }\n"
					      "\n"
					      "@error = external constant %error\n"
					      "@vtable.BaseClass = constant [3 x ptr] [ptr null, ptr @BaseClass_F2do, ptr @BaseClass_F9BaseClass]\n"
					      "@vtable.TestClass = constant [3 x ptr] [ptr null, ptr @TestClass_F4undo, ptr @TestClass_F9TestClass]\n"
					      "\n"
					      "define linkonce_odr ptr @BaseClass.init_ctor(ptr %0) {\n"
					      "entry:\n"
					      "  %1 = alloca ptr, align 8\n"
					      "  store ptr %0, ptr %1, align 8\n"
					      "  %2 = load ptr, ptr %1, align 8\n"
					      "  %3 = getelementptr inbounds %BaseClass, ptr %2, i32 0, i32 0\n"
					      "  store ptr @vtable.BaseClass, ptr %3, align 8\n"
					      "  %4 = getelementptr inbounds %BaseClass, ptr %2, i32 0, i32 1\n"
					      "  store i32 0, ptr %4, align 4\n"
					      "  ret ptr %2\n"
					      "}\n"
					      "\n"
					      "define void @BaseClass_F2do(ptr %0, ptr %1) {\n"
					      "entry:\n"
					      "  %2 = alloca ptr, align 8\n"
					      "  %3 = alloca ptr, align 8\n"
					      "  store ptr %0, ptr %2, align 8\n"
					      "  store ptr %1, ptr %3, align 8\n"
					      "  ret void\n"
					      "}\n"
					      "\n"
					      "define void @BaseClass_F9BaseClass(ptr %0, ptr %1) {\n"
					      "entry:\n"
					      "  %2 = alloca ptr, align 8\n"
					      "  %3 = alloca ptr, align 8\n"
					      "  store ptr %0, ptr %2, align 8\n"
					      "  store ptr %1, ptr %3, align 8\n"
					      "  ret void\n"
					      "}\n"
					      "\n"
					      "define linkonce_odr ptr @TestClass.init_ctor(ptr %0) {\n"
					      "entry:\n"
					      "  %1 = alloca ptr, align 8\n"
					      "  store ptr %0, ptr %1, align 8\n"
					      "  %2 = load ptr, ptr %1, align 8\n"
					      "  %3 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 0\n"
					      "  store ptr @vtable.TestClass, ptr %3, align 8\n"
					      "  %4 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 1\n"
					      "  %5 = call ptr @BaseClass.init_ctor(ptr %4)\n"
					      "  ret ptr %2\n"
					      "}\n"
					      "\n"
					      "define void @TestClass_F4undo(ptr %0, ptr %1) {\n"
					      "entry:\n"
					      "  %2 = alloca ptr, align 8\n"
					      "  %3 = alloca ptr, align 8\n"
					      "  store ptr %0, ptr %2, align 8\n"
					      "  store ptr %1, ptr %3, align 8\n"
					      "  ret void\n"
					      "}\n"
					      "\n"
					      "define void @TestClass_F9TestClass(ptr %0, ptr %1) {\n"
					      "entry:\n"
					      "  %2 = alloca ptr, align 8\n"
					      "  %3 = alloca ptr, align 8\n"
					      "  store ptr %0, ptr %2, align 8\n"
					      "  store ptr %1, ptr %3, align 8\n"
					      "  ret void\n"
					      "}\n"
					      "\n"
					      "define void @_F4func(ptr %0) {\n"
					      "entry:\n"
					      "  %1 = alloca ptr, align 8\n"
					      "  %2 = alloca ptr, align 8\n"
					      "  store ptr %0, ptr %1, align 8\n"
					      "  %3 = load ptr, ptr %1, align 8\n"
					      // BaseClass a = new TestClass()  — upcast adjusts the TestClass pointer to its
					      // BaseClass subobject (index 1) so `a` is a valid BaseClass* (bug #8 fix).
					      "  %4 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%TestClass, ptr null, i32 1) to i64))\n"
					      "  %5 = call ptr @TestClass.init_ctor(ptr %4)\n"
					      "  call void @TestClass_F9TestClass(ptr %3, ptr %5)\n"
					      "  %6 = getelementptr inbounds %TestClass, ptr %5, i32 0, i32 1\n"
					      "  store ptr %6, ptr %2, align 8\n"
					      // a.do()  — virtual dispatch through the BaseClass-subobject vtable slot 1
					      "  %7 = load ptr, ptr %2, align 8\n"
					      "  %8 = getelementptr inbounds nuw %BaseClass, ptr %7, i32 0, i32 0\n"
					      "  %9 = load ptr, ptr %8, align 8\n"
					      "  %10 = getelementptr ptr, ptr %9, i64 1\n"
					      "  %11 = load ptr, ptr %10, align 8\n"
					      "  call void %11(ptr %3, ptr %7)\n"
					      // delete a — frees the BaseClass-subobject pointer. Raw `delete` on an upcast
					      // variable is a known limitation; the idiomatic `.free()` method re-adjusts
					      // `this` to the complete object through the per-base vtable thunk.
					      "  call void @free(ptr %7)\n"
					      "  ret void\n"
					      "}\n"
					      "\n"
					      "declare ptr @malloc(i64)\n"
					      "\n"
					      "declare void @free(ptr)\n"
					      );
    }

	TEST_F(CodeGenTest, CGClassBaseFieldAccess) {
        /**
         * Fly code (feature 5b: BaseClass.field from derived method):
         * class BaseClass {
         *   int b
         * }
         * class TestClass : BaseClass {
         *   void setBase() {
         *     BaseClass.b = 5
         *   }
         * }
         */
        ASTModule *Module = CreateModule();

        // BaseClass { int b }
        llvm::SmallVector<ASTType *, 4> BaseSuperClasses;
        ASTClass *BaseClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "BaseClass",
                                                      TopModifiers, BaseSuperClasses);
        ASTAttribute *bAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, BaseClass, IntTypeRef, "b", TopModifiers);

        // TestClass : BaseClass { void setBase() { BaseClass.b = 5 } }
        llvm::SmallVector<ASTType *, 4> TestSuperClasses;
        TestSuperClasses.push_back(CreateType(BaseClass));
        ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
                                                       TopModifiers, TestSuperClasses);

        // void setBase() { BaseClass.b = 5 }
        ASTBlockStmt *SetBaseBody = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *setBaseMethod = ASTBuilder::CreateClassMethod(SourceLoc, TestClass, "setBase",
                                                                    TopModifiers, Params, SetBaseBody);
        // BaseClass.b = 5   (feature 5b: access base field via class name)
        ASTMember *baseb = ASTBuilder::CreateMember(SourceLoc, bAttribute->getName(),
                                                    ASTBuilder::CreateIdentifier(SourceLoc, "BaseClass"));
        ASTExprStmt *setBaseStmt = ASTBuilder::CreateExprStmt(SetBaseBody, SourceLoc);
        ASTValue *value5 = ASTBuilder::CreateNumberValue(SourceLoc, "5");
        ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, baseb, value5);
        setBaseStmt->setExpr(AssignExpr);

        // Generate Code
        Generate();
        llvm::Module *M = getModules()[0];
        std::string output = getOutput(M);

        EXPECT_EQ(output, "\n"
                        "%error = type { i32, ptr, ptr }\n"
                        "%BaseClass = type { ptr, i32 }\n"
                        "%TestClass = type { ptr, %BaseClass }\n"
                        "\n"
                        "@error = external constant %error\n"
                        "@vtable.BaseClass = constant [2 x ptr] [ptr null, ptr @BaseClass_F9BaseClass]\n"
                        "@vtable.TestClass = constant [3 x ptr] [ptr null, ptr @TestClass_F7setBase, ptr @TestClass_F9TestClass]\n"
                        "\n"
                        "define linkonce_odr ptr @BaseClass.init_ctor(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %2 = load ptr, ptr %1, align 8\n"
                        "  %3 = getelementptr inbounds %BaseClass, ptr %2, i32 0, i32 0\n"
                        "  store ptr @vtable.BaseClass, ptr %3, align 8\n"
                        "  %4 = getelementptr inbounds %BaseClass, ptr %2, i32 0, i32 1\n"
                        "  store i32 0, ptr %4, align 4\n"
                        "  ret ptr %2\n"
                        "}\n"
                        "\n"
                        "define void @BaseClass_F9BaseClass(ptr %0, ptr %1) {\n"
                        "entry:\n"
                        "  %2 = alloca ptr, align 8\n"
                        "  %3 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %2, align 8\n"
                        "  store ptr %1, ptr %3, align 8\n"
                        "  ret void\n"
                        "}\n"
                        "\n"
                        "define linkonce_odr ptr @TestClass.init_ctor(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %2 = load ptr, ptr %1, align 8\n"
                        "  %3 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 0\n"
                        "  store ptr @vtable.TestClass, ptr %3, align 8\n"
                        "  %4 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 1\n"
                        "  %5 = call ptr @BaseClass.init_ctor(ptr %4)\n"
                        "  ret ptr %2\n"
                        "}\n"
                        "\n"
                        "define void @TestClass_F7setBase(ptr %0, ptr %1) {\n"
                        "entry:\n"
                        "  %2 = alloca ptr, align 8\n"
                        "  %3 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %2, align 8\n"
                        "  store ptr %1, ptr %3, align 8\n"
                        "  %4 = load ptr, ptr %3, align 8\n"
                        // Feature 5b: two-level GEP — first to embedded BaseClass (index 1), then to field b (index 1)
                        "  %5 = getelementptr inbounds %TestClass, ptr %4, i32 0, i32 1\n"
                        "  %6 = getelementptr inbounds %BaseClass, ptr %5, i32 0, i32 1\n"
                        "  store i32 5, ptr %6, align 4\n"
                        "  ret void\n"
                        "}\n"
                        "\n"
                        "define void @TestClass_F9TestClass(ptr %0, ptr %1) {\n"
                        "entry:\n"
                        "  %2 = alloca ptr, align 8\n"
                        "  %3 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %2, align 8\n"
                        "  store ptr %1, ptr %3, align 8\n"
                        "  ret void\n"
                        "}\n");
    }

	TEST_F(CodeGenTest, CGClassExtendsStruct) {
		/**
		 * Fly code:
		 * struct BaseStruct {
		 *   int a = 3
		 * }
		 * class TestClass : BaseStruct {
		 *   void do() {
		 *		this.a = 1
		 *   }
		 * }
		 */
		ASTModule *Module = CreateModule();

		// struct BaseStruct { int a = 3 }
		llvm::SmallVector<ASTType *, 4> BaseSuperClasses;
		ASTClass *BaseStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct",
		                                               TopModifiers, BaseSuperClasses);
		ASTAttribute *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, BaseStruct, IntTypeRef, "a", TopModifiers);
		aAttribute->setExpr(ASTBuilder::CreateNumberValue(SourceLoc, "3"));

		// class TestClass : BaseStruct { void do() { this.a = 1 } }
		llvm::SmallVector<ASTType *, 4> TestSuperClasses;
		TestSuperClasses.push_back(CreateType(BaseStruct));
		ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
		                                              TopModifiers, TestSuperClasses);

		// void do() { this.a = 1 }
		ASTBlockStmt *DoBody = ASTBuilder::CreateBlockStmt(SourceLoc);
		ASTMember *this_a = ASTBuilder::CreateMember(SourceLoc, aAttribute->getName(),
		                                              ASTBuilder::CreateIdentifier(SourceLoc, "this"));
		ASTExprStmt *doStmt = ASTBuilder::CreateExprStmt(DoBody, SourceLoc);
		ASTValue *value1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
		ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
		                                                  this_a, value1);
		doStmt->setExpr(AssignExpr);
		ASTBuilder::CreateClassMethod(SourceLoc, TestClass, "do", TopModifiers, Params, DoBody);

		// Generate Code
		Generate();
		llvm::Module *M = getModules()[0];
		std::string output = getOutput(M);

		EXPECT_EQ(output, "\n"
		                  "%error = type { i32, ptr, ptr }\n"
		                  "%BaseStruct = type { i32 }\n"
		                  "%TestClass = type { ptr, %BaseStruct }\n"
		                  "\n"
		                  "@error = external constant %error\n"
		                  "@vtable.TestClass = constant [3 x ptr] [ptr null, ptr @TestClass_F2do, ptr @TestClass_F9TestClass]\n"
		                  "\n"
		                  "define linkonce_odr ptr @BaseStruct.init_ctor(ptr %0) {\n"
		                  "entry:\n"
		                  "  %1 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %1, align 8\n"
		                  "  %2 = load ptr, ptr %1, align 8\n"
		                  "  %3 = getelementptr inbounds %BaseStruct, ptr %2, i32 0, i32 0\n"
		                  "  store i32 3, ptr %3, align 4\n"
		                  "  ret ptr %2\n"
		                  "}\n"
		                  "\n"
		                  "define linkonce_odr ptr @TestClass.init_ctor(ptr %0) {\n"
		                  "entry:\n"
		                  "  %1 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %1, align 8\n"
		                  "  %2 = load ptr, ptr %1, align 8\n"
		                  "  %3 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 0\n"
		                  "  store ptr @vtable.TestClass, ptr %3, align 8\n"
		                  "  %4 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 1\n"
		                  "  %5 = call ptr @BaseStruct.init_ctor(ptr %4)\n"
		                  "  ret ptr %2\n"
		                  "}\n"
		                  "\n"
		                  "define void @TestClass_F2do(ptr %0, ptr %1) {\n"
		                  "entry:\n"
		                  "  %2 = alloca ptr, align 8\n"
		                  "  %3 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %2, align 8\n"
		                  "  store ptr %1, ptr %3, align 8\n"
		                  "  %4 = load ptr, ptr %3, align 8\n"
		                  // this.a: two-level GEP — TestClass→BaseStruct(index 1)→a(index 0)
		                  "  %5 = getelementptr inbounds %TestClass, ptr %4, i32 0, i32 1\n"
		                  "  %6 = getelementptr inbounds %BaseStruct, ptr %5, i32 0, i32 0\n"
		                  "  store i32 1, ptr %6, align 4\n"
		                  "  ret void\n"
		                  "}\n"
		                  "\n"
		                  "define void @TestClass_F9TestClass(ptr %0, ptr %1) {\n"
		                  "entry:\n"
		                  "  %2 = alloca ptr, align 8\n"
		                  "  %3 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %2, align 8\n"
		                  "  store ptr %1, ptr %3, align 8\n"
		                  "  ret void\n"
		                  "}\n");
	}

	TEST_F(CodeGenTest, CGClassExtendsInterface) {
		/**
		 * Fly code:
		 *  interface IFirst {
		 *		void first()
		 *  }
		 *  interface ISecond {
		 *		void second()
		 *  }
		 *  class TestClass : IFirst, ISecond {
		 *		void first() {}
		 *		void second() {}
		 *  }
		 *  void func() {
		 *		TestClass t = new TestClass()
		 *		t.first()
		 *		t.second()
		 *		delete t
		 *  }
		 */
		ASTModule *Module = CreateModule();

		// interface IFirst { void first() }
		llvm::SmallVector<ASTType *, 4> IFirstSuperClasses;
		ASTClass *IFirst = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::INTERFACE, "IFirst",
		                                           TopModifiers, IFirstSuperClasses);
		ASTBlockStmt *IFirstFirstBody = ASTBuilder::CreateBlockStmt(SourceLoc);
		ASTBuilder::CreateClassMethod(SourceLoc, IFirst, "first", TopModifiers, Params, IFirstFirstBody);

		// interface ISecond { void second() }
		llvm::SmallVector<ASTType *, 4> ISecondSuperClasses;
		ASTClass *ISecond = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::INTERFACE, "ISecond",
		                                            TopModifiers, ISecondSuperClasses);
		ASTBlockStmt *ISecondSecondBody = ASTBuilder::CreateBlockStmt(SourceLoc);
		ASTBuilder::CreateClassMethod(SourceLoc, ISecond, "second", TopModifiers, Params, ISecondSecondBody);

		// class TestClass : IFirst, ISecond { void first() {} void second() {} }
		llvm::SmallVector<ASTType *, 4> TestSuperClasses;
		TestSuperClasses.push_back(CreateType(IFirst));
		TestSuperClasses.push_back(CreateType(ISecond));
		ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
		                                              TopModifiers, TestSuperClasses);

		ASTBlockStmt *FirstBody = ASTBuilder::CreateBlockStmt(SourceLoc);
		ASTBuilder::CreateClassMethod(SourceLoc, TestClass, "first", TopModifiers, Params, FirstBody);

		ASTBlockStmt *SecondBody = ASTBuilder::CreateBlockStmt(SourceLoc);
		ASTBuilder::CreateClassMethod(SourceLoc, TestClass, "second", TopModifiers, Params, SecondBody);

		// void func() { TestClass t = new TestClass(); t.first(); t.second(); delete t }
		ASTBlockStmt *FuncBody = ASTBuilder::CreateBlockStmt(SourceLoc);

		// TestClass t = new TestClass()
		ASTType *TestClassType = CreateType(TestClass);
		ASTLocalVar *tVar = ASTBuilder::CreateLocalVar(SourceLoc, TestClassType, "t", EmptyModifiers);
		ASTDeclStmt *tDeclStmt = ASTBuilder::CreateDeclStmt(FuncBody, SourceLoc, tVar);
		ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);
		ASTIdentifier *tIdent = ASTBuilder::CreateIdentifier(tVar);
		ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, tIdent, ConstructorCall);
		tDeclStmt->setExpr(AssignExpr);

		// t.first()
		llvm::SmallVector<ASTExpr *, 8> firstArgs;
		ASTCall *firstCall = ASTBuilder::CreateCall(SourceLoc, "first", firstArgs,
		                                            ASTCallKind::CALL_DIRECT, ASTBuilder::CreateIdentifier(tVar));
		ASTExprStmt *firstStmt = ASTBuilder::CreateExprStmt(FuncBody, SourceLoc);
		firstStmt->setExpr(firstCall);

		// t.second()
		llvm::SmallVector<ASTExpr *, 8> secondArgs;
		ASTCall *secondCall = ASTBuilder::CreateCall(SourceLoc, "second", secondArgs,
		                                             ASTCallKind::CALL_DIRECT, ASTBuilder::CreateIdentifier(tVar));
		ASTExprStmt *secondStmt = ASTBuilder::CreateExprStmt(FuncBody, SourceLoc);
		secondStmt->setExpr(secondCall);

		// delete t
		ASTBuilder::CreateDeleteStmt(FuncBody, SourceLoc, ASTBuilder::CreateIdentifier(tVar));

		ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, FuncBody);

		// Generate Code
		Generate();
		llvm::Module *M = getModules()[0];
		std::string output = getOutput(M);

		EXPECT_EQ(output, "\n"
		                  "%error = type { i32, ptr, ptr }\n"
		                  "%TestClass = type { ptr, %IFirst, %ISecond }\n"
		                  "%IFirst = type { ptr }\n"
		                  "%ISecond = type { ptr }\n"
		                  "\n"
		                  "@error = external constant %error\n"
		                  "@vtable.IFirst = constant [2 x ptr] zeroinitializer\n"
		                  "@vtable.ISecond = constant [2 x ptr] zeroinitializer\n"
		                  "@vtable.TestClass = constant [4 x ptr] [ptr null, ptr @TestClass_F5first, ptr @TestClass_F6second, ptr @TestClass_F9TestClass]\n"
		                  "@vtable.TestClass.IFirst = constant [2 x ptr] [ptr inttoptr (i64 -8 to ptr), ptr @thunk.TestClass.IFirst.first.1]\n"
		                  "@vtable.TestClass.ISecond = constant [2 x ptr] [ptr inttoptr (i64 -16 to ptr), ptr @thunk.TestClass.ISecond.second.1]\n"
		                  "\n"
		                  "define linkonce_odr ptr @TestClass.init_ctor(ptr %0) {\n"
		                  "entry:\n"
		                  "  %1 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %1, align 8\n"
		                  "  %2 = load ptr, ptr %1, align 8\n"
		                  "  %3 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 0\n"
		                  "  store ptr @vtable.TestClass, ptr %3, align 8\n"
		                  "  %4 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 1\n"
		                  "  store ptr @vtable.TestClass.IFirst, ptr %4, align 8\n"
		                  "  %5 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 2\n"
		                  "  store ptr @vtable.TestClass.ISecond, ptr %5, align 8\n"
		                  "  ret ptr %2\n"
		                  "}\n"
		                  "\n"
		                  "define void @TestClass_F5first(ptr %0, ptr %1) {\n"
		                  "entry:\n"
		                  "  %2 = alloca ptr, align 8\n"
		                  "  %3 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %2, align 8\n"
		                  "  store ptr %1, ptr %3, align 8\n"
		                  "  ret void\n"
		                  "}\n"
		                  "\n"
		                  "define void @TestClass_F6second(ptr %0, ptr %1) {\n"
		                  "entry:\n"
		                  "  %2 = alloca ptr, align 8\n"
		                  "  %3 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %2, align 8\n"
		                  "  store ptr %1, ptr %3, align 8\n"
		                  "  ret void\n"
		                  "}\n"
		                  "\n"
		                  "define void @TestClass_F9TestClass(ptr %0, ptr %1) {\n"
		                  "entry:\n"
		                  "  %2 = alloca ptr, align 8\n"
		                  "  %3 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %2, align 8\n"
		                  "  store ptr %1, ptr %3, align 8\n"
		                  "  ret void\n"
		                  "}\n"
		                  "\n"
		                  "define internal void @thunk.TestClass.IFirst.first.1(ptr %0, ptr %1) {\n"
		                  "entry:\n"
		                  "  %2 = ptrtoint ptr %1 to i64\n"
		                  "  %3 = sub i64 %2, 8\n"
		                  "  %4 = inttoptr i64 %3 to ptr\n"
		                  "  call void @TestClass_F5first(ptr %0, ptr %4)\n"
		                  "  ret void\n"
		                  "}\n"
		                  "\n"
		                  "define internal void @thunk.TestClass.ISecond.second.1(ptr %0, ptr %1) {\n"
		                  "entry:\n"
		                  "  %2 = ptrtoint ptr %1 to i64\n"
		                  "  %3 = sub i64 %2, 16\n"
		                  "  %4 = inttoptr i64 %3 to ptr\n"
		                  "  call void @TestClass_F6second(ptr %0, ptr %4)\n"
		                  "  ret void\n"
		                  "}\n"
		                  "\n"
		                  "define void @_F4func(ptr %0) {\n"
		                  "entry:\n"
		                  "  %1 = alloca ptr, align 8\n"
		                  "  %2 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %1, align 8\n"
		                  "  %3 = load ptr, ptr %1, align 8\n"
		                  "  %4 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%TestClass, ptr null, i32 1) to i64))\n"
		                  "  %5 = call ptr @TestClass.init_ctor(ptr %4)\n"
		                  "  call void @TestClass_F9TestClass(ptr %3, ptr %5)\n"
		                  "  store ptr %5, ptr %2, align 8\n"
		                  "  %6 = load ptr, ptr %2, align 8\n"
		                  "  %7 = getelementptr inbounds nuw %TestClass, ptr %6, i32 0, i32 0\n"
		                  "  %8 = load ptr, ptr %7, align 8\n"
		                  "  %9 = getelementptr ptr, ptr %8, i64 1\n"
		                  "  %10 = load ptr, ptr %9, align 8\n"
		                  "  call void %10(ptr %3, ptr %6)\n"
		                  "  %11 = getelementptr inbounds nuw %TestClass, ptr %6, i32 0, i32 0\n"
		                  "  %12 = load ptr, ptr %11, align 8\n"
		                  "  %13 = getelementptr ptr, ptr %12, i64 2\n"
		                  "  %14 = load ptr, ptr %13, align 8\n"
		                  "  call void %14(ptr %3, ptr %6)\n"
		                  "  call void @free(ptr %6)\n"
		                  "  ret void\n"
		                  "}\n"
		                  "\n"
		                  "declare ptr @malloc(i64)\n"
		                  "\n"
		                  "declare void @free(ptr)\n");
	}

	TEST_F(CodeGenTest, CGClassExtendsStructAndInterface) {
		/**
		 * Fly code:
		 * struct BaseStruct {
		 *   int a = 3
		 * }
		 * interface BaseInterface {
		 *   void do()
		 * }
		 * class TestClass : BaseStruct, BaseInterface {
		 *   void do() {
		 *     this.a = 1
		 *   }
		 * }
		 * void func() {
		 *   TestClass test = new TestClass()
		 *   test.do()
		 *   delete test
		 * }
		 */
		ASTModule *Module = CreateModule();

		// struct BaseStruct { int a = 3 }
		llvm::SmallVector<ASTType *, 4> BaseSuperClasses;
		ASTClass *BaseStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct",
		                                               TopModifiers, BaseSuperClasses);
		ASTAttribute *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, BaseStruct, IntTypeRef, "a", TopModifiers);
		aAttribute->setExpr(ASTBuilder::CreateNumberValue(SourceLoc, "3"));

		// interface BaseInterface { void do() }
		llvm::SmallVector<ASTType *, 4> InterfaceSuperClasses;
		ASTClass *BaseInterface = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::INTERFACE, "BaseInterface",
		                                                  TopModifiers, InterfaceSuperClasses);
		ASTBlockStmt *DoAbstractBody = ASTBuilder::CreateBlockStmt(SourceLoc);
		ASTBuilder::CreateClassMethod(SourceLoc, BaseInterface, "do", TopModifiers, Params, DoAbstractBody);

		// class TestClass : BaseStruct, BaseInterface { void do() { this.a = 1 } }
		llvm::SmallVector<ASTType *, 4> TestSuperClasses;
		TestSuperClasses.push_back(CreateType(BaseStruct));
		TestSuperClasses.push_back(CreateType(BaseInterface));
		ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
		                                              TopModifiers, TestSuperClasses);

		ASTBlockStmt *DoBody = ASTBuilder::CreateBlockStmt(SourceLoc);
		ASTMember *this_a = ASTBuilder::CreateMember(SourceLoc, aAttribute->getName(),
		                                              ASTBuilder::CreateIdentifier(SourceLoc, "this"));
		ASTExprStmt *doAssignStmt = ASTBuilder::CreateExprStmt(DoBody, SourceLoc);
		ASTValue *value1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
		ASTBinary *AssignExpr_do = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, this_a, value1);
		doAssignStmt->setExpr(AssignExpr_do);
		ASTBuilder::CreateClassMethod(SourceLoc, TestClass, "do", TopModifiers, Params, DoBody);

		// void func() { TestClass test = new TestClass(); test.do(); delete test }
		ASTBlockStmt *FuncBody = ASTBuilder::CreateBlockStmt(SourceLoc);

		ASTType *TestClassType = CreateType(TestClass);
		ASTLocalVar *testVar = ASTBuilder::CreateLocalVar(SourceLoc, TestClassType, "test", EmptyModifiers);
		ASTDeclStmt *testDeclStmt = ASTBuilder::CreateDeclStmt(FuncBody, SourceLoc, testVar);
		ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);
		ASTIdentifier *testIdent = ASTBuilder::CreateIdentifier(testVar);
		ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, testIdent, ConstructorCall);
		testDeclStmt->setExpr(AssignExpr);

		// test.do()
		llvm::SmallVector<ASTExpr *, 8> doArgs;
		ASTCall *doCall = ASTBuilder::CreateCall(SourceLoc, "do", doArgs,
		                                         ASTCallKind::CALL_DIRECT, ASTBuilder::CreateIdentifier(testVar));
		ASTExprStmt *doCallStmt = ASTBuilder::CreateExprStmt(FuncBody, SourceLoc);
		doCallStmt->setExpr(doCall);

		// delete test
		ASTBuilder::CreateDeleteStmt(FuncBody, SourceLoc, ASTBuilder::CreateIdentifier(testVar));

		ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, FuncBody);

		// Generate Code
		Generate();
		llvm::Module *M = getModules()[0];
		std::string output = getOutput(M);

		EXPECT_EQ(output, "\n"
		                  "%error = type { i32, ptr, ptr }\n"
		                  "%BaseStruct = type { i32 }\n"
		                  "%TestClass = type { ptr, %BaseStruct, %BaseInterface }\n"
		                  "%BaseInterface = type { ptr }\n"
		                  "\n"
		                  "@error = external constant %error\n"
		                  "@vtable.BaseInterface = constant [2 x ptr] zeroinitializer\n"
		                  "@vtable.TestClass = constant [3 x ptr] [ptr null, ptr @TestClass_F2do, ptr @TestClass_F9TestClass]\n"
		                  "@vtable.TestClass.BaseInterface = constant [2 x ptr] [ptr inttoptr (i64 -16 to ptr), ptr @thunk.TestClass.BaseInterface.do.1]\n"
		                  "\n"
		                  "define linkonce_odr ptr @BaseStruct.init_ctor(ptr %0) {\n"
		                  "entry:\n"
		                  "  %1 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %1, align 8\n"
		                  "  %2 = load ptr, ptr %1, align 8\n"
		                  "  %3 = getelementptr inbounds %BaseStruct, ptr %2, i32 0, i32 0\n"
		                  "  store i32 3, ptr %3, align 4\n"
		                  "  ret ptr %2\n"
		                  "}\n"
		                  "\n"
		                  "define linkonce_odr ptr @TestClass.init_ctor(ptr %0) {\n"
		                  "entry:\n"
		                  "  %1 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %1, align 8\n"
		                  "  %2 = load ptr, ptr %1, align 8\n"
		                  "  %3 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 0\n"
		                  "  store ptr @vtable.TestClass, ptr %3, align 8\n"
		                  "  %4 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 1\n"
		                  "  %5 = call ptr @BaseStruct.init_ctor(ptr %4)\n"
		                  "  %6 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 2\n"
		                  "  store ptr @vtable.TestClass.BaseInterface, ptr %6, align 8\n"
		                  "  ret ptr %2\n"
		                  "}\n"
		                  "\n"
		                  "define void @TestClass_F2do(ptr %0, ptr %1) {\n"
		                  "entry:\n"
		                  "  %2 = alloca ptr, align 8\n"
		                  "  %3 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %2, align 8\n"
		                  "  store ptr %1, ptr %3, align 8\n"
		                  "  %4 = load ptr, ptr %3, align 8\n"
		                  // this.a: two-level GEP — TestClass→BaseStruct(index 1)→a(index 0)
		                  "  %5 = getelementptr inbounds %TestClass, ptr %4, i32 0, i32 1\n"
		                  "  %6 = getelementptr inbounds %BaseStruct, ptr %5, i32 0, i32 0\n"
		                  "  store i32 1, ptr %6, align 4\n"
		                  "  ret void\n"
		                  "}\n"
		                  "\n"
		                  "define void @TestClass_F9TestClass(ptr %0, ptr %1) {\n"
		                  "entry:\n"
		                  "  %2 = alloca ptr, align 8\n"
		                  "  %3 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %2, align 8\n"
		                  "  store ptr %1, ptr %3, align 8\n"
		                  "  ret void\n"
		                  "}\n"
		                  "\n"
		                  "define internal void @thunk.TestClass.BaseInterface.do.1(ptr %0, ptr %1) {\n"
		                  "entry:\n"
		                  "  %2 = ptrtoint ptr %1 to i64\n"
		                  "  %3 = sub i64 %2, 16\n"
		                  "  %4 = inttoptr i64 %3 to ptr\n"
		                  "  call void @TestClass_F2do(ptr %0, ptr %4)\n"
		                  "  ret void\n"
		                  "}\n"
		                  "\n"
		                  "define void @_F4func(ptr %0) {\n"
		                  "entry:\n"
		                  "  %1 = alloca ptr, align 8\n"
		                  "  %2 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %1, align 8\n"
		                  "  %3 = load ptr, ptr %1, align 8\n"
		                  "  %4 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%TestClass, ptr null, i32 1) to i64))\n"
		                  "  %5 = call ptr @TestClass.init_ctor(ptr %4)\n"
		                  "  call void @TestClass_F9TestClass(ptr %3, ptr %5)\n"
		                  "  store ptr %5, ptr %2, align 8\n"
		                  "  %6 = load ptr, ptr %2, align 8\n"
		                  "  %7 = getelementptr inbounds nuw %TestClass, ptr %6, i32 0, i32 0\n"
		                  "  %8 = load ptr, ptr %7, align 8\n"
		                  "  %9 = getelementptr ptr, ptr %8, i64 1\n"
		                  "  %10 = load ptr, ptr %9, align 8\n"
		                  "  call void %10(ptr %3, ptr %6)\n"
		                  "  call void @free(ptr %6)\n"
		                  "  ret void\n"
		                  "}\n"
		                  "\n"
		                  "declare ptr @malloc(i64)\n"
		                  "\n"
		                  "declare void @free(ptr)\n");
	}

	TEST_F(CodeGenTest, CGAbstractClass) {
		/**
		* Fly code:
		* abstract class BaseClass {
		*   int b
		*   do()
		*   foo() { int a = 1; this.b = 1 }
		* }
		* */
		ASTModule *Module = CreateModule();

		// abstract class BaseClass { int b; do(); foo() { int a = 1 } }
		llvm::SmallVector<ASTModifier *, 8> AbstractModifiers;
		AbstractModifiers.push_back(ASTBuilder::CreateModifier(SourceLoc, ASTModifierKind::MOD_ABSTRACT));
		llvm::SmallVector<ASTType *, 4> SuperClasses;
		ASTClass *BaseClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "BaseClass",
		                                              AbstractModifiers, SuperClasses);

		// int b
		ASTVar *bAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, BaseClass, IntTypeRef, "b", TopModifiers);

		// do() - abstract method (no body)
		llvm::SmallVector<ASTModifier *, 8> AbstractMethodModifiers;
		AbstractMethodModifiers.push_back(ASTBuilder::CreateModifier(SourceLoc, ASTModifierKind::MOD_ABSTRACT));
		ASTBuilder::CreateClassMethod(SourceLoc, BaseClass, "do", AbstractMethodModifiers, Params, nullptr);

		// foo() { int a = 1; this.b = 1 }
		ASTBlockStmt *FooBody = ASTBuilder::CreateBlockStmt(SourceLoc);
		ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "a", EmptyModifiers);
		ASTDeclStmt *aDeclStmt = ASTBuilder::CreateDeclStmt(FooBody, SourceLoc, aVar);
		ASTValue *value1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
		ASTIdentifier *aIdent = ASTBuilder::CreateIdentifier(aVar);
		ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, aIdent, value1);
		aDeclStmt->setExpr(AssignExpr);
		ASTMember *thisB = ASTBuilder::CreateMember(SourceLoc, bAttribute->getName(),
		                                             ASTBuilder::CreateIdentifier(SourceLoc, "this"));
		ASTExprStmt *bAssignStmt = ASTBuilder::CreateExprStmt(FooBody, SourceLoc);
		ASTValue *value2 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
		ASTBinary *bAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, thisB, value2);
		bAssignStmt->setExpr(bAssignExpr);
		ASTBuilder::CreateClassMethod(SourceLoc, BaseClass, "foo", TopModifiers, Params, FooBody);

		// Generate Code
		Generate();
		llvm::Module *M = getModules()[0];
		std::string output = getOutput(M);

		EXPECT_EQ(output, "\n"
		                  "%error = type { i32, ptr, ptr }\n"
		                  "%BaseClass = type { ptr, i32 }\n"
		                  "\n"
		                  "@error = external constant %error\n"
		                  // abstract `do` → null slot; concrete `foo` and constructor → function ptrs
		                  "@vtable.BaseClass = constant [4 x ptr] [ptr null, ptr null, ptr @BaseClass_F3foo, ptr @BaseClass_F9BaseClass]\n"
		                  "\n"
		                  // No init_ctor for abstract class
		                  "define void @BaseClass_F3foo(ptr %0, ptr %1) {\n"
		                  "entry:\n"
		                  "  %2 = alloca ptr, align 8\n"
		                  "  %3 = alloca ptr, align 8\n"
		                  "  %4 = alloca i32, align 4\n"
		                  "  store ptr %0, ptr %2, align 8\n"
		                  "  store ptr %1, ptr %3, align 8\n"
		                  "  store i32 1, ptr %4, align 4\n"
		                  "  %5 = load ptr, ptr %3, align 8\n"
		                  "  %6 = getelementptr inbounds %BaseClass, ptr %5, i32 0, i32 1\n"
		                  "  store i32 1, ptr %6, align 4\n"
		                  "  ret void\n"
		                  "}\n"
		                  "\n"
		                  "define void @BaseClass_F9BaseClass(ptr %0, ptr %1) {\n"
		                  "entry:\n"
		                  "  %2 = alloca ptr, align 8\n"
		                  "  %3 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %2, align 8\n"
		                  "  store ptr %1, ptr %3, align 8\n"
		                  "  ret void\n"
		                  "}\n");
	}

	TEST_F(CodeGenTest, CGFinalClass) {
		/**
		 * Fly code:
		 * final class FinalClass {
		 *   int b
		 *   foo() { this.b = 1 }
		 *   FinalClass() {}
		 * }
		 * */
		ASTModule *Module = CreateModule();

		llvm::SmallVector<ASTModifier *, 8> FinalModifiers;
		FinalModifiers.push_back(ASTBuilder::CreateModifier(SourceLoc, ASTModifierKind::MOD_FINAL));
		llvm::SmallVector<ASTType *, 4> SuperClasses;
		ASTClass *FinalClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "FinalClass",
		                                               FinalModifiers, SuperClasses);

		// int b
		ASTVar *bAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, FinalClass, IntTypeRef, "b", TopModifiers);

		// foo() { this.b = 1 }
		ASTBlockStmt *FooBody = ASTBuilder::CreateBlockStmt(SourceLoc);
		ASTMember *thisB = ASTBuilder::CreateMember(SourceLoc, bAttribute->getName(),
		                                             ASTBuilder::CreateIdentifier(SourceLoc, "this"));
		ASTExprStmt *bAssignStmt = ASTBuilder::CreateExprStmt(FooBody, SourceLoc);
		ASTValue *value1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
		ASTBinary *bAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, thisB, value1);
		bAssignStmt->setExpr(bAssignExpr);
		ASTBuilder::CreateClassMethod(SourceLoc, FinalClass, "foo", TopModifiers, Params, FooBody);

		// FinalClass() {}
		ASTBlockStmt *CtorBody = ASTBuilder::CreateBlockStmt(SourceLoc);
		ASTBuilder::CreateClassMethod(SourceLoc, FinalClass, "FinalClass", TopModifiers, Params, CtorBody);

		Generate();
		llvm::Module *M = getModules()[0];
		std::string output = getOutput(M);

		EXPECT_EQ(output, "\n"
		                  "%error = type { i32, ptr, ptr }\n"
		                  "%FinalClass = type { ptr, i32 }\n"
		                  "\n"
		                  "@error = external constant %error\n"
		                  "@vtable.FinalClass = constant [3 x ptr] [ptr null, ptr @FinalClass_F3foo, ptr @FinalClass_F10FinalClass]\n"
		                  "\n"
		                  "define linkonce_odr ptr @FinalClass.init_ctor(ptr %0) {\n"
		                  "entry:\n"
		                  "  %1 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %1, align 8\n"
		                  "  %2 = load ptr, ptr %1, align 8\n"
		                  "  %3 = getelementptr inbounds %FinalClass, ptr %2, i32 0, i32 0\n"
		                  "  store ptr @vtable.FinalClass, ptr %3, align 8\n"
		                  "  %4 = getelementptr inbounds %FinalClass, ptr %2, i32 0, i32 1\n"
		                  "  store i32 0, ptr %4, align 4\n"
		                  "  ret ptr %2\n"
		                  "}\n"
		                  "\n"
		                  "define void @FinalClass_F3foo(ptr %0, ptr %1) {\n"
		                  "entry:\n"
		                  "  %2 = alloca ptr, align 8\n"
		                  "  %3 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %2, align 8\n"
		                  "  store ptr %1, ptr %3, align 8\n"
		                  "  %4 = load ptr, ptr %3, align 8\n"
		                  "  %5 = getelementptr inbounds %FinalClass, ptr %4, i32 0, i32 1\n"
		                  "  store i32 1, ptr %5, align 4\n"
		                  "  ret void\n"
		                  "}\n"
		                  "\n"
		                  "define void @FinalClass_F10FinalClass(ptr %0, ptr %1) {\n"
		                  "entry:\n"
		                  "  %2 = alloca ptr, align 8\n"
		                  "  %3 = alloca ptr, align 8\n"
		                  "  store ptr %0, ptr %2, align 8\n"
		                  "  store ptr %1, ptr %3, align 8\n"
		                  "  ret void\n"
		                  "}\n");
	}

	TEST_F(CodeGenTest, CGClassUpcastBaseFieldWrite) {
		/**
		 * Fly code (bug #8 regression — base field access through an upcast variable):
		 * class BaseClass { int b }
		 * class TestClass : BaseClass { int t }   // adds a field, so the layout differs
		 * void func() {
		 *   BaseClass a = new TestClass()
		 *   a.b = 7
		 * }
		 * The upcast must adjust `a` to the BaseClass subobject so `a.b` writes the right slot.
		 */
		ASTModule *Module = CreateModule();

		llvm::SmallVector<ASTType *, 4> BaseSupers;
		ASTClass *BaseClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "BaseClass", TopModifiers, BaseSupers);
		ASTVar *bAttr = ASTBuilder::CreateClassAttribute(SourceLoc, BaseClass, IntTypeRef, "b", TopModifiers);

		llvm::SmallVector<ASTType *, 4> TestSupers;
		TestSupers.push_back(CreateType(BaseClass));
		ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass", TopModifiers, TestSupers);
		ASTBuilder::CreateClassAttribute(SourceLoc, TestClass, IntTypeRef, "t", TopModifiers);

		ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);

		// BaseClass a = new TestClass()
		ASTType *BaseT = CreateType(BaseClass);
		ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(SourceLoc, BaseT, "a", EmptyModifiers);
		ASTDeclStmt *aDecl = ASTBuilder::CreateDeclStmt(Body, SourceLoc, aVar);
		ASTCall *ctor = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);
		ASTBinary *initAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aVar), ctor);
		aDecl->setExpr(initAssign);

		// a.b = 7
		ASTMember *a_b = ASTBuilder::CreateMember(SourceLoc, bAttr->getName(), ASTBuilder::CreateIdentifier(aVar));
		ASTExprStmt *stmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
		ASTValue *v7 = ASTBuilder::CreateNumberValue(SourceLoc, "7");
		ASTBinary *fieldAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, a_b, v7);
		stmt->setExpr(fieldAssign);

		ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

		Generate();
		llvm::Module *M = getModules()[0];
		std::string output = getOutput(M);

		EXPECT_EQ(output, "\n"
					  "%error = type { i32, ptr, ptr }\n"
					  "%BaseClass = type { ptr, i32 }\n"
					  "%TestClass = type { ptr, %BaseClass, i32 }\n"
					  "\n"
					  "@error = external constant %error\n"
					  "@vtable.BaseClass = constant [2 x ptr] [ptr null, ptr @BaseClass_F9BaseClass]\n"
					  "@vtable.TestClass = constant [2 x ptr] [ptr null, ptr @TestClass_F9TestClass]\n"
					  "\n"
					  "define linkonce_odr ptr @BaseClass.init_ctor(ptr %0) {\n"
					  "entry:\n"
					  "  %1 = alloca ptr, align 8\n"
					  "  store ptr %0, ptr %1, align 8\n"
					  "  %2 = load ptr, ptr %1, align 8\n"
					  "  %3 = getelementptr inbounds %BaseClass, ptr %2, i32 0, i32 0\n"
					  "  store ptr @vtable.BaseClass, ptr %3, align 8\n"
					  "  %4 = getelementptr inbounds %BaseClass, ptr %2, i32 0, i32 1\n"
					  "  store i32 0, ptr %4, align 4\n"
					  "  ret ptr %2\n"
					  "}\n"
					  "\n"
					  "define void @BaseClass_F9BaseClass(ptr %0, ptr %1) {\n"
					  "entry:\n"
					  "  %2 = alloca ptr, align 8\n"
					  "  %3 = alloca ptr, align 8\n"
					  "  store ptr %0, ptr %2, align 8\n"
					  "  store ptr %1, ptr %3, align 8\n"
					  "  ret void\n"
					  "}\n"
					  "\n"
					  "define linkonce_odr ptr @TestClass.init_ctor(ptr %0) {\n"
					  "entry:\n"
					  "  %1 = alloca ptr, align 8\n"
					  "  store ptr %0, ptr %1, align 8\n"
					  "  %2 = load ptr, ptr %1, align 8\n"
					  "  %3 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 0\n"
					  "  store ptr @vtable.TestClass, ptr %3, align 8\n"
					  "  %4 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 1\n"
					  "  %5 = call ptr @BaseClass.init_ctor(ptr %4)\n"
					  "  %6 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 2\n"
					  "  store i32 0, ptr %6, align 4\n"
					  "  ret ptr %2\n"
					  "}\n"
					  "\n"
					  "define void @TestClass_F9TestClass(ptr %0, ptr %1) {\n"
					  "entry:\n"
					  "  %2 = alloca ptr, align 8\n"
					  "  %3 = alloca ptr, align 8\n"
					  "  store ptr %0, ptr %2, align 8\n"
					  "  store ptr %1, ptr %3, align 8\n"
					  "  ret void\n"
					  "}\n"
					  "\n"
					  "define void @_F4func(ptr %0) {\n"
					  "entry:\n"
					  "  %1 = alloca ptr, align 8\n"
					  "  %2 = alloca ptr, align 8\n"
					  "  store ptr %0, ptr %1, align 8\n"
					  "  %3 = load ptr, ptr %1, align 8\n"
					  // BaseClass a = new TestClass()  — upcast adjusts to the BaseClass subobject (index 1)
					  "  %4 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%TestClass, ptr null, i32 1) to i64))\n"
					  "  %5 = call ptr @TestClass.init_ctor(ptr %4)\n"
					  "  call void @TestClass_F9TestClass(ptr %3, ptr %5)\n"
					  "  %6 = getelementptr inbounds %TestClass, ptr %5, i32 0, i32 1\n"
					  "  store ptr %6, ptr %2, align 8\n"
					  // a.b = 7  — field 1 of BaseClass, GEP'd from the adjusted base-subobject pointer
					  "  %7 = load ptr, ptr %2, align 8\n"
					  "  %8 = getelementptr inbounds %BaseClass, ptr %7, i32 0, i32 1\n"
					  "  store i32 7, ptr %8, align 4\n"
					  "  ret void\n"
					  "}\n"
					  "\n"
					  "declare ptr @malloc(i64)\n");
	}

	TEST_F(CodeGenTest, CGClassOverrideDispatch) {
		/**
		 * Fly code (polymorphism — overridden method dispatched through a base variable):
		 * class BaseClass { void run() { } }
		 * class TestClass : BaseClass { void run() { } }   // override
		 * void func() {
		 *   BaseClass a = new TestClass()
		 *   a.run()   // must dispatch to TestClass::run via the BaseClass vtable slot (thunk)
		 * }
		 */
		ASTModule *Module = CreateModule();

		llvm::SmallVector<ASTType *, 4> BaseSupers;
		ASTClass *BaseClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "BaseClass", TopModifiers, BaseSupers);
		ASTBlockStmt *BaseRunBody = ASTBuilder::CreateBlockStmt(SourceLoc);
		ASTFunction *BaseRun = ASTBuilder::CreateClassMethod(SourceLoc, BaseClass, "run", TopModifiers, Params, BaseRunBody);

		llvm::SmallVector<ASTType *, 4> TestSupers;
		TestSupers.push_back(CreateType(BaseClass));
		ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass", TopModifiers, TestSupers);
		ASTBlockStmt *TestRunBody = ASTBuilder::CreateBlockStmt(SourceLoc);
		ASTBuilder::CreateClassMethod(SourceLoc, TestClass, "run", TopModifiers, Params, TestRunBody);

		ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);

		// BaseClass a = new TestClass()
		ASTType *BaseT = CreateType(BaseClass);
		ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(SourceLoc, BaseT, "a", EmptyModifiers);
		ASTDeclStmt *aDecl = ASTBuilder::CreateDeclStmt(Body, SourceLoc, aVar);
		ASTCall *ctor = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);
		ASTBinary *initAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aVar), ctor);
		aDecl->setExpr(initAssign);

		// a.run()
		llvm::SmallVector<ASTExpr *, 8> runArgs;
		ASTCall *runCall = ASTBuilder::CreateCall(SourceLoc, BaseRun->getName(), runArgs, ASTCallKind::CALL_DIRECT, ASTBuilder::CreateIdentifier(aVar));
		ASTExprStmt *runStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
		runStmt->setExpr(runCall);

		ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

		Generate();
		llvm::Module *M = getModules()[0];
		std::string output = getOutput(M);

		EXPECT_EQ(output, "\n"
					  "%error = type { i32, ptr, ptr }\n"
					  "%BaseClass = type { ptr }\n"
					  "%TestClass = type { ptr, %BaseClass }\n"
					  "\n"
					  "@error = external constant %error\n"
					  "@vtable.BaseClass = constant [3 x ptr] [ptr null, ptr @BaseClass_F3run, ptr @BaseClass_F9BaseClass]\n"
					  "@vtable.TestClass = constant [3 x ptr] [ptr null, ptr @TestClass_F3run, ptr @TestClass_F9TestClass]\n"
					  // per-base vtable: offset-to-top (-8) + a this-adjusting thunk to the override
					  "@vtable.TestClass.BaseClass = constant [3 x ptr] [ptr inttoptr (i64 -8 to ptr), ptr @thunk.TestClass.BaseClass.run.1, ptr null]\n"
					  "\n"
					  "define linkonce_odr ptr @BaseClass.init_ctor(ptr %0) {\n"
					  "entry:\n"
					  "  %1 = alloca ptr, align 8\n"
					  "  store ptr %0, ptr %1, align 8\n"
					  "  %2 = load ptr, ptr %1, align 8\n"
					  "  %3 = getelementptr inbounds %BaseClass, ptr %2, i32 0, i32 0\n"
					  "  store ptr @vtable.BaseClass, ptr %3, align 8\n"
					  "  ret ptr %2\n"
					  "}\n"
					  "\n"
					  "define void @BaseClass_F3run(ptr %0, ptr %1) {\n"
					  "entry:\n"
					  "  %2 = alloca ptr, align 8\n"
					  "  %3 = alloca ptr, align 8\n"
					  "  store ptr %0, ptr %2, align 8\n"
					  "  store ptr %1, ptr %3, align 8\n"
					  "  ret void\n"
					  "}\n"
					  "\n"
					  "define void @BaseClass_F9BaseClass(ptr %0, ptr %1) {\n"
					  "entry:\n"
					  "  %2 = alloca ptr, align 8\n"
					  "  %3 = alloca ptr, align 8\n"
					  "  store ptr %0, ptr %2, align 8\n"
					  "  store ptr %1, ptr %3, align 8\n"
					  "  ret void\n"
					  "}\n"
					  "\n"
					  "define linkonce_odr ptr @TestClass.init_ctor(ptr %0) {\n"
					  "entry:\n"
					  "  %1 = alloca ptr, align 8\n"
					  "  store ptr %0, ptr %1, align 8\n"
					  "  %2 = load ptr, ptr %1, align 8\n"
					  "  %3 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 0\n"
					  "  store ptr @vtable.TestClass, ptr %3, align 8\n"
					  "  %4 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 1\n"
					  "  %5 = call ptr @BaseClass.init_ctor(ptr %4)\n"
					  "  store ptr @vtable.TestClass.BaseClass, ptr %4, align 8\n"
					  "  ret ptr %2\n"
					  "}\n"
					  "\n"
					  "define void @TestClass_F3run(ptr %0, ptr %1) {\n"
					  "entry:\n"
					  "  %2 = alloca ptr, align 8\n"
					  "  %3 = alloca ptr, align 8\n"
					  "  store ptr %0, ptr %2, align 8\n"
					  "  store ptr %1, ptr %3, align 8\n"
					  "  ret void\n"
					  "}\n"
					  "\n"
					  "define void @TestClass_F9TestClass(ptr %0, ptr %1) {\n"
					  "entry:\n"
					  "  %2 = alloca ptr, align 8\n"
					  "  %3 = alloca ptr, align 8\n"
					  "  store ptr %0, ptr %2, align 8\n"
					  "  store ptr %1, ptr %3, align 8\n"
					  "  ret void\n"
					  "}\n"
					  "\n"
					  // thunk: shift `this` back to the most-derived TestClass, then call the override
					  "define internal void @thunk.TestClass.BaseClass.run.1(ptr %0, ptr %1) {\n"
					  "entry:\n"
					  "  %2 = ptrtoint ptr %1 to i64\n"
					  "  %3 = sub i64 %2, 8\n"
					  "  %4 = inttoptr i64 %3 to ptr\n"
					  "  call void @TestClass_F3run(ptr %0, ptr %4)\n"
					  "  ret void\n"
					  "}\n"
					  "\n"
					  "define void @_F4func(ptr %0) {\n"
					  "entry:\n"
					  "  %1 = alloca ptr, align 8\n"
					  "  %2 = alloca ptr, align 8\n"
					  "  store ptr %0, ptr %1, align 8\n"
					  "  %3 = load ptr, ptr %1, align 8\n"
					  // BaseClass a = new TestClass()  — upcast to the BaseClass subobject
					  "  %4 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%TestClass, ptr null, i32 1) to i64))\n"
					  "  %5 = call ptr @TestClass.init_ctor(ptr %4)\n"
					  "  call void @TestClass_F9TestClass(ptr %3, ptr %5)\n"
					  "  %6 = getelementptr inbounds %TestClass, ptr %5, i32 0, i32 1\n"
					  "  store ptr %6, ptr %2, align 8\n"
					  // a.run()  — load the base-subobject vtable, slot 1 = the thunk → TestClass::run
					  "  %7 = load ptr, ptr %2, align 8\n"
					  "  %8 = getelementptr inbounds nuw %BaseClass, ptr %7, i32 0, i32 0\n"
					  "  %9 = load ptr, ptr %8, align 8\n"
					  "  %10 = getelementptr ptr, ptr %9, i64 1\n"
					  "  %11 = load ptr, ptr %10, align 8\n"
					  "  call void %11(ptr %3, ptr %7)\n"
					  "  ret void\n"
					  "}\n"
					  "\n"
					  "declare ptr @malloc(i64)\n");
	}

	TEST_F(CodeGenTest, CGClassMultiLevelUpcast) {
		/**
		 * Fly code (multi-level inheritance — upcast across two levels):
		 * class A { int a }
		 * class B : A { int b }
		 * class C : B { int c }
		 * void func() {
		 *   A x = new C()   // C → A: recursive base-subobject navigation
		 *   x.a = 5
		 * }
		 */
		ASTModule *Module = CreateModule();

		llvm::SmallVector<ASTType *, 4> NoSupers;
		ASTClass *A = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "A", TopModifiers, NoSupers);
		ASTVar *aAttr = ASTBuilder::CreateClassAttribute(SourceLoc, A, IntTypeRef, "a", TopModifiers);

		llvm::SmallVector<ASTType *, 4> BSupers;
		BSupers.push_back(CreateType(A));
		ASTClass *B = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "B", TopModifiers, BSupers);
		ASTBuilder::CreateClassAttribute(SourceLoc, B, IntTypeRef, "b", TopModifiers);

		llvm::SmallVector<ASTType *, 4> CSupers;
		CSupers.push_back(CreateType(B));
		ASTClass *C = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "C", TopModifiers, CSupers);
		ASTBuilder::CreateClassAttribute(SourceLoc, C, IntTypeRef, "c", TopModifiers);

		ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);

		// A x = new C()
		ASTType *AType = CreateType(A);
		ASTLocalVar *xVar = ASTBuilder::CreateLocalVar(SourceLoc, AType, "x", EmptyModifiers);
		ASTDeclStmt *xDecl = ASTBuilder::CreateDeclStmt(Body, SourceLoc, xVar);
		ASTCall *ctor = ASTBuilder::CreateCall(SourceLoc, C->getName(), Args, ASTCallKind::CALL_NEW);
		ASTBinary *initAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(xVar), ctor);
		xDecl->setExpr(initAssign);

		// x.a = 5
		ASTMember *x_a = ASTBuilder::CreateMember(SourceLoc, aAttr->getName(), ASTBuilder::CreateIdentifier(xVar));
		ASTExprStmt *stmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
		ASTValue *v5 = ASTBuilder::CreateNumberValue(SourceLoc, "5");
		ASTBinary *fieldAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, x_a, v5);
		stmt->setExpr(fieldAssign);

		ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

		Generate();
		llvm::Module *M = getModules()[0];
		std::string output = getOutput(M);

		// Behavioral checks (the full module IR is large; assert the inheritance-specific shape):
		// nested subobject layout, recursive base init, the two-step C→A upcast adjustment, and
		// the field write GEP'd against A from the adjusted pointer.
		EXPECT_NE(output.find("%A = type { ptr, i32 }"), std::string::npos);
		EXPECT_NE(output.find("%B = type { ptr, %A, i32 }"), std::string::npos);
		EXPECT_NE(output.find("%C = type { ptr, %B, i32 }"), std::string::npos);
		// C.init_ctor initialises its B subobject, which initialises its A subobject.
		EXPECT_NE(output.find("call ptr @B.init_ctor"), std::string::npos);
		EXPECT_NE(output.find("call ptr @A.init_ctor"), std::string::npos);
		// Upcast C* → A*: GEP into C's B subobject (index 1), then B's A subobject (index 1).
		EXPECT_NE(output.find("getelementptr inbounds %C, ptr %5, i32 0, i32 1"), std::string::npos);
		EXPECT_NE(output.find("getelementptr inbounds %B, ptr %6, i32 0, i32 1"), std::string::npos);
		// x.a = 5: field 1 of A from the adjusted A-subobject pointer.
		EXPECT_NE(output.find("getelementptr inbounds %A, ptr %8, i32 0, i32 1"), std::string::npos);
		EXPECT_NE(output.find("store i32 5, ptr %9"), std::string::npos);
	}

} // anonymous namespace
