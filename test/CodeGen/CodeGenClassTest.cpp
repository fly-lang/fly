//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend tests
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
                        "define ptr @TestClass.init_ctor(ptr %0) {\n"
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
                        "  %6 = load %TestClass, ptr %2, align 8\n"
                        "  %7 = getelementptr inbounds %TestClass, %TestClass %6, i32 0, i32 1\n"
                        "  store i32 2, %TestClass %7, align 4\n"
                        "  call void @free(%TestClass %6)\n"
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
                        "define ptr @TestClass.init_ctor(ptr %0) {\n"
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
                        "  %5 = load %TestClass, ptr %4, align 8\n"
                        "  %6 = getelementptr inbounds %TestClass, %TestClass %5, i32 0, i32 1\n"
                        "  %7 = load i32, %TestClass %6, align 4\n"
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
                        "  %7 = load %TestClass, ptr %2, align 8\n"
                        "  %8 = load i32, ptr %3, align 4\n"
                        "  %9 = alloca i32, align 4\n"
                        "  store i32 %8, ptr %9, align 4\n"
                        "  %10 = getelementptr inbounds nuw %TestClass, %TestClass %7, i32 0, i32 0\n"
                        "  %11 = load ptr, %TestClass %10, align 8\n"
                        "  %12 = getelementptr ptr, ptr %11, i64 1\n"
                        "  %13 = load ptr, ptr %12, align 8\n"
                        "  call void %13(ptr %4, %TestClass %7, ptr %9)\n"
                        "  call void @free(%TestClass %7)\n"
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
                        "define ptr @TestClass.init_ctor(ptr %0) {\n"
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
                        "  %5 = load %TestClass, ptr %4, align 8\n"
                        "  %6 = getelementptr inbounds %TestClass, %TestClass %5, i32 0, i32 1\n"
                        "  %7 = load i32, ptr %2, align 4\n"
                        "  store i32 %7, %TestClass %6, align 4\n"
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
                        "  %6 = load %TestClass, ptr %2, align 8\n"
                        "  %7 = alloca i32, align 4\n"
                        "  store i32 1, ptr %7, align 4\n"
                        "  %8 = getelementptr inbounds nuw %TestClass, %TestClass %6, i32 0, i32 0\n"
                        "  %9 = load ptr, %TestClass %8, align 8\n"
                        "  %10 = getelementptr ptr, ptr %9, i64 1\n"
                        "  %11 = load ptr, ptr %10, align 8\n"
                        "  call void %11(ptr %3, %TestClass %6, ptr %7)\n"
                        "  call void @free(%TestClass %6)\n"
                        "  ret void\n"
                        "}\n"
                        "\n"
                        "declare ptr @malloc(i64)\n"
                        "\n"
                        "declare void @free(ptr)\n");
    }

	TEST_F(CodeGenTest, DISABLED_CGClassStaticAttributes) {
        /**
         * Fly code:
         * class TestClass {
         *   static int a
         *   static loadA(const int a) {
         *
         *   }
         * }
         * void func() {
         *   TestClass test = new TestClass()
         *   int x = test.getA()
         *   delete test
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
    	ASTType *TestClassType = CreateType(TestClass);
        ASTExprStmt * attrStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTValue *value2Expr = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        ASTIdentifier *attrIdent = ASTBuilder::CreateIdentifier(aAttribute);
        ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, attrIdent, value2Expr);
        attrStmt->setExpr(AssignExpr);

		// Generate Code
		Generate();
		llvm::Module * M = getModules()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i32, i8*, i8* }\n"
						  "%TestClass = type { i8**, i32 }\n"
                          "\n"
						  "@error = external constant %error\n"
						  "@vtable.TestClass = constant [2 x i8*] [i8* null, i8* bitcast (void (%error*, %TestClass*)* @TestClass_F9TestClass to i8*)]\n"
						  "@0 = external global i32\n"
                          "\n"
						  "define void @_F4func(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  // TestClass.a = 2
						  "  store i32 2, i32* @0, align 4\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
						  "define %TestClass* @TestClass.init_ctor(%TestClass* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %TestClass*, align 8\n"
						  "  store %TestClass* %0, %TestClass** %1, align 8\n"
						  "  %2 = load %TestClass*, %TestClass** %1, align 8\n"
						  "  %3 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 0\n"
						  "  store i8** getelementptr inbounds ([2 x i8*], [2 x i8*]* @vtable.TestClass, i32 0, i32 0), i8*** %3, align 8\n"
						  "  %4 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 0\n"
						  "  store i32 0, i8*** %4, align 4\n"
						  "  ret %TestClass* %2\n"
						  "}\n"
						  "\n"
                          "define void @TestClass_F9TestClass(%error* %0, %TestClass* %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca %TestClass*, align 8\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store %TestClass* %1, %TestClass** %3, align 8\n"
                          "  ret void\n"
                          "}\n"
        );
    }

	TEST_F(CodeGenTest, DISABLED_CGClassStaticMethods) {
        /**
         * Fly code:
         * class TestClass {
         *   static int do() {
         *     return 1
         *   }
         * }
         * void func() {
         *   int x = TestClass.do()
         * }
         */
        ASTModule *Module = CreateModule();

        // TestClass {
        //   static int do() { return 1 }
        // }
        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
                                                  TopModifiers, SuperClasses);

        ASTBlockStmt *aFuncBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	llvm::SmallVector<ASTModifier *, 8> Modifiers;
    	Modifiers.push_back(ASTBuilder::CreateModifier(SourceLoc, ASTModifierKind::MOD_STATIC));
        llvm::SmallVector<ASTParam *, 8> Params;
        ASTFunction *aFunc = ASTBuilder::CreateClassMethod(SourceLoc, TestClass,
                                                          "do", Modifiers, Params, aFuncBody);

        ASTReturnStmt * aFuncReturn = ASTBuilder::CreateReturnStmt(aFuncBody, SourceLoc);

        // int main() {
        //  int a = TestClass.do()
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // int a = TestClass.do()
        ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        ASTDeclStmt *aDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, aVar);
    	ASTIdentifier *TestClassType = ASTBuilder::CreateIdentifier(SourceLoc, TestClass->getName());
        ASTCall *aCallExpr = ASTBuilder::CreateCall(SourceLoc, aFunc->getName(), Args, ASTCallKind::CALL_DIRECT, TestClassType);
        ASTIdentifier *aIdent = ASTBuilder::CreateIdentifier(aVar);
        ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, aIdent, aCallExpr);
        aDeclStmt->setExpr(AssignExpr);

    	// Generate Code
    	Generate();
		llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i32, i8*, i8* }\n"
						  "%TestClass = type { i8** }\n"
                          "\n"
						  "@error = external constant %error\n"
						  "@vtable.TestClass = constant [3 x i8*] [i8* null, i8* bitcast (void (%error*, %TestClass*)* @TestClass_F9TestClass to i8*), i8* bitcast (i32 (%error*)* @TestClass_F2do to i8*)]\n"
						  "\n"
                          "define void @_F4func(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  %2 = alloca i32, align 4\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  %3 = load %error*, %error** %1, align 8\n"
						  "  %4 = call i32 @TestClass_F2do(%error* %3)\n"
						  "  store i32 %4, i32* %2, align 4\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
						  "define %TestClass* @TestClass.init_ctor(%TestClass* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %TestClass*, align 8\n"
						  "  store %TestClass* %0, %TestClass** %1, align 8\n"
						  "  %2 = load %TestClass*, %TestClass** %1, align 8\n"
						  "  %3 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 0\n"
						  "  store i8** getelementptr inbounds ([3 x i8*], [3 x i8*]* @vtable.TestClass, i32 0, i32 0), i8*** %3, align 8\n"
						  "  ret %TestClass* %2\n"
						  "}\n"
						  "\n"
                          "define void @TestClass_F9TestClass(%error* %0, %TestClass* %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca %TestClass*, align 8\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store %TestClass* %1, %TestClass** %3, align 8\n"
                          "  ret void\n"
                          "}\n"
                          "\n"
                          "define i32 @TestClass_F2do(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  ret i32 1\n"
                          "}\n"
                          );
    }

	TEST_F(CodeGenTest, DISABLED_CGClassExtendsClass) {
        /**
         * Fly code:
         * class BaseClass {
         *   int a
         *   void do() {
         *   }
         * }
         * class TestClass : BaseClass {
         *   void undo() {
         *   }
         * }
         * void func() {
         *   BaseClass a = new TestClass()
         *   delete a
         * }
         */
    	ASTModule *Module = CreateModule();

    	// class BaseClass {
    	//   int a
    	//   void do() {}
    	// }
    	//
    	// class TestClass : BaseClass {
    	//   void undo() {}
    	// }
    	//
    	// func() {
    	//  BaseClass a = new TestClass()
    	// }

    	// BaseClass
    	llvm::SmallVector<ASTType *, 4> BaseSuperClasses;
    	ASTClass *BaseClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "BaseClass",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTVar *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, BaseClass, IntTypeRef, "a", TopModifiers);
    	// void do()
    	ASTBlockStmt *DoBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *BaseClass_do = ASTBuilder::CreateClassMethod(SourceLoc, BaseClass, "do", TopModifiers, Params, DoBody);

    	// TestClass
    	llvm::SmallVector<ASTType *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(CreateType(BaseClass));
    	ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopModifiers, TestSuperClasses);
    	// void undo()
    	ASTBlockStmt *UndoBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *TestClass_undo = ASTBuilder::CreateClassMethod(SourceLoc, TestClass, "undo", TopModifiers, Params, UndoBody);

    	// func() { BaseClass a = new TestClass() }
    	ASTBlockStmt *MainBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);

    	ASTType *Base = CreateType(BaseClass);
    	ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(SourceLoc, Base, "a", EmptyModifiers);
    	ASTDeclStmt *aDeclStmt = ASTBuilder::CreateDeclStmt(MainBody, SourceLoc, aVar);
    	ASTIdentifier *aIdent = ASTBuilder::CreateIdentifier(aVar);
    	ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, aIdent, ConstructorCall);
    	aDeclStmt->setExpr(AssignExpr);
    	ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, MainBody);

    	// Generate Code
    	Generate();
		llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M);

    	// TODO:
    	// Missing step: ctors must store the vtable pointer into the object.
    	// Derived ctor should call base ctor
	    // Hardcoded object size should be replaced with sizeof computed in IR.
	    // Offset - to - top is trivial now(always 0), but will matter with multiple inheritance

	    EXPECT_EQ(output, "\n"
						  "%error = type { i32, i8*, i8* }\n"
						  "%BaseClass = type { i8**, i32 }\n"
    					  "%TestClass = type { i8**, %BaseClass, i32 }\n"
						  "\n"
						  "@error = external constant %error\n"
						  "@vtable.BaseClass = constant [3 x i8*] [i8* null, i8* bitcast (void (%error*, %BaseClass*)* @BaseClass_F9BaseClass to i8*), i8* bitcast (void (%error*, %BaseClass*)* @BaseClass_F2do to i8*)]\n"
						  "@vtable.TestClass = constant [3 x i8*] [i8* null, i8* bitcast (void (%error*, %TestClass*)* @TestClass_F9TestClass to i8*), i8* bitcast (void (%error*, %TestClass*)* @TestClass_F4undo to i8*)]\n"
						  "\n"
						  "define void @_F4func(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  %2 = alloca %BaseClass*, align 8\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  %3 = load %error*, %error** %1, align 8\n"
						  "  %4 = call i8* @malloc(i64 ptrtoint (%TestClass* getelementptr (%TestClass, %TestClass* null, i32 1) to i64))\n"
						  "  %5 = bitcast i8* %4 to %TestClass*\n"
						  "  %6 = call %TestClass* @TestClass.init_ctor(%TestClass* %5)\n"
						  "  call void @TestClass_F9TestClass(%error* %3, %TestClass* %6)\n"
						  "  store %TestClass* %6, %TestClass** %2, align 8\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
						  "define %BaseClass* @BaseClass.init_ctor(%BaseClass* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %BaseClass*, align 8\n"
						  "  store %BaseClass* %0, %BaseClass** %1, align 8\n"
						  "  %2 = load %BaseClass*, %BaseClass** %1, align 8\n"
						  "  %3 = getelementptr inbounds %BaseClass, %BaseClass* %2, i32 0, i32 0\n"
						  "  store i8** getelementptr inbounds ([3 x i8*], [3 x i8*]* @vtable.BaseClass, i32 0, i32 0), i8*** %3, align 8\n"
						  "  %4 = getelementptr inbounds %BaseClass, %BaseClass* %2, i32 0, i32 1\n"
						  "  store i32 0, i32* %4, align 4\n"
						  "  ret %BaseClass* %2\n"
						  "}\n"
						  "\n"
						  "define void @BaseClass_F9BaseClass(%error* %0, %BaseClass* %1) {\n"
						  "entry:\n"
						  "  %2 = alloca %error*, align 8\n"
						  "  %3 = alloca %BaseClass*, align 8\n"
						  "  store %error* %0, %error** %2, align 8\n"
						  "  store %BaseClass* %1, %BaseClass** %3, align 8\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
						  "define void @BaseClass_F2do(%error* %0, %BaseClass* %1) {\n"
						  "entry:\n"
						  "  %2 = alloca %error*, align 8\n"
						  "  %3 = alloca %BaseClass*, align 8\n"
						  "  store %error* %0, %error** %2, align 8\n"
						  "  store %BaseClass* %1, %BaseClass** %3, align 8\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
						  "define %TestClass* @TestClass.init_ctor(%TestClass* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %TestClass*, align 8\n"
						  "  store %TestClass* %0, %TestClass** %1, align 8\n"
						  "  %2 = load %TestClass*, %TestClass** %1, align 8\n"
						  "  %3 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 0\n"
						  "  store i8** getelementptr inbounds ([3 x i8*], [3 x i8*]* @vtable.TestClass, i32 0, i32 0), i8*** %3, align 8\n"
						  "  %4 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 1\n"
						  "  %5 = call %BaseClass* @BaseClass.init_ctor(%BaseClass* %4)\n"
						  "  %6 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 2\n"
						  "  store i32 0, i32* %6, align 4\n"
						  "  ret %TestClass* %2\n"
						  "}\n"
						  "\n"
						  "define void @TestClass_F9TestClass(%error* %0, %TestClass* %1) {\n"
						  "entry:\n"
						  "  %2 = alloca %error*, align 8\n"
						  "  %3 = alloca %TestClass*, align 8\n"
						  "  store %error* %0, %error** %2, align 8\n"
						  "  store %TestClass* %1, %TestClass** %3, align 8\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
						  "define void @TestClass_F4undo(%error* %0, %TestClass* %1) {\n"
						  "entry:\n"
						  "  %2 = alloca %error*, align 8\n"
						  "  %3 = alloca %TestClass*, align 8\n"
						  "  store %error* %0, %error** %2, align 8\n"
						  "  store %TestClass* %1, %TestClass** %3, align 8\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
                          "declare i8* @malloc(i64)\n"
						  );
    }


	TEST_F(CodeGenTest, DISABLED_CGClassExtendsStruct) {
		/**
		 * Fly code:
		 * struct BaseStruct {
		 *   int a
		 * }
		 * interface BaseInterface {
		 *   void do()
		 * }
		 * class TestClass : BaseStruct, BaseInterface {
		 *   void do() {
		 *   }
		 * }
		 * void func() {
		 *   TestClass test = new TestClass()
		 *   delete test
		 * }
		 */
		ASTModule *Module = CreateModule();

		// struct BaseStruct {
		// int a
		// }
		// class BaseClass {
		// void undo() {}
		// }
		// interface BaseInterface {
		// void do()
		// }
		//
		// class TestClass : BaseStruct, BaseClass, BaseInterface {
		//	void do() {
		//		a = 2
		//	    undo()
		//  }
		// }
	}

	TEST_F(CodeGenTest, DISABLED_CGClassExtendsInterface) {
		/**
		 * Fly code:
		 * struct BaseStruct {
		 *   int a
		 * }
		 * interface BaseInterface {
		 *   void do()
		 * }
		 * class TestClass : BaseStruct, BaseInterface {
		 *   void do() {
		 *   }
		 * }
		 * void func() {
		 *   TestClass test = new TestClass()
		 *   delete test
		 * }
		 */
		ASTModule *Module = CreateModule();

		// struct BaseStruct {
		// int a
		// }
		// class BaseClass {
		// void undo() {}
		// }
		// interface BaseInterface {
		// void do()
		// }
		//
		// class TestClass : BaseStruct, BaseClass, BaseInterface {
		//	void do() {
		//		a = 2
		//	    undo()
		//  }
		// }
	}

	TEST_F(CodeGenTest, DISABLED_CGClassExtendsStructAndInterface) {
        /**
         * Fly code:
         * struct BaseStruct {
         *   int a
         * }
         * interface BaseInterface {
         *   void do()
         * }
         * class TestClass : BaseStruct, BaseInterface {
         *   void do() {
         *   }
         * }
         * void func() {
         *   TestClass test = new TestClass()
         *   delete test
         * }
         */
        ASTModule *Module = CreateModule();

    	// struct BaseStruct {
    	// int a
    	// }
    	// class BaseClass {
    	// void undo() {}
    	// }
    	// interface BaseInterface {
    	// void do()
    	// }
    	//
    	// class TestClass : BaseStruct, BaseClass, BaseInterface {
    	//	void do() {
    	//		a = 2
    	//	    undo()
    	//  }
    	// }
    }
} // anonymous namespace
