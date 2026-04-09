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
        ASTAttribute *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

        // func() {
        //    new TestStruct()
        // }
        ASTBlockStmt *MainBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestStruct->getName(), Args, ASTCallKind::CALL_NEW);
    	ASTExprStmt *testNewStmt = ASTBuilder::CreateExprStmt(MainBody, SourceLoc);
    	testNewStmt->setExpr(ConstructorCall);
    	ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, MainBody);

		// Generate Code
		Generate();
		llvm::Module * M = getModules()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n"
                        "%error = type { i32, ptr, ptr }\n"
                        "%TestStruct = type { i32 }\n"
                        "\n"
                        "@error = external constant %error\n"
                        "\n"
                        "define ptr @TestStruct.init_ctor(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %2 = load ptr, ptr %1, align 8\n"
                        "  %3 = getelementptr inbounds %TestStruct, ptr %2, i32 0, i32 0\n"
                        "  store i32 0, ptr %3, align 4\n"
                        "  ret ptr %2\n"
                        "}\n"
                        "\n"
                        "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %2 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%TestStruct, ptr null, i32 1) to i64))\n"
                        "  %3 = call ptr @TestStruct.init_ctor(ptr %2)\n"
                        "  ret void\n"
                        "}\n"
                        "\n"
                        "declare ptr @malloc(i64)\n");
    }

