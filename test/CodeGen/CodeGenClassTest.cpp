//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

// fly
#include "CodeGenTest.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTIdentifier.h"

#include <AST/ASTAttribute.h>
#include <AST/ASTExprStmt.h>
#include <AST/ASTDeclStmt.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTMethod.h>
#include <AST/ASTParam.h>
#include <AST/ASTReturnStmt.h>

namespace {

    using namespace fly;

    TEST_F(CodeGenTest, CGStruct) {
        /**
         * Fly code:
         * struct TestStruct {
         *   int a
         * }
         * void func() {
         *   new TestStruct()
         * }
         */
        ASTModule *Module = CreateModule();

        // struct TestStruct {
    	//  int a
    	// }
        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct", TopModifiers, SuperClasses);
        ASTAttribute *aField = ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

        // func() {
        //    new TestStruct()
        // }
        ASTBlockStmt *MainBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestStruct->getName(), Args, ASTCallKind::CALL_NEW);
    	ASTExprStmt *testNewStmt = ASTBuilder::CreateExprStmt(MainBody, SourceLoc);
    	testNewStmt->setExpr(ConstructorCall);
    	ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, MainBody);

		// Generate Code
		llvm::Module * M = Generate()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n"
        				  "%error = type { i8, i32, i8* }\n"
						  "%TestStruct = type { i32 }\n"
						  "\n"
						  "@error = external constant %error\n"
        				  "\n"
        				  "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = call i8* @malloc(i64 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i64))\n"
                          "  %3 = bitcast i8* %2 to %TestStruct*\n"
                          "  %4 = call %TestStruct* @TestStruct.init_ctor(%TestStruct* %3)\n"
                          "  ret void\n"
                          "}\n"
                          "\n"
                          "define %TestStruct* @TestStruct.init_ctor(%TestStruct* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %TestStruct*, align 8\n"
						  "  store %TestStruct* %0, %TestStruct** %1, align 8\n"
						  "  %2 = load %TestStruct*, %TestStruct** %1, align 8\n"
						  "  %3 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 0\n"
						  "  store i32 0, i32* %3, align 4\n"
						  "  ret %TestStruct* %2\n"
						  "}\n"
						  "\n"
						  "declare i8* @malloc(i64)\n");
    }

