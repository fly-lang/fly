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
#include "Sema/SemaBuilderStmt.h"
#include "AST/ASTRef.h"
#include <Sema/SemaFunction.h>

namespace {

    using namespace fly;

    TEST_F(CodeGenTest, CGStruct) {
        ASTModule *Module = CreateModule();

        // struct TestStruct {
    	//  int a
    	// }
        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
        ASTClass *TestStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct", TopScopes, SuperClasses);
        ASTVar *aField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopScopes);

        // main() {
        //    new TestStruct()
        // }
        ASTBlockStmt *MainBody = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTCall *ConstructorCall = CreateCall(TestStruct->getName(), Args, ASTCallKind::CALL_NEW);
    	ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
    	SemaBuilderStmt *testNewStmt = getASTBuilder().CreateExprStmt(MainBody, SourceLoc);
    	testNewStmt->setExpr(NewExpr);
    	getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopScopes, Params, MainBody);

		// validate and resolve
		EXPECT_TRUE(S->Resolve());

		// Generate Code
		llvm::Module * M = Generate();
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n"
        				  "%error = type { i8, i32, i8* }\n"
						  "%TestStruct = type { i32 }\n"
        				  "\n"
        				  "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %malloccall = tail call i8* @malloc(i64 4)\n"
                          "  %2 = bitcast i8* %malloccall to %TestStruct*\n"
                          "  call void @TestStruct_F10TestStruct(%TestStruct* %2)\n"
                          "  ret void\n"
                          "}\n"
                          "\n"
						  "define void @TestStruct_F10TestStruct(%TestStruct* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %TestStruct*, align 8\n"
						  "  store %TestStruct* %0, %TestStruct** %1, align 8\n"
						  "  %2 = load %TestStruct*, %TestStruct** %1, align 8\n"
						  "  %3 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 0\n"
						  "  store i32 0, i32* %3, align 4\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
						  "declare noalias i8* @malloc(i64)\n");
    }