TEST_F(CodeGenTest, CGStructAssignVar) {
        /**
         * Fly code:
         * struct TestStruct {
         *   int a
         * }
         * void func() {
         *   TestStruct test = new TestStruct()
         *   int x = test.a
         *   test.a = 2
         * }
         */
        ASTModule *Module = CreateModule();

        // struct TestStruct {
        //   int a
        // }
    	llvm::SmallVector<ASTType *, 4> SuperClasses;
    	ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct", TopModifiers, SuperClasses);
    	ASTAttribute *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

        // void func() {
        //  TestStruct test = new TestStruct()
        //  int x = test.a
        //  test.a = 2
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // TestStruct test = new TestStruct()
        ASTType *TestClassType = CreateType(TestStruct);
        ASTLocalVar *TestVar = ASTBuilder::CreateLocalVar(SourceLoc, TestClassType, "test", EmptyModifiers);
        ASTDeclStmt *TestDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TestVar);
        ASTIdentifier *testIdent = ASTBuilder::CreateIdentifier(TestVar);
        ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestStruct->getName(), Args, ASTCallKind::CALL_NEW);
        ASTBinary *AssignExpr1 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, testIdent, ConstructorCall);
        TestDeclStmt->setExpr(AssignExpr1);

        // int x = test.a
        ASTLocalVar *xVar = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "x", EmptyModifiers);
        ASTDeclStmt *xDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, xVar);
        ASTMember *test_aRead = ASTBuilder::CreateMember(SourceLoc, aAttribute->getName(), ASTBuilder::CreateIdentifier(TestVar));
        ASTIdentifier *xIdent = ASTBuilder::CreateIdentifier(xVar);
        ASTBinary *AssignExpr2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, xIdent, test_aRead);
        xDeclStmt->setExpr(AssignExpr2);

        // test.a = 2
        ASTMember *test_aWrite = ASTBuilder::CreateMember(SourceLoc, aAttribute->getName(), ASTBuilder::CreateIdentifier(TestVar));
        ASTExprStmt *attrStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTValue *value2 = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        ASTBinary *AssignExpr3 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, test_aWrite, value2);
        attrStmt->setExpr(AssignExpr3);

		// Generate Code
		Generate();
		llvm::Module * M = getModules()[0];
		std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n"
                        "%error = type { i32, ptr, ptr }\n"
                        "%TestStruct = type { i32 }\n"
                        "\n"
                        "@error = external constant %error\n"
                        "\n"
                        "define ptr @TestStruct.init_ctor(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %2 = load ptr, ptr %1, align 8\n"
                        "  %3 = getelementptr inbounds %TestStruct, ptr %2, i32 0, i32 0\n"
                        "  store i32 0, ptr %3, align 4\n"
                        "  ret ptr %2\n"
                        "}\n"
                        "\n"
                        "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca ptr, align 8\n"
                        "  %3 = alloca i32, align 4\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %4 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%TestStruct, ptr null, i32 1) to i64))\n"
                        "  %5 = call ptr @TestStruct.init_ctor(ptr %4)\n"
                        "  store ptr %5, ptr %2, align 8\n"
                        "  %6 = load %TestStruct, ptr %2, align 4\n"
                        "  %7 = getelementptr inbounds %TestStruct, %TestStruct %6, i32 0, i32 0\n"
                        "  %8 = load i32, %TestStruct %7, align 4\n"
                        "  store i32 %8, ptr %3, align 4\n"
                        "  %9 = getelementptr inbounds %TestStruct, %TestStruct %6, i32 0, i32 0\n"
                        "  store i32 2, %TestStruct %9, align 4\n"
                        "  ret void\n"
                        "}\n"
                        "\n"
                        "declare ptr @malloc(i64)\n");
    }

	TEST_F(CodeGenTest, CGStructValueAssign) {
        /**
         * Fly code:
         * struct TestStruct {
         *   int a
         * }
         * void func() {
         *   TestStruct test = { a=1 }
         * }
         */
        ASTModule *Module = CreateModule();

        // struct TestStruct {
        //   int a
        // }
    	llvm::SmallVector<ASTType *, 4> SuperClasses;
    	ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct", TopModifiers, SuperClasses);
    	ASTAttribute *aAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

        // void func() {
        //  TestStruct test = { a=1 }
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // TestStruct test = {a=1}
        ASTType *TestClassType = CreateType(TestStruct);
        ASTLocalVar *TestVar = ASTBuilder::CreateLocalVar(SourceLoc, TestClassType, "test", EmptyModifiers);
        ASTDeclStmt *TestDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TestVar);
        ASTIdentifier *testIdent = ASTBuilder::CreateIdentifier(TestVar);

        // Create struct value {a=1}
        llvm::StringMap<ASTValue *> StructValues;
        StructValues.insert(std::make_pair("a", ASTBuilder::CreateNumberValue(SourceLoc, "1")));
        ASTStructValue *structVal = ASTBuilder::CreateStructValue(SourceLoc, std::move(StructValues));

        ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, testIdent, structVal);
        TestDeclStmt->setExpr(AssignExpr);

		// Generate Code
		Generate();
		llvm::Module * M = getModules()[0];
		std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n"
                        "%error = type { i32, ptr, ptr }\n"
                        "%TestStruct = type { i32 }\n"
                        "\n"
                        "@error = external constant %error\n"
                        "\n"
                        "define ptr @TestStruct.init_ctor(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %2 = load ptr, ptr %1, align 8\n"
                        "  %3 = getelementptr inbounds %TestStruct, ptr %2, i32 0, i32 0\n"
                        "  store i32 0, ptr %3, align 4\n"
                        "  ret ptr %2\n"
                        "}\n"
                        "\n"
                        "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %3 = alloca %TestStruct, align 8\n"
                        "  call void @llvm.memset.p0.i64(ptr %3, i8 0, i64 4, i1 false)\n"
                        "  %4 = getelementptr inbounds nuw %TestStruct, ptr %3, i32 0, i32 0\n"
                        "  store i32 1, ptr %4, align 4\n"
                        "  store ptr %3, ptr %2, align 8\n"
                        "  ret void\n"
                        "}\n"
                        "\n"
                        "; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)\n"
                        "declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #0\n"
                        "\n"
                        "attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: write) }\n");
    }


	TEST_F(CodeGenTest, DISABLED_CGStructExtendsStruct) {
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
    	Generate();
		llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n%error = type { i32, i8*, i8* }\n"
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

	TEST_F(CodeGenTest, DISABLED_CGStructExtendsStructs) {
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
    	ASTVar *aAttribute2 = ASTBuilder::CreateClassAttribute(SourceLoc, BaseStruct2, IntTypeRef, "a", TopModifiers);
    	// int b
    	ASTVar *bAttribute = ASTBuilder::CreateClassAttribute(SourceLoc, BaseStruct2, IntTypeRef, "b", TopModifiers);

    	// TestStruct
    	llvm::SmallVector<ASTType *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(CreateType(BaseStruct));
    	TestSuperClasses.push_back(CreateType(BaseStruct2));
    	ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct",
			TopModifiers, TestSuperClasses);

    	// Generate Code
    	Generate();
		llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n%error = type { i32, i8*, i8* }\n"
					      "%BaseStruct = type { i32 }\n"
					      "%BaseStruct2 = type { i32, i32 }\n"
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
						  );
    }

} // anonymous namespace