TEST_F(CodeGenTest, CGStructAssignVar) {
        /**
         * Fly code:
         * struct TestStruct {
         *   int a
         * }
         * int func() {
         *   TestStruct test = new TestStruct()
         *   int x = test.a
         *   return 1
         * }
         */
        ASTModule *Module = CreateModule();

        // struct TestStruct {
        //   int a
        //   int b
        //   const int c
        // }

        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct",
                                                   TopModifiers, SuperClasses);
        ASTAttribute *aField = ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);
        // ASTVar *bField = ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "b", TopModifiers);
        // ASTVar *cField = ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "c", TopModifiers);

        // int func() {
        //  TestStruct test = new TestStruct();
        //  int x = test.a
        //  test.b = 2
        //  return 1
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // TestStruct test = new TestStruct()
        ASTType *TestClassType = CreateType(TestStruct);
        ASTLocalVar *TestVar = ASTBuilder::CreateLocalVar(SourceLoc, TestClassType, "test", EmptyModifiers);
        ASTDeclStmt *TestDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TestVar);
    	ASTIdentifier *Instance = ASTBuilder::CreateIdentifier(TestVar);
        ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestStruct->getName(), Args, ASTCallKind::CALL_NEW);
        ASTBinaryOp *AssignExpr1 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, Instance, ConstructorCall);
        TestDeclStmt->setExpr(AssignExpr1);

        // int x = test.a
        ASTLocalVar *xVar = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "x", EmptyModifiers);
        ASTDeclStmt *xDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, xVar);
    	ASTIdentifier *xVar_Id = ASTBuilder::CreateIdentifier(TestVar);
        
        ASTMember *test_aVarRef = ASTBuilder::CreateMember(aField, Instance);
        ASTIdentifier *xIdent = ASTBuilder::CreateIdentifier(xVar);
        ASTBinaryOp *AssignExpr2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, xIdent, test_aVarRef);
        xDeclStmt->setExpr(AssignExpr2);

		// Generate Code
		llvm::Module * M = Generate()[0];
		std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n"
    					  "%error = type { i8, i32, i8* }\n"
						  "%TestStruct = type { i32 }\n"
						  "\n"
						  "@error = external constant %error\n"
						  "\n"
                          "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca %TestStruct*, align 8\n"
                          "  %3 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %4 = call i8* @malloc(i64 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i64))\n"
                          "  %5 = bitcast i8* %4 to %TestStruct*\n"
                          "  %6 = call %TestStruct* @TestStruct.init_ctor(%TestStruct* %5)\n"
                          "  store %TestStruct* %6, %TestStruct** %2, align 8\n"
                          "  %7 = load %TestStruct*, %TestStruct** %2, align 8\n"
                          "  %8 = getelementptr inbounds %TestStruct, %TestStruct* %7, i32 0, i32 0\n"
                          "  %9 = load i32, i32* %8, align 4\n"
                          "  store i32 %9, i32* %3, align 4\n"
                          "  ret void\n"
                          "}\n"
                          "\n"
                          "define %TestStruct* @TestStruct.init_ctor(%TestStruct* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %TestStruct*, align 8\n"
						  "  store %TestStruct* %0, %TestStruct** %1, align 8\n"
						  "  %2 = load %TestStruct*, %TestStruct** %1, align 8\n"
						  "  %3 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 0\n"
						  "  store i32 0, i32* %3, align 4\n"
						  "  ret %TestStruct* %2\n"
						  "}\n"
						  "\n"
                          "declare i8* @malloc(i64)\n"
                          );
    }

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
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // TestClass test = new TestClass()
        ASTType *TestClassType = CreateType(TestClass);
        ASTLocalVar *TestVar = ASTBuilder::CreateLocalVar(SourceLoc, TestClassType, "test", EmptyModifiers);
        ASTDeclStmt *TestDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TestVar);
        ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);
        ASTIdentifier *testIdent = ASTBuilder::CreateIdentifier(TestVar);
        ASTBinaryOp *AssignExpr1 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, testIdent, ConstructorCall);
        TestDeclStmt->setExpr(AssignExpr1);

        //  test.a = 2
    	ASTMember *test_a = ASTBuilder::CreateMember(aAttribute, ASTBuilder::CreateIdentifier(TestVar));
        ASTExprStmt * attrStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTValue *value2 = ASTBuilder::CreateNumberValue(SourceLocation(), "2");
        ASTBinaryOp *AssignExpr2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, test_a, value2);
        attrStmt->setExpr(AssignExpr2);

        // delete test
        ASTBuilder::CreateDeleteStmt(Body, SourceLoc, ASTBuilder::CreateIdentifier(TestVar));

		// Generate Code
		llvm::Module * M = Generate()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n"
						  "%error = type { i8, i32, i8* }\n"
						  "%TestClass = type { i8**, i32 }\n"
                          "\n"
                          "@error = external constant %error\n"
                          "@vtable.TestClass = constant [2 x i8*] [i8* null, i8* bitcast (void (%error*, %TestClass*)* @TestClass_F9TestClass to i8*)]\n"
                          "\n"
						  "define void @_F4func(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  %2 = alloca %TestClass*, align 8\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  %3 = load %error*, %error** %1, align 8\n"
						  // TestClass test = new TestClass()
						  "  %4 = call i8* @malloc(i64 ptrtoint (%TestClass* getelementptr (%TestClass, %TestClass* null, i32 1) to i64))\n"
						  "  %5 = bitcast i8* %4 to %TestClass*\n"
						  "  %6 = call %TestClass* @TestClass.init_ctor(%TestClass* %5)\n"
						  "  call void @TestClass_F9TestClass(%error* %3, %TestClass* %6)\n"
						  "  store %TestClass* %6, %TestClass** %2, align 8\n"
						  "  %7 = load %TestClass*, %TestClass** %2, align 8\n"
						  // test.a = 2
						  "  %8 = getelementptr inbounds %TestClass, %TestClass* %7, i32 0, i32 1\n"
						  "  store i32 2, i32* %8, align 4\n"
						  // delete test
						  "  %9 = load %TestClass*, %TestClass** %2, align 8\n"
						  "  %10 = bitcast %TestClass* %9 to i8*\n"
						  "  tail call void @free(i8* %10)\n"
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
						  "  %4 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 1\n"
						  "  store i32 0, i32* %4, align 4\n"
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
                          "declare i8* @malloc(i64)\n"
                          "\n"
                          "declare void @free(i8*)\n"
        );
    }

    TEST_F(CodeGenTest, CGClassGetterMethod) {
        /**
         * Fly code:
         * class TestClass {
         *   int a
         *   int getA() {
         *     return a
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
        //   int a
        //   int getA() { return a }
        // }
        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass", TopModifiers,
                                                  SuperClasses);

        // int a
        ASTVar *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, TestClass, IntTypeRef, "a", TopModifiers);

        // int getA() { return a }
        ASTBlockStmt *MethodBody = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTMethod *getAMethod = ASTBuilder::CreateClassMethod(SourceLoc, TestClass, IntTypeRef,
                                                               "getA", TopModifiers, Params, MethodBody);

        ASTReturnStmt * MethodReturn = ASTBuilder::CreateReturnStmt(MethodBody, SourceLoc);
        MethodReturn->setExpr(ASTBuilder::CreateIdentifier(aAttribute));

        // int main() {
        //  TestClass test = new TestClass()
        //  int x = test.getA()
        //  delete test
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // TestClass test = new TestClass()
        ASTType *TestClassType = CreateType(TestClass);
        ASTVar *TestVar = ASTBuilder::CreateLocalVar(SourceLoc, TestClassType, "test", EmptyModifiers);
        ASTDeclStmt *TestDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, (ASTLocalVar*)TestVar);
        ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);
        ASTIdentifier *testIdent = ASTBuilder::CreateIdentifier(TestVar);
        ASTBinaryOp *AssignExpr1 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, testIdent, ConstructorCall);
        TestDeclStmt->setExpr(AssignExpr1);

        // int x = test.getA()
        ASTType *xType = getAMethod->getReturnType();
        ASTLocalVar *xVar = ASTBuilder::CreateLocalVar(SourceLoc, xType, "x", EmptyModifiers);
        ASTDeclStmt *xDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, xVar);
        ASTCall *xCallExpr = ASTBuilder::CreateCall(SourceLoc, getAMethod->getName(), Args, ASTCallKind::CALL_DIRECT, ASTBuilder::CreateIdentifier(TestVar));
        ASTIdentifier *xIdent = ASTBuilder::CreateIdentifier(xVar);
        ASTBinaryOp *AssignExpr2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, xIdent, xCallExpr);
        xDeclStmt->setExpr(AssignExpr2);

        // delete test
        ASTBuilder::CreateDeleteStmt(Body, SourceLoc, ASTBuilder::CreateIdentifier(TestVar));

		// Generate Code
		llvm::Module * M = Generate()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n"
						  "%error = type { i8, i32, i8* }\n"
                          "%TestClass = type { i8**, i32 }\n"
                          "\n"
                          "@error = external constant %error\n"
                          "@vtable.TestClass = constant [3 x i8*] [i8* null, i8* bitcast (void (%error*, %TestClass*)* @TestClass_F9TestClass to i8*), i8* bitcast (i32 (%error*, %TestClass*)* @TestClass_F4getA to i8*)]\n"
                          "\n"
                          "define void @_F4func(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  %2 = alloca %TestClass*, align 8\n"
						  "  %3 = alloca i32, align 4\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  %4 = load %error*, %error** %1, align 8\n"
						  // TestClass test = new TestClass()
						  "  %5 = call i8* @malloc(i64 ptrtoint (%TestClass* getelementptr (%TestClass, %TestClass* null, i32 1) to i64))\n"
						  "  %6 = bitcast i8* %5 to %TestClass*\n"
						  "  %7 = call %TestClass* @TestClass.init_ctor(%TestClass* %6)\n"
						  "  call void @TestClass_F9TestClass(%error* %4, %TestClass* %7)\n"
						  "  store %TestClass* %7, %TestClass** %2, align 8\n"
						  "  %8 = load %TestClass*, %TestClass** %2, align 8\n"
						  // int a = test.a()
						  "  %9 = getelementptr inbounds %TestClass, %TestClass* %8, i32 0, i32 0\n"
						  "  %10 = load i8**, i8*** %9, align 8\n"
						  "  %11 = getelementptr i8*, i8** %10, i64 2\n"
						  "  %12 = load i8*, i8** %11, align 8\n"
						  "  %13 = bitcast i8* %12 to i32 (%error*, %TestClass*)*\n"
						  "  %14 = call i32 %13(%error* %4, %TestClass* %8)\n"
						  "  store i32 %14, i32* %3, align 4\n"
						  // delete test
						  "  %15 = load %TestClass*, %TestClass** %2, align 8\n"
						  "  %16 = bitcast %TestClass* %15 to i8*\n"
						  "  tail call void @free(i8* %16)\n"
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
						  "  store i32 0, i32* %4, align 4\n"
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
                          "define i32 @TestClass_F4getA(%error* %0, %TestClass* %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca %TestClass*, align 8\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store %TestClass* %1, %TestClass** %3, align 8\n"
                          "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
                          "  %5 = getelementptr inbounds %TestClass, %TestClass* %4, i32 0, i32 1\n"
                          "  %6 = load i32, i32* %5, align 4\n"
                          "  ret i32 %6\n"
                          "}\n"
                          "\n"
                          "declare i8* @malloc(i64)\n"
                          "\n"
                          "declare void @free(i8*)\n"
        );
    }

	TEST_F(CodeGenTest, CGClassSetterMethod) {
        /**
         * Fly code:
         * class TestClass {
         *   int a
         *   void setA(int value) {
         *     a = value
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
    	ASTParam * aParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
    	setAParams.push_back(aParam);
    	ASTBlockStmt *MethodBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	const SourceLocation &Loc = SourceLoc;
    	ASTMember * this_a = ASTBuilder::CreateMember(aAttribute, ASTBuilder::CreateIdentifier(Loc, "this"));
    	ASTExprStmt * setAttributeAStmt = ASTBuilder::CreateExprStmt(MethodBody, SourceLoc);
    	ASTIdentifier *aParamIdent = ASTBuilder::CreateIdentifier(aParam);
    	ASTBinaryOp *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, this_a, aParamIdent);
    	setAttributeAStmt->setExpr(AssignExpr);
        ASTFunction *setAMethod = ASTBuilder::CreateClassMethod(SourceLoc, TestClass, VoidTypeRef,
                                                               "setA", TopModifiers, setAParams, MethodBody);

        // int func() {
        //  TestClass test = new TestClass()
        //  test.setA(1)
        //  delete test
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // TestClass test = new TestClass()
        ASTType *TestClassType = CreateType(TestClass);
        ASTLocalVar *TestVar = ASTBuilder::CreateLocalVar(SourceLoc, TestClassType, "test", EmptyModifiers);
        ASTDeclStmt *TestDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TestVar);
        ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);
        ASTIdentifier *testIdent = ASTBuilder::CreateIdentifier(TestVar);
        ASTBinaryOp *AssignExpr2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, testIdent, ConstructorCall);
        TestDeclStmt->setExpr(AssignExpr2);

        // test.setA(1)
    	ASTNumberValue * IntValue = ASTBuilder::CreateNumberValue(SourceLocation(), "1");
    	llvm::SmallVector<ASTExpr *, 8> setAArgs;
    	setAArgs.push_back(IntValue);
    	ASTCall *Call = ASTBuilder::CreateCall(SourceLoc, setAMethod->getName(), setAArgs, ASTCallKind::CALL_DIRECT, ASTBuilder::CreateIdentifier(TestVar));
    	ASTExprStmt * ExprStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
    	ExprStmt->setExpr(Call);

        // delete test
        ASTBuilder::CreateDeleteStmt(Body, SourceLoc, ASTBuilder::CreateIdentifier(TestVar));

		// Generate Code
		llvm::Module * M = Generate()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
                          "%TestClass = type { i8**, i32 }\n"
                          "\n"
                          "@error = external constant %error\n"
                          "@vtable.TestClass = constant [3 x i8*] [i8* null, i8* bitcast (void (%error*, %TestClass*)* @TestClass_F9TestClass to i8*), i8* bitcast (void (%error*, %TestClass*, i32)* @TestClass_F4setA_i to i8*)]\n"
                          "\n"
						  "define void @_F4func(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  %2 = alloca %TestClass*, align 8\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  %3 = load %error*, %error** %1, align 8\n"
						  // TestClass test = new TestClass()
						  "  %4 = call i8* @malloc(i64 ptrtoint (%TestClass* getelementptr (%TestClass, %TestClass* null, i32 1) to i64))\n"
						  "  %5 = bitcast i8* %4 to %TestClass*\n"
						  "  %6 = call %TestClass* @TestClass.init_ctor(%TestClass* %5)\n"
						  "  call void @TestClass_F9TestClass(%error* %3, %TestClass* %6)\n"
						  "  store %TestClass* %6, %TestClass** %2, align 8\n"
						  "  %7 = load %TestClass*, %TestClass** %2, align 8\n"
						  // test.setA(1)
						  "  %8 = getelementptr inbounds %TestClass, %TestClass* %7, i32 0, i32 0\n"
						  "  %9 = load i8**, i8*** %8, align 8\n"
						  "  %10 = getelementptr i8*, i8** %9, i64 2\n"
						  "  %11 = load i8*, i8** %10, align 8\n"
						  "  %12 = bitcast i8* %11 to void (%error*, %TestClass*, i32)*\n"
						  "  call void %12(%error* %3, %TestClass* %7, i32 1)\n"
						  // delete test
						  "  %13 = load %TestClass*, %TestClass** %2, align 8\n"
						  "  %14 = bitcast %TestClass* %13 to i8*\n"
						  "  tail call void @free(i8* %14)\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
                          "define %TestClass* @TestClass.init_ctor(%TestClass* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %TestClass*, align 8\n"
                          "  store %TestClass* %0, %TestClass** %1, align 8\n"
                          "  %2 = load %TestClass*, %TestClass** %1, align 8\n"
                          "  %3 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 0\n"
                          "  store i8** getelementptr inbounds ([3 x i8*], [3 x i8*]* @vtable.TestClass, i32 0, i32 0), i8*** %3, align 8\n" // equivalent of %bitcast = bitcast [3 x i8*]* @vtable.TestClass to i8** store i8** %cast, i8*** %3, align 8
                          "  %4 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 1\n"
                          "  store i32 0, i32* %4, align 4\n"
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
                          "define void @TestClass_F4setA_i(%error* %0, %TestClass* %1, i32 %2) {\n"
                          "entry:\n"
                          "  %3 = alloca %error*, align 8\n"
                          "  %4 = alloca %TestClass*, align 8\n"
                          "  %5 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %3, align 8\n"
                          "  store %TestClass* %1, %TestClass** %4, align 8\n"
                          "  store i32 %2, i32* %5, align 4\n"
                          "  %6 = load %TestClass*, %TestClass** %4, align 8\n"
                          "  %7 = load i32, i32* %5, align 4\n"
                          "  %8 = getelementptr inbounds %TestClass, %TestClass* %6, i32 0, i32 1\n"
                          "  store i32 %7, i32* %8, align 4\n"
                          "  ret void\n"
                          "}\n"
                          "\n"
                          "declare i8* @malloc(i64)\n"
                          "\n"
                          "declare void @free(i8*)\n"
        );
    }

	TEST_F(CodeGenTest, CGClassConstructorCallMethod) {
        /**
         * Fly code:
         * class TestClass {
         *   int a
         *   void setA(int value) {
         *     a = value
         *   }
         * }
         * void func() {
         *   TestClass test = new TestClass()
         *   test.setA(1)
         *   delete test
         * }
         */
        ASTModule *Module = CreateModule();

        // TestClass {
    	//   int a
    	//   TestClass() {
    	//     a = a()
    	//   }
        //   int a() { return 1 }
        // }
        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
                                                  TopModifiers, SuperClasses);

        // int a() { return 1 }
        ASTBlockStmt *aFuncBody = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *aFunc = ASTBuilder::CreateClassMethod(SourceLoc, TestClass, IntTypeRef,
                                                          "a", TopModifiers, Params, aFuncBody);
        ASTReturnStmt * aFuncReturn = ASTBuilder::CreateReturnStmt(aFuncBody, SourceLoc);
        ASTValue *aFuncExpr = ASTBuilder::CreateNumberValue(SourceLocation(), "1");
        aFuncReturn->setExpr(aFuncExpr);

        // int main() {
        //  TestClass test = new TestClass()
        //  int a = test.a()
        //  delete test
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // TestClass test = new TestClass()
        ASTType *TestClassType = CreateType(TestClass);
        ASTLocalVar *TestVar = ASTBuilder::CreateLocalVar(SourceLoc, TestClassType, "test", EmptyModifiers);
        ASTDeclStmt *TestDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TestVar);
        ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);
        ASTIdentifier *testIdent = ASTBuilder::CreateIdentifier(TestVar);
        ASTBinaryOp *AssignExpr1 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, testIdent, ConstructorCall);
        TestDeclStmt->setExpr(AssignExpr1);

        // int a = test.a()
        ASTType *aType = aFunc->getReturnType();
        ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(SourceLoc, aType, "a", EmptyModifiers);
        ASTDeclStmt *aDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, aVar);
        ASTCall *aCallExpr = ASTBuilder::CreateCall(SourceLoc, aFunc->getName(), Args, ASTCallKind::CALL_DIRECT, ASTBuilder::CreateIdentifier(TestVar));
        ASTIdentifier *aIdent = ASTBuilder::CreateIdentifier(aVar);
        ASTBinaryOp *AssignExpr2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, aIdent, aCallExpr);
        aDeclStmt->setExpr(AssignExpr2);

        // delete test
        ASTDeleteStmt *DeleteStmt = ASTBuilder::CreateDeleteStmt(Body, SourceLoc, ASTBuilder::CreateIdentifier(TestVar));

    	// Generate Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
                          "%TestClass = type { i8** }\n"
                          "\n"
						  "@error = external constant %error\n"
						  "@vtable.TestClass = constant [3 x i8*] [i8* null, i8* bitcast (void (%error*, %TestClass*)* @TestClass_F9TestClass to i8*), i8* bitcast (i32 (%error*, %TestClass*)* @TestClass_F1a to i8*)]\n"
                          "\n"
                          "define void @_F4func(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  %2 = alloca %TestClass*, align 8\n"
						  "  %3 = alloca i32, align 4\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  %4 = load %error*, %error** %1, align 8\n"
						  "  %5 = call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))\n"
						  "  %6 = bitcast i8* %5 to %TestClass*\n"
						  "  %7 = call %TestClass* @TestClass.init_ctor(%TestClass* %6)\n"
						  "  call void @TestClass_F9TestClass(%error* %4, %TestClass* %7)\n"
						  "  store %TestClass* %7, %TestClass** %2, align 8\n"
						  "  %8 = load %TestClass*, %TestClass** %2, align 8\n"
						  "  %9 = getelementptr inbounds %TestClass, %TestClass* %8, i32 0, i32 0\n"
						  "  %10 = load i8**, i8*** %9, align 8\n"
						  "  %11 = getelementptr i8*, i8** %10, i64 2\n"
						  "  %12 = load i8*, i8** %11, align 8\n"
						  "  %13 = bitcast i8* %12 to i32 (%error*, %TestClass*)*\n"
						  "  %14 = call i32 %13(%error* %4, %TestClass* %8)\n"
						  "  store i32 %14, i32* %3, align 4\n"
						  "  %15 = load %TestClass*, %TestClass** %2, align 8\n"
						  "  %16 = bitcast %TestClass* %15 to i8*\n"
						  "  tail call void @free(i8* %16)\n"
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
                          "define i32 @TestClass_F1a(%error* %0, %TestClass* %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca %TestClass*, align 8\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store %TestClass* %1, %TestClass** %3, align 8\n"
                          "  ret i32 1\n"
                          "}\n"
                          "\n"
                          "declare i8* @malloc(i64)\n"
                          "\n"
                          "declare void @free(i8*)\n"
                          );
    }

	TEST_F(CodeGenTest, CGClassStaticAttributes) {
        /**
         * Fly code:
         * class TestClass {
         *   static int a
         *   int getA() {
         *     return a
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
        ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        //  TestClass.a = 2
    	ASTType *TestClassType = CreateType(TestClass);
        ASTExprStmt * attrStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTValue *value2Expr = ASTBuilder::CreateNumberValue(SourceLocation(), "2");
        ASTIdentifier *attrIdent = ASTBuilder::CreateIdentifier(aAttribute);
        ASTBinaryOp *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, attrIdent, value2Expr);
        attrStmt->setExpr(AssignExpr);

		// Generate Code
		llvm::Module * M = Generate()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
						  "%TestClass = type { i8**, i32 }\n"
                          "\n"
						  "@error = external constant %error\n"
						  "@vtable.TestClass = constant [2 x i8*] [i8* null, i8* bitcast (void (%error*, %TestClass*)* @TestClass_F9TestClass to i8*)]\n" // TestClass.a
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

	TEST_F(CodeGenTest, CGClassStaticMethods) {
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
        ASTFunction *aFunc = ASTBuilder::CreateClassMethod(SourceLoc, TestClass, IntTypeRef,
                                                          "do", Modifiers, Params, aFuncBody);

        ASTReturnStmt * aFuncReturn = ASTBuilder::CreateReturnStmt(aFuncBody, SourceLoc);
        ASTValue *aFuncExpr = ASTBuilder::CreateNumberValue(SourceLocation(), "1");
        aFuncReturn->setExpr(aFuncExpr);

        // int main() {
        //  int a = TestClass.do()
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // int a = TestClass.do()
        ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        ASTDeclStmt *aDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, aVar);
    	ASTIdentifier *TestClassType = ASTBuilder::CreateIdentifier(SourceLoc, TestClass->getName());
        ASTCall *aCallExpr = ASTBuilder::CreateCall(SourceLoc, aFunc->getName(), Args, ASTCallKind::CALL_DIRECT, TestClassType);
        ASTIdentifier *aIdent = ASTBuilder::CreateIdentifier(aVar);
        ASTBinaryOp *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, aIdent, aCallExpr);
        aDeclStmt->setExpr(AssignExpr);

    	// Generate Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
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

	TEST_F(CodeGenTest, CGStructExtendsStruct) {
        /**
         * Fly code:
         * struct ParentStruct {
         *   int a
         * }
         * struct ChildStruct : ParentStruct {
         *   int b
         * }
         * void func() {
         *   ChildStruct child = new ChildStruct()
         * }
         */
        ASTModule *Module = CreateModule();

    	// struct BaseStruct {
    	// int a
    	// }
    	//
    	// struct TestStruct : BaseStruct {
    	// int a
    	// }
    	llvm::SmallVector<ASTType *, 4> BaseSuperClasses;
    	ASTClass *BaseStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct",
			TopModifiers, BaseSuperClasses);

    	// int a
    	ASTVar *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, BaseStruct, IntTypeRef, "a", TopModifiers);

    	llvm::SmallVector<ASTType *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(CreateType(BaseStruct));
    	ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct",
			TopModifiers, TestSuperClasses);
    	ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

    	// Generate Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
    					  "%BaseStruct = type { i32 }\n"
    					  "%TestStruct = type { %BaseStruct, i32 }\n"
    					  "\n"
						  "@error = external constant %error\n"
						  "\n"
						  "define %BaseStruct* @BaseStruct.init_ctor(%BaseStruct* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %BaseStruct*, align 8\n"
						  "  store %BaseStruct* %0, %BaseStruct** %1, align 8\n"
						  "  %2 = load %BaseStruct*, %BaseStruct** %1, align 8\n"
						  "  %3 = getelementptr inbounds %BaseStruct, %BaseStruct* %2, i32 0, i32 0\n"
						  "  store i32 0, i32* %3, align 4\n"
						  "  ret %BaseStruct* %2\n"
						  "}\n"
						  "\n"
						  "define %TestStruct* @TestStruct.init_ctor(%TestStruct* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %TestStruct*, align 8\n"
						  "  store %TestStruct* %0, %TestStruct** %1, align 8\n"
						  "  %2 = load %TestStruct*, %TestStruct** %1, align 8\n"
						  "  %3 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 1\n"
						  "  store i32 0, i32* %3, align 4\n"
						  "  ret %TestStruct* %2\n"
						  "}\n"
						  );
    }

	TEST_F(CodeGenTest, CGStructExtendsStructs) {
        /**
         * Fly code:
         * struct BaseStruct {
         *   int a
         * }
         * struct MiddleStruct : BaseStruct {
         *   int b
         * }
         * struct ChildStruct : MiddleStruct {
         *   int c
         * }
         * void func() {
         *   ChildStruct child = new ChildStruct()
         * }
         */
        ASTModule *Module = CreateModule();

    	// struct BaseStruct {
    	// int a
    	// }
    	// struct BaseStruct2 {
    	// int b
    	// }
    	//
    	// struct TestStruct : BaseStruct, BaseStruct2 {
    	// }

    	llvm::SmallVector<ASTType *, 4> BaseSuperClasses;

    	// BaseStruct
    	ASTClass *BaseStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTVar *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, BaseStruct, IntTypeRef, "a", TopModifiers);

    	// BaseStruct2
    	ASTClass *BaseStruct2 = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct2",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTVar *bAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, BaseStruct2, IntTypeRef, "b", TopModifiers);

    	// TestStruct
    	llvm::SmallVector<ASTType *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(CreateType(BaseStruct));
    	TestSuperClasses.push_back(CreateType(BaseStruct2));
    	ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct",
			TopModifiers, TestSuperClasses);

    	// Generate Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
					      "%BaseStruct = type { i32 }\n"
					      "%TestStruct = type { %BaseStruct, %BaseStruct2, i32, i32 }\n"
    					  "%BaseStruct2 = type { i32 }\n"
    					  "\n"
						  "@error = external constant %error\n"
						  "\n"
						  "define %BaseStruct* @BaseStruct.init_ctor(%BaseStruct* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %BaseStruct*, align 8\n"
						  "  store %BaseStruct* %0, %BaseStruct** %1, align 8\n"
						  "  %2 = load %BaseStruct*, %BaseStruct** %1, align 8\n"
						  "  %3 = getelementptr inbounds %BaseStruct, %BaseStruct* %2, i32 0, i32 0\n"
						  "  store i32 0, i32* %3, align 4\n"
						  "  ret %BaseStruct* %2\n"
						  "}\n"
						  "\n"
						  "define %TestStruct* @TestStruct.init_ctor(%TestStruct* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %TestStruct*, align 8\n"
						  "  store %TestStruct* %0, %TestStruct** %1, align 8\n"
						  "  %2 = load %TestStruct*, %TestStruct** %1, align 8\n"
						  "  %3 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 2\n"
						  "  store i32 0, i32* %3, align 4\n"
						  "  %4 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 3\n"
						  "  store i32 0, i32* %4, align 4\n"
						  "  ret %TestStruct* %2\n"
						  "}\n"
						  "\n"
						  "define %BaseStruct2* @BaseStruct2.init_ctor(%BaseStruct2* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %BaseStruct2*, align 8\n"
						  "  store %BaseStruct2* %0, %BaseStruct2** %1, align 8\n"
						  "  %2 = load %BaseStruct2*, %BaseStruct2** %1, align 8\n"
						  "  %3 = getelementptr inbounds %BaseStruct2, %BaseStruct2* %2, i32 0, i32 0\n"
						  "  store i32 0, i32* %3, align 4\n"
						  "  ret %BaseStruct2* %2\n"
						  "}\n"
						  );
    }

	TEST_F(CodeGenTest, CGClassExtendsStruct) {
        /**
         * Fly code:
         * struct BaseStruct {
         *   int a
         * }
         * class TestClass : BaseStruct {
         *   int b
         * }
         * void func() {
         *   TestClass test = new TestClass()
         *   delete test
         * }
         */
    	ASTModule *Module = CreateModule();

    	// struct BaseStruct {
    	//   int a
    	// }
    	//
    	// class TestClass : BaseStruct {
    	//   TestClass() {
    	//     this.a = 1
    	//   }
    	// }

    	// func() {
    	//    new TestClass()
    	// }

    	// struct BaseStruct
    	llvm::SmallVector<ASTType *, 4> BaseSuperClasses;
    	ASTClass *BaseStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTAttribute *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, BaseStruct, IntTypeRef, "a", TopModifiers);

    	// class TestClass
    	llvm::SmallVector<ASTType *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(CreateType(BaseStruct));
    	ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopModifiers, TestSuperClasses);

    	//   TestClass() {
    	//     this.a = 1
    	//   }
    	ASTBlockStmt *ConstructorBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTIdentifier * this_a = ASTBuilder::CreateIdentifier(aAttribute->getLocation(), aAttribute->getName(), ASTBuilder::CreateIdentifier(SourceLoc, "this"));
    	ASTExprStmt * assignThis_a = ASTBuilder::CreateExprStmt(ConstructorBody, SourceLoc);
    	ASTNumberValue *value1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
    	ASTBinaryOp *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, this_a, value1);
    	assignThis_a->setExpr(AssignExpr);
    	ASTFunction *ConstructorMethod = ASTBuilder::CreateClassMethod(SourceLoc, TestClass, VoidTypeRef,
															   "TestClass", TopModifiers, Params, ConstructorBody);



    	// func() { new TestClass() }
    	ASTBlockStmt *MainBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);
    	ASTExprStmt * testNewStmt = ASTBuilder::CreateExprStmt(MainBody, SourceLoc);
    	testNewStmt->setExpr(ConstructorCall);
    	ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, MainBody);

    	// Generate Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n"
    					  "%error = type { i8, i32, i8* }\n"
						  "%TestClass = type { i8**, %BaseStruct, i32 }\n"
						  "%BaseStruct = type { i32 }\n"
						  "\n"
						  "@error = external constant %error\n"
						  "@vtable.TestClass = constant [2 x i8*] [i8* null, i8* bitcast (void (%error*, %TestClass*)* @TestClass_F9TestClass to i8*)]\n"
						  "\n"
						  "define void @_F4func(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  %2 = load %error*, %error** %1, align 8\n"
						  "  %3 = call i8* @malloc(i64 ptrtoint (%TestClass* getelementptr (%TestClass, %TestClass* null, i32 1) to i64))\n"
						  "  %4 = bitcast i8* %3 to %TestClass*\n"
						  "  %5 = call %TestClass* @TestClass.init_ctor(%TestClass* %4)\n"
						  "  call void @TestClass_F9TestClass(%error* %2, %TestClass* %5)\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
						  "define %BaseStruct* @BaseStruct.init_ctor(%BaseStruct* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %BaseStruct*, align 8\n"
						  "  store %BaseStruct* %0, %BaseStruct** %1, align 8\n"
						  "  %2 = load %BaseStruct*, %BaseStruct** %1, align 8\n"
						  "  %3 = getelementptr inbounds %BaseStruct, %BaseStruct* %2, i32 0, i32 0\n"
						  "  store i32 0, i32* %3, align 4\n"
						  "  ret %BaseStruct* %2\n"
						  "}\n"
						  "\n"
						  "define %TestClass* @TestClass.init_ctor(%TestClass* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %TestClass*, align 8\n"
						  "  store %TestClass* %0, %TestClass** %1, align 8\n"
						  "  %2 = load %TestClass*, %TestClass** %1, align 8\n"
						  "  %3 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 0\n"
						  "  store i8** getelementptr inbounds ([2 x i8*], [2 x i8*]* @vtable.TestClass, i32 0, i32 0), i8*** %3, align 8\n"
						  "  %4 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 2\n"
						  "  store i32 0, i32* %4, align 4\n"
						  "  ret %TestClass* %2\n"
						  "}\n"
						  "\n"
						  "define void @TestClass_F9TestClass(%error* %0, %TestClass* %1) {\n"
						  "entry:\n"
						  "  %2 = alloca %error*, align 8\n"
						  "  %3 = alloca %TestClass*, align 8\n"
						  "  store %error* %0, %error** %2, align 8\n"
						  "  store %TestClass* %1, %TestClass** %3, align 8\n"
						  "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
						  "  %5 = getelementptr inbounds %TestClass, %TestClass* %4, i32 0, i32 2\n"
						  "  store i32 1, i32* %5, align 4\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
						  "declare i8* @malloc(i64)\n"
						  );
    }

	TEST_F(CodeGenTest, CGClassExtendsStructs) {
        /**
         * Fly code:
         * struct BaseStruct {
         *   int a
         * }
         * struct MiddleStruct : BaseStruct {
         *   int b
         * }
         * class TestClass : MiddleStruct {
         *   int c
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
    	//
    	// struct BaseStruct2 {
    	// int a
    	// int b
    	// }
    	//
    	// class TestClass : BaseStruct, BaseStruct2 {
    	// int a
    	// }
    	llvm::SmallVector<ASTType *, 4> BaseSuperClasses;
    	ASTClass *BaseStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTVar *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, BaseStruct, IntTypeRef, "a", TopModifiers);

    	ASTClass *BaseStruct2 = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct2",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTVar *aAttribute2 = ASTBuilder::CreateClassAttribute(SourceLoc, BaseStruct2, IntTypeRef, "a", TopModifiers);
    	// int b
    	ASTVar *bAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, BaseStruct2, IntTypeRef, "b", TopModifiers);

    	// class TestClass
    	llvm::SmallVector<ASTType *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(CreateType(BaseStruct));
    	TestSuperClasses.push_back(CreateType(BaseStruct2));
    	ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopModifiers, TestSuperClasses);
    	// int a
    	ASTVar *aAttribute3 = ASTBuilder::CreateClassAttribute(SourceLoc, TestClass, IntTypeRef, "a", TopModifiers);

    	// Generate Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
						  "%BaseStruct = type { i32 }\n"
						  "%BaseStruct2 = type { i32, i32 }\n"
						  "%TestClass = type { i8**, %BaseStruct, %BaseStruct2, i32, i32 }\n"
						  "\n"
						  "@error = external constant %error\n"
						  "@vtable.TestClass = constant [2 x i8*] [i8* null, i8* bitcast (void (%error*, %TestClass*)* @TestClass_F9TestClass to i8*)]\n"
						  "\n"
						  "define %BaseStruct* @BaseStruct.init_ctor(%BaseStruct* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %BaseStruct*, align 8\n"
						  "  store %BaseStruct* %0, %BaseStruct** %1, align 8\n"
						  "  %2 = load %BaseStruct*, %BaseStruct** %1, align 8\n"
						  "  %3 = getelementptr inbounds %BaseStruct, %BaseStruct* %2, i32 0, i32 0\n"
						  "  store i32 0, i32* %3, align 4\n"
						  "  ret %BaseStruct* %2\n"
						  "}\n"
						  "\n"
						  "define %BaseStruct2* @BaseStruct2.init_ctor(%BaseStruct2* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %BaseStruct2*, align 8\n"
						  "  store %BaseStruct2* %0, %BaseStruct2** %1, align 8\n"
						  "  %2 = load %BaseStruct2*, %BaseStruct2** %1, align 8\n"
						  "  %3 = getelementptr inbounds %BaseStruct2, %BaseStruct2* %2, i32 0, i32 0\n"
						  "  store i32 0, i32* %3, align 4\n"
						  "  %4 = getelementptr inbounds %BaseStruct2, %BaseStruct2* %2, i32 0, i32 1\n"
						  "  store i32 0, i32* %4, align 4\n"
						  "  ret %BaseStruct2* %2\n"
						  "}\n"
						  "\n"
						  "define %TestClass* @TestClass.init_ctor(%TestClass* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %TestClass*, align 8\n"
						  "  store %TestClass* %0, %TestClass** %1, align 8\n"
						  "  %2 = load %TestClass*, %TestClass** %1, align 8\n"
						  "  %3 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 0\n"
						  "  store i8** getelementptr inbounds ([2 x i8*], [2 x i8*]* @vtable.TestClass, i32 0, i32 0), i8*** %3, align 8\n"
						  "  %4 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 3\n"
						  "  store i32 0, i32* %4, align 4\n"
						  "  %5 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 4\n"
						  "  store i32 0, i32* %5, align 4\n"
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

	TEST_F(CodeGenTest, CGClassExtendsClass) {
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
    	// int a
    	// void do() {}
    	// }
    	//
    	// class TestClass : BaseClass {
    	//	void undo() {}
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
    	ASTFunction *Do = ASTBuilder::CreateClassMethod(SourceLoc, BaseClass, VoidTypeRef, "do", TopModifiers, Params, DoBody);

    	// TestClass
    	llvm::SmallVector<ASTType *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(CreateType(BaseClass));
    	ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopModifiers, TestSuperClasses);
    	// void undo()
    	ASTBlockStmt *UndoBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *Undo = ASTBuilder::CreateClassMethod(SourceLoc, TestClass, VoidTypeRef, "undo", TopModifiers, Params, UndoBody);

    	// func() { BaseClass a = new TestClass() }
    	ASTBlockStmt *MainBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);
    	ASTType *Base = CreateType(BaseClass);
    	ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(MainBody, SourceLoc, Base, "a", EmptyModifiers);
    	ASTDeclStmt *aDeclStmt = ASTBuilder::CreateDeclStmt(MainBody, SourceLoc, aVar);
    	ASTIdentifier *aIdent = ASTBuilder::CreateIdentifier(aVar);
    	ASTBinaryOp *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, aIdent, ConstructorCall);
    	aDeclStmt->setExpr(AssignExpr);
    	ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, MainBody);

    	// Generate Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M);

    	// TODO:
    	// Missing step: ctors must store the vtable pointer into the object.
    	// Derived ctor should call base ctor
	    // Hardcoded object size should be replaced with sizeof computed in IR.
	    // Offset - to - top is trivial now(always 0), but will matter with multiple inheritance

	    EXPECT_EQ(output, "\n"
						  "%error = type { i8, i32, i8* }\n"
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
						  "  store %TestClass* %6, %BaseClass** %2, align 8\n"
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

    TEST_F(CodeGenTest, CGClassExtendsClasses) {
        /**
         * Fly code:
         * class BaseClass {
         *   int a
         *   void do() {
         *   }
         * }
         * class BaseClass2 {
         *   int b
         *   void do() {
         *   }
         *   void undo() {
         *   }
         * }
         * class TestClass : BaseClass, BaseClass2 {
         *   void call() {
         *   }
         * }
         * void func() {
         *   TestClass test = new TestClass()
         *   delete test
         * }
         */
        ASTModule *Module = CreateModule();

    	// class BaseClass {
    	//   int a
    	//   void do() {}
    	// }
    	//
    	// class BaseClass2 {
    	//   int b
    	//   void do() {}
    	//   void undo() {}
    	// }
    	//
    	// class TestClass : BaseClass, BaseClass2 {
    	//   void foo() {
    	//		BaseClass.do()
    	//   }
    	// }

    	// BaseClass
    	llvm::SmallVector<ASTType *, 4> BaseSuperClasses;
    	ASTClass *BaseClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "BaseClass",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTVar *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, BaseClass, IntTypeRef, "a", TopModifiers);
    	// void do()
    	ASTBlockStmt *DoBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *BaseClass_do = ASTBuilder::CreateClassMethod(SourceLoc, BaseClass, VoidTypeRef, "do", TopModifiers, Params, DoBody);

    	// BaseClass2
    	llvm::SmallVector<ASTType *, 4> Base2SuperClasses;
    	ASTClass *BaseClass2 = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "BaseClass2",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTVar *bAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, BaseClass2, IntTypeRef, "b", TopModifiers);
    	// void do()
    	ASTBlockStmt *DoBody2 = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *BaseClass2_do = ASTBuilder::CreateClassMethod(SourceLoc, BaseClass2, VoidTypeRef, "do", TopModifiers, Params, DoBody2);
    	// void undo()
    	ASTBlockStmt *UndoBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *BaseClass2_undo = ASTBuilder::CreateClassMethod(SourceLoc, BaseClass2, VoidTypeRef, "undo", TopModifiers, Params, UndoBody);

    	// TestClass
    	llvm::SmallVector<ASTType *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(CreateType(BaseClass));
    	TestSuperClasses.push_back(CreateType(BaseClass2));
    	ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopModifiers, TestSuperClasses);

    	// void foo() {
    	//    BaseClass.do()
    	// }
    	ASTBlockStmt *DoBody3 = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTCall *aCallExpr = ASTBuilder::CreateCall(SourceLoc, BaseClass_do->getName(), Args, ASTCallKind::CALL_DIRECT,
    		ASTBuilder::CreateIdentifier(BaseClass->getLocation(), BaseClass->getName()));
    	ASTExprStmt * aStmt = ASTBuilder::CreateExprStmt(DoBody3, SourceLoc);
    	aStmt->setExpr(aCallExpr);
    	ASTFunction *TestClass_do = ASTBuilder::CreateClassMethod(SourceLoc, TestClass, VoidTypeRef, "foo", TopModifiers, Params, DoBody3);

    	// Generate Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n"
    					  "%error = type { i8, i32, i8* }\n"
    					  "%BaseClass2 = type { i8**, i32 }\n"
    					  "%BaseClass = type { i8**, i32 }\n"
						  "%TestClass = type { i8**, %BaseClass, %BaseClass2, i32, i32 }\n"
						  "\n"
						  "@error = external constant %error\n"
						  "@vtable.BaseClass2 = constant [4 x i8*] [i8* null, i8* bitcast (void (%error*, %BaseClass2*)* @BaseClass2_F10BaseClass2 to i8*), i8* bitcast (void (%error*, %BaseClass2*)* @BaseClass2_F2do to i8*), i8* bitcast (void (%error*, %BaseClass2*)* @BaseClass2_F4undo to i8*)]\n"
						  "@vtable.BaseClass = constant [3 x i8*] [i8* null, i8* bitcast (void (%error*, %BaseClass*)* @BaseClass_F9BaseClass to i8*), i8* bitcast (void (%error*, %BaseClass*)* @BaseClass_F2do to i8*)]\n"
						  "@vtable.TestClass = constant [3 x i8*] [i8* null, i8* bitcast (void (%error*, %TestClass*)* @TestClass_F9TestClass to i8*), i8* bitcast (void (%error*, %TestClass*)* @TestClass_F3foo to i8*)]\n"
						  "\n"
						  "define %BaseClass2* @BaseClass2.init_ctor(%BaseClass2* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %BaseClass2*, align 8\n"
						  "  store %BaseClass2* %0, %BaseClass2** %1, align 8\n"
						  "  %2 = load %BaseClass2*, %BaseClass2** %1, align 8\n"
						  "  %3 = getelementptr inbounds %BaseClass2, %BaseClass2* %2, i32 0, i32 0\n"
						  "  store i8** getelementptr inbounds ([4 x i8*], [4 x i8*]* @vtable.BaseClass2, i32 0, i32 0), i8*** %3, align 8\n"
						  "  %4 = getelementptr inbounds %BaseClass2, %BaseClass2* %2, i32 0, i32 1\n"
						  "  store i32 0, i32* %4, align 4\n"
						  "  ret %BaseClass2* %2\n"
						  "}\n"
						  "\n"
						  "define void @BaseClass2_F10BaseClass2(%error* %0, %BaseClass2* %1) {\n"
						  "entry:\n"
						  "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca %BaseClass2*, align 8\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store %BaseClass2* %1, %BaseClass2** %3, align 8\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
						  "define void @BaseClass2_F2do(%error* %0, %BaseClass2* %1) {\n"
						  "entry:\n"
						  "  %2 = alloca %error*, align 8\n"
						  "  %3 = alloca %BaseClass2*, align 8\n"
						  "  store %error* %0, %error** %2, align 8\n"
						  "  store %BaseClass2* %1, %BaseClass2** %3, align 8\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
						  "define void @BaseClass2_F4undo(%error* %0, %BaseClass2* %1) {\n"
						  "entry:\n"
						  "  %2 = alloca %error*, align 8\n"
						  "  %3 = alloca %BaseClass2*, align 8\n"
						  "  store %error* %0, %error** %2, align 8\n"
						  "  store %BaseClass2* %1, %BaseClass2** %3, align 8\n"
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
						  "  %7 = call %BaseClass2* @BaseClass2.init_ctor(%BaseClass2* %6)\n"
						  "  %8 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 3\n"
						  "  store i32 0, i32* %8, align 4\n"
						  "  %9 = getelementptr inbounds %TestClass, %TestClass* %2, i32 0, i32 4\n"
						  "  store i32 0, i32* %9, align 4\n"
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
						  "define void @TestClass_F3foo(%error* %0, %TestClass* %1) {\n"
						  "entry:\n"
						  "  %2 = alloca %error*, align 8\n"
						  "  %3 = alloca %TestClass*, align 8\n"
						  "  store %error* %0, %error** %2, align 8\n"
						  "  store %TestClass* %1, %TestClass** %3, align 8\n"
						  "  %4 = load %error*, %error** %2, align 8\n"
						  "  %5 = load %TestClass*, %TestClass** %3, align 8\n"
						  "  %6 = getelementptr inbounds %TestClass, %TestClass* %5, i32 0, i32 1\n"
						  "  %7 = getelementptr inbounds %BaseClass, %BaseClass* %6, i32 0, i32 0\n"
						  "  %8 = load i8**, i8*** %7, align 8\n"
						  "  %9 = getelementptr i8*, i8** %8, i64 2\n"
						  "  %10 = load i8*, i8** %9, align 8\n"
						  "  %11 = bitcast i8* %10 to void (%error*, %BaseClass*)*\n"
						  "  call void %11(%error* %4, %BaseClass* %6)\n"
						  "  ret void\n"
						  "}\n"
						  );
    }

	TEST_F(CodeGenTest, CGInterfaceExtendsInterfaces) {
        /**
         * Fly code:
         * interface BaseInterface {
         *   void do()
         * }
         * interface BaseInterface2 {
         *   void undo()
         * }
         * interface TestInterface : BaseInterface, BaseInterface2 {
         *   void call()
         * }
         */
        ASTModule *Module = CreateModule();

    	// interface BaseInterface {
    	//   void do()
    	// }
    	//
    	// interface BaseInterface2 {
    	//   void undo()
    	// }
    	//
    	// interface TestInterface : BaseInterface, BaseInterface2 {
    	// }
    	//
    	// class TestClass : TestInterface {
	    //	 void do() {}
    	//   void undo() {}
	    // }
    	//
	    // func() {
    	//    TestInterface b = new TestClass()
    	// }

    	// BaseInterface
    	llvm::SmallVector<ASTType *, 4> BaseInterfaces;
    	ASTClass *BaseInterface = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::INTERFACE,
    		"BaseInterface", TopModifiers, BaseInterfaces);
    	ASTBuilder::CreateClassMethod(SourceLoc, BaseInterface, VoidTypeRef, "do", TopModifiers, Params);


    	// BaseInterface2
    	llvm::SmallVector<ASTType *, 4> Base2Interfaces;
    	ASTClass *BaseInterface2 = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::INTERFACE,
    		"BaseInterface2", TopModifiers, Base2Interfaces);
    	ASTBuilder::CreateClassMethod(SourceLoc, BaseInterface2, VoidTypeRef, "undo", TopModifiers, Params);

    	llvm::SmallVector<ASTType *, 4> TestBaseInterfaces;
    	TestBaseInterfaces.push_back(CreateType(BaseInterface));
    	TestBaseInterfaces.push_back(CreateType(BaseInterface2));
    	ASTClass *TestInterface = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::INTERFACE,
    		"TestInterface", TopModifiers, TestBaseInterfaces);

    	// class TestClass
    	llvm::SmallVector<ASTType *, 4> TestBaseClasses;
    	TestBaseClasses.push_back(CreateType(TestInterface));
    	ASTClass *TestClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopModifiers, TestBaseClasses);

    	// void do()
    	ASTBlockStmt *DoBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTMethod *TestClass_do = ASTBuilder::CreateClassMethod(SourceLoc, TestClass, VoidTypeRef, "do", TopModifiers, Params, DoBody);

    	// void undo()
    	ASTBlockStmt *UndoBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTMethod *TestClass_undo = ASTBuilder::CreateClassMethod(SourceLoc, TestClass, VoidTypeRef, "undo", TopModifiers, Params, UndoBody);

    	// main() { new TestClass() }
    	ASTBlockStmt *MainBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestClass->getName(), Args, ASTCallKind::CALL_NEW);

    	ASTType *TestInterfaceTypeeRef = CreateType(TestInterface);
    	ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(MainBody, SourceLoc, TestInterfaceTypeeRef, "a", EmptyModifiers);
    	ASTDeclStmt *aDeclStmt = ASTBuilder::CreateDeclStmt(MainBody, SourceLoc, aVar);
    	ASTIdentifier *aIdent = ASTBuilder::CreateIdentifier(aVar);
    	ASTBinaryOp *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, aIdent, ConstructorCall);
    	aDeclStmt->setExpr(AssignExpr);
    	ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, MainBody);

    	// Generate Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n");
    }

	TEST_F(CodeGenTest, CGClassExtendsStructAndInterface) {
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