TEST_F(CodeGenTest, CGStruct2) {
        ASTModule *Module = CreateModule();

        // struct TestStruct {
        //   int a
        //   int b
        //   const int c
        // }

        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
        ASTClass *TestStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct",
                                                   TopScopes, SuperClasses);
        ASTVar *aField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopScopes);
        // ASTVar *bField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "b", TopScopes);
        // ASTVar *cField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "c", TopScopes);

        // int func() {
        //  TestStruct test = new TestStruct();
        //  int x = test.a
        //  test.b = 2
        //  return 1
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopScopes, Params, Body);

        // TestStruct test = new TestStruct()
        ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestStruct);
        ASTVar *TestVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyScopes);
        ASTCall *ConstructorCall = CreateCall(TestStruct->getName(), Args, ASTCallKind::CALL_NEW);
        ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
        SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(Body, TestVar);
        testNewStmt->setExpr(NewExpr);

        // int x = test.a
        ASTVar *xVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntTypeRef, "x", EmptyScopes);
        ASTRef *Instance = getASTBuilder().CreateVarRef(TestVar);
        ASTRef *test_aVarRef = getASTBuilder().CreateVarRef(aField, Instance);
        ASTVarRefExpr *test_aRefExpr = getASTBuilder().CreateExpr(test_aVarRef);
        SemaBuilderStmt *xVarStmt = getASTBuilder().CreateAssignmentStmt(Body, xVar);
        xVarStmt->setExpr(test_aRefExpr);

		// validate and resolve
		EXPECT_TRUE(S->Resolve());

		// Generate Code
		llvm::Module * M = Generate();
		std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n"
    					  "%error = type { i8, i32, i8* }\n"
						  "%TestStruct = type { i32 }\n"
						  "\n"
                          "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca %TestStruct*, align 8\n"
                          "  %3 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %malloccall = tail call i8* @malloc(i64 4)\n"
                          "  %4 = bitcast i8* %malloccall to %TestStruct*\n"
                          "  call void @TestStruct_F10TestStruct(%TestStruct* %4)\n"
                          "  store %TestStruct* %4, %TestStruct** %2, align 8\n"
                          "  %5 = load %TestStruct*, %TestStruct** %2, align 8\n"
                          "  %6 = getelementptr inbounds %TestStruct, %TestStruct* %5, i32 0, i32 0\n"
                          "  %7 = load i32, i32* %6, align 4\n"
                          "  store i32 %7, i32* %3, align 4\n"
                          "  ret void\n"
                          "}\n"
                          "\n"
                          "define void @TestStruct_F10TestStruct(%TestStruct* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %TestStruct*, align 8\n"
						  "  store %TestStruct* %0, %TestStruct** %1, align 8\n"
						  "  %2 = load %TestStruct*, %TestStruct** %1, align 8\n"
						  "  %3 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 0\n"
						  "  store i32 0, i32* %3, align 4\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
                          "declare noalias i8* @malloc(i64)\n"
                          );
    }

    TEST_F(CodeGenTest, CGClassMethods) {
        ASTModule *Module = CreateModule();

        // TestClass {
        //   int a() { return 1 }
        // }
        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
        ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
                                                  TopScopes, SuperClasses);

        // int a() { return 1 }
        ASTBlockStmt *aFuncBody = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *aFunc = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, IntTypeRef,
                                                          "a", TopScopes, Params, aFuncBody);

        SemaBuilderStmt *aFuncReturn = getASTBuilder().CreateReturnStmt(aFuncBody, SourceLoc);
        ASTValueExpr *aFuncExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLocation(), "1"));
        aFuncReturn->setExpr(aFuncExpr);

        // int main() {
        //  TestClass test = new TestClass()
        //  int a = test.a()
        //  delete test
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopScopes, Params, Body);

        // TestClass test = new TestClass()
        ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestClass);
        ASTVar *TestVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyScopes);
        ASTCall *ConstructorCall = CreateCall(TestClass->getName(), Args, ASTCallKind::CALL_NEW);
        ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
        SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(Body, TestVar);
        testNewStmt->setExpr(NewExpr);

        // int a = test.a()
        ASTTypeRef *aType = aFunc->getReturnTypeRef();
        ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, aType, "a", EmptyScopes);
        ASTCallExpr *aCallExpr = getASTBuilder().CreateExpr(CreateCall(aFunc, Args, ASTCallKind::CALL_FUNCTION, getASTBuilder().CreateVarRef(TestVar)));
        SemaBuilderStmt *aStmt = getASTBuilder().CreateAssignmentStmt(Body, aVar);
        aStmt->setExpr(aCallExpr);

        // delete test
        ASTDeleteStmt *DeleteStmt = getASTBuilder().CreateDeleteStmt(Body, SourceLoc, getASTBuilder().CreateVarRef(TestVar));

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// Generate Code
    	llvm::Module * M = Generate();
    	std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
                          "%TestClass = type { %TestClass_vtable* }\n"
                          "%TestClass_vtable = type { i32 (%error*, %TestClass*)* }\n"
                          "\n"
                          "define void @_F4func(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  %2 = alloca %TestClass*, align 8\n"
						  "  %3 = alloca i32, align 4\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  %4 = load %error*, %error** %1, align 8\n"
						  "  %malloccall = tail call i8* @malloc(i64 8)\n"
						  "  %5 = bitcast i8* %malloccall to %TestClass*\n"
						  "  call void @TestClass_F9TestClass(%error* %4, %TestClass* %5)\n"
						  "  store %TestClass* %5, %TestClass** %2, align 8\n"
						  "  %6 = load %TestClass*, %TestClass** %2, align 8\n"
						  "  %7 = call i32 @TestClass_F1a(%error* %4, %TestClass* %6)\n"
						  "  store i32 %7, i32* %3, align 4\n"
						  "  %8 = load %TestClass*, %TestClass** %2, align 8\n"
						  "  %9 = bitcast %TestClass* %8 to i8*\n"
						  "  tail call void @free(i8* %9)\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
                          "define void @TestClass_F9TestClass(%error* %0, %TestClass* %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca %TestClass*, align 8\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store %TestClass* %1, %TestClass** %3, align 8\n"
                          "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
                          "  ret void\n"
                          "}\n"
                          "\n"
                          "define i32 @TestClass_F1a(%error* %0, %TestClass* %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca %TestClass*, align 8\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store %TestClass* %1, %TestClass** %3, align 8\n"
                          "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
                          "  ret i32 1\n"
                          "}\n"
                          "\n"
                          "declare noalias i8* @malloc(i64)\n"
                          "\n"
                          "declare void @free(i8*)\n"
                          );
    }

    TEST_F(CodeGenTest, CGClassAttributes) {
        ASTModule *Module = CreateModule();

        // TestClass {
        //   int a
        //   int getA() { return a }
        // }
        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
        ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass", TopScopes,
                                                  SuperClasses);

        // int a
        ASTVar *aAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, TestClass, IntTypeRef, "a", TopScopes);

        // int getA() { return a }
        ASTBlockStmt *MethodBody = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *getAMethod = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, IntTypeRef,
                                                               "getA", TopScopes, Params, MethodBody);

        SemaBuilderStmt *MethodReturn = getASTBuilder().CreateReturnStmt(MethodBody, SourceLoc);
        MethodReturn->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aAttribute)));

        // int main() {
        //  TestClass test = new TestClass()
        //  int x = test.getA()
        //  test.a = 2
        //  delete test
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopScopes, Params, Body);

        // TestClass test = new TestClass()
        ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestClass);
        ASTVar *TestVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyScopes);
        ASTCall *ConstructorCall = CreateCall(TestClass->getName(), Args, ASTCallKind::CALL_NEW);
        ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
        SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(Body, TestVar);
        testNewStmt->setExpr(NewExpr);

        // int x = test.getA()
        ASTTypeRef *xType = getAMethod->getReturnTypeRef();
        ASTVar *xVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, xType, "x", EmptyScopes);
        ASTCallExpr *xCallExpr = getASTBuilder().CreateExpr(CreateCall(getAMethod, Args, ASTCallKind::CALL_FUNCTION, getASTBuilder().CreateVarRef(TestVar)));
        SemaBuilderStmt *xStmt = getASTBuilder().CreateAssignmentStmt(Body, xVar);
        xStmt->setExpr(xCallExpr);

        //  test.a = 2
    	ASTRef *test_a = getASTBuilder().CreateVarRef(aAttribute, getASTBuilder().CreateVarRef(TestVar));
        SemaBuilderStmt *attrStmt = getASTBuilder().CreateAssignmentStmt(Body, test_a);
        ASTValueExpr *value2Expr = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLocation(), "2"));
        attrStmt->setExpr(value2Expr);

        // delete test
        getASTBuilder().CreateDeleteStmt(Body, SourceLoc, getASTBuilder().CreateVarRef(TestVar));

		// validate and resolve
		EXPECT_TRUE(S->Resolve());

		// Generate Code
		llvm::Module * M = Generate();
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
                          "%TestClass = type { %TestClass_vtable*, i32 }\n"
                          "%TestClass_vtable = type { i32 (%error*, %TestClass*)* }\n"
                          "\n"
						  "define void @_F4func(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  %2 = alloca %TestClass*, align 8\n"
						  "  %3 = alloca i32, align 4\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  %4 = load %error*, %error** %1, align 8\n"
						  // TestClass test = new TestClass()
						  "  %malloccall = tail call i8* @malloc(i64 16)\n"
						  "  %5 = bitcast i8* %malloccall to %TestClass*\n"
						  "  call void @TestClass_F9TestClass(%error* %4, %TestClass* %5)\n"
						  "  store %TestClass* %5, %TestClass** %2, align 8\n"
						  "  %6 = load %TestClass*, %TestClass** %2, align 8\n"
						  // int a = test.a()
						  "  %7 = call i32 @TestClass_F4getA(%error* %4, %TestClass* %6)\n"
						  "  store i32 %7, i32* %3, align 4\n"
						  // test.a = 2
						  "  %8 = getelementptr inbounds %TestClass, %TestClass* %6, i32 0, i32 1\n"
						  "  store i32 2, i32* %8, align 4\n"
						  // delete test
						  "  %9 = load %TestClass*, %TestClass** %2, align 8\n"
						  "  %10 = bitcast %TestClass* %9 to i8*\n"
						  "  tail call void @free(i8* %10)\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
                          "define void @TestClass_F9TestClass(%error* %0, %TestClass* %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca %TestClass*, align 8\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store %TestClass* %1, %TestClass** %3, align 8\n"
                          "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
                          "  %5 = getelementptr inbounds %TestClass, %TestClass* %4, i32 0, i32 1\n"
                          "  store i32 0, i32* %5, align 4\n"
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
                          "declare noalias i8* @malloc(i64)\n"
                          "\n"
                          "declare void @free(i8*)\n"
        );
    }

	TEST_F(CodeGenTest, CGStructExtendsStruct) {
    	ASTModule *Module = CreateModule();

    	// struct BaseStruct {
    	// int a
    	// }
    	//
    	// struct TestStruct : BaseStruct {
    	// }
    	llvm::SmallVector<ASTTypeRef *, 4> BaseSuperClasses;
    	ASTClass *BaseStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct",
			TopScopes, BaseSuperClasses);
    	llvm::SmallVector<ASTTypeRef *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseStruct));
    	ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct",
			TopScopes, TestSuperClasses);

    }

	TEST_F(CodeGenTest, CGStructExtendsStructs) {
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
    	llvm::SmallVector<ASTTypeRef *, 4> BaseSuperClasses;
    	ASTClass *BaseStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct",
			TopScopes, BaseSuperClasses);
    	ASTClass *BaseStruct2 = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct2",
			TopScopes, BaseSuperClasses);
    	llvm::SmallVector<ASTTypeRef *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseStruct));
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseStruct2));
    	ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct",
			TopScopes, TestSuperClasses);
    }

	TEST_F(CodeGenTest, CGClassExtendsStruct) {
    	ASTModule *Module = CreateModule();

    	// struct BaseStruct {
    	// int a
    	// }
    	//
    	// class TestClass : BaseStruct {
    	// }
    	llvm::SmallVector<ASTTypeRef *, 4> BaseSuperClasses;
    	ASTClass *BaseStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "BaseStruct",
			TopScopes, BaseSuperClasses);
    	llvm::SmallVector<ASTTypeRef *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseStruct));
    	ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestClass",
			TopScopes, TestSuperClasses);

    }

	TEST_F(CodeGenTest, CGClassExtendsStructs) {
	    ASTModule *Module = CreateModule();

    	// struct BaseStruct {
    	// int a
    	// }
    	// struct BaseStruct2 {
    	// int b
    	// }
    	//
    	// class TestClass : BaseStruct, BaseStruct2 {
    	// }
    	llvm::SmallVector<ASTTypeRef *, 4> BaseSuperClasses;
    	ASTClass *BaseStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct",
			TopScopes, BaseSuperClasses);
    	ASTClass *BaseStruct2 = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct2",
			TopScopes, BaseSuperClasses);
    	llvm::SmallVector<ASTTypeRef *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseStruct));
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseStruct2));
    	ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopScopes, TestSuperClasses);
    }

	TEST_F(CodeGenTest, CGClassExtendsClass) {
    	ASTModule *Module = CreateModule();

    	// class BaseClass {
    	// int a
    	// void do() {}
    	// }
    	//
    	// class TestClass : BaseClass {
    	//	void undo() {
    	//		a = 2
    	//	    do()
    	//  }
    	// }
    	llvm::SmallVector<ASTTypeRef *, 4> BaseSuperClasses;
    	ASTClass *BaseClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopScopes, BaseSuperClasses);
    	llvm::SmallVector<ASTTypeRef *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseClass));
    	ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopScopes, TestSuperClasses);
    }

    TEST_F(CodeGenTest, CGClassExtendsClasses) {
    	ASTModule *Module = CreateModule();

    	// class BaseClass {
    	// int a
    	// void do() {}
    	// }
    	// class BaseClass2 {
    	// int b
    	// void do() {}
    	// }
    	//
    	// class TestClass : BaseClass, BaseClass2 {
    	//	void undo() {
    	//		a = 2
    	//	    do()
    	//  }
    	// }
    }

	TEST_F(CodeGenTest, CGInterfaceExtendsInterface) {
    	ASTModule *Module = CreateModule();

    	// interface BaseInterface {
    	// void do()
    	// }
    	// interface BaseInterface2 {
    	// void undo()
    	// }
    	//
    	// interface TestInterface : BaseInterface, BaseInterface2 {
    	// }
    }

	TEST_F(CodeGenTest, CGClassExtendsInterface) {
    	ASTModule *Module = CreateModule();

    	// interface BaseInterface {
    	// int a
    	// void do() {}
    	// }
    	//
    	// class TestClass : BaseInterface {
    	//	void undo() {
    	//		a = 2
    	//	    do()
    	//  }
    	// }
    	llvm::SmallVector<ASTTypeRef *, 4> BaseSuperClasses;
    	ASTClass *BaseClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopScopes, BaseSuperClasses);
    	llvm::SmallVector<ASTTypeRef *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseClass));
    	ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopScopes, TestSuperClasses);
    }

	TEST_F(CodeGenTest, CGClassExtendsAll) {
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
