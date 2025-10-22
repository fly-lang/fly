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
        ASTClass *TestStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct", TopModifiers, SuperClasses);
        ASTVar *aField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

        // func() {
        //    new TestStruct()
        // }
        ASTBlockStmt *MainBody = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTCall *ConstructorCall = CreateCall(TestStruct->getName(), Args, ASTCallKind::CALL_NEW);
    	ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
    	SemaBuilderStmt *testNewStmt = getASTBuilder().CreateExprStmt(MainBody, SourceLoc);
    	testNewStmt->setExpr(NewExpr);
    	getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, MainBody);

		// validate and resolve
		EXPECT_TRUE(S->Resolve());

		// Generate Code
		llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // struct TestStruct {
        //   int a
        //   int b
        //   const int c
        // }

        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
        ASTClass *TestStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct",
                                                   TopModifiers, SuperClasses);
        ASTVar *aField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);
        // ASTVar *bField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "b", TopModifiers);
        // ASTVar *cField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "c", TopModifiers);

        // int func() {
        //  TestStruct test = new TestStruct();
        //  int x = test.a
        //  test.b = 2
        //  return 1
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // TestStruct test = new TestStruct()
        ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestStruct);
        ASTVar *TestVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyModifiers);
        ASTCall *ConstructorCall = CreateCall(TestStruct->getName(), Args, ASTCallKind::CALL_NEW);
        ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
        SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(Body, TestVar);
        testNewStmt->setExpr(NewExpr);

        // int x = test.a
        ASTVar *xVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntTypeRef, "x", EmptyModifiers);
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
        ASTModule *Module = CreateModule();

        // TestClass {
        //   int a
        // }
        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
        ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass", TopModifiers,
                                                  SuperClasses);

        // int a
        ASTVar *aAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, TestClass, IntTypeRef, "a", TopModifiers);

        // int main() {
        //  TestClass test = new TestClass()
        //  test.a = 2
        //  delete test
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // TestClass test = new TestClass()
        ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestClass);
        ASTVar *TestVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyModifiers);
        ASTCall *ConstructorCall = CreateCall(TestClass->getName(), Args, ASTCallKind::CALL_NEW);
        ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
        SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(Body, TestVar);
        testNewStmt->setExpr(NewExpr);

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
        ASTModule *Module = CreateModule();

        // TestClass {
        //   int a
        //   int getA() { return a }
        // }
        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
        ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass", TopModifiers,
                                                  SuperClasses);

        // int a
        ASTVar *aAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, TestClass, IntTypeRef, "a", TopModifiers);

        // int getA() { return a }
        ASTBlockStmt *MethodBody = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *getAMethod = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, IntTypeRef,
                                                               "getA", TopModifiers, Params, MethodBody);

        SemaBuilderStmt *MethodReturn = getASTBuilder().CreateReturnStmt(MethodBody, SourceLoc);
        MethodReturn->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aAttribute)));

        // int main() {
        //  TestClass test = new TestClass()
        //  int x = test.getA()
        //  delete test
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // TestClass test = new TestClass()
        ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestClass);
        ASTVar *TestVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyModifiers);
        ASTCall *ConstructorCall = CreateCall(TestClass->getName(), Args, ASTCallKind::CALL_NEW);
        ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
        SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(Body, TestVar);
        testNewStmt->setExpr(NewExpr);

        // int x = test.getA()
        ASTTypeRef *xType = getAMethod->getReturnTypeRef();
        ASTVar *xVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, xType, "x", EmptyModifiers);
        ASTCallExpr *xCallExpr = getASTBuilder().CreateExpr(CreateCall(getAMethod, Args, ASTCallKind::CALL_DIRECT, getASTBuilder().CreateVarRef(TestVar)));
        SemaBuilderStmt *xStmt = getASTBuilder().CreateAssignmentStmt(Body, xVar);
        xStmt->setExpr(xCallExpr);

        // delete test
        getASTBuilder().CreateDeleteStmt(Body, SourceLoc, getASTBuilder().CreateVarRef(TestVar));

		// validate and resolve
		EXPECT_TRUE(S->Resolve());

		// Generate Code
		llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // TestClass {
        //   int a
        //   int setA(int a) { this.a = a }
        // }
        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
        ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass", TopModifiers,
                                                  SuperClasses);

        // int a
        ASTVar *aAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, TestClass, IntTypeRef, "a", TopModifiers);

        // int setA(int a) { this.a = a }
    	llvm::SmallVector<ASTVar *, 8> setAParams;
    	ASTVar * aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
    	setAParams.push_back(aParam);
    	ASTBlockStmt *MethodBody = getASTBuilder().CreateBlockStmt(SourceLoc);
    	const SourceLocation &Loc = SourceLoc;
    	ASTRef * this_a = getASTBuilder().CreateVarRef(aAttribute, getASTBuilder().CreateVarRef(Loc, "this"));
    	SemaBuilderStmt *setAttributeAStmt = getASTBuilder().CreateAssignmentStmt(MethodBody, this_a);
    	setAttributeAStmt->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam)));
        ASTFunction *setAMethod = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, VoidTypeRef,
                                                               "setA", TopModifiers, setAParams, MethodBody);

        // int func() {
        //  TestClass test = new TestClass()
        //  test.setA(1)
        //  delete test
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // TestClass test = new TestClass()
        ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestClass);
        ASTVar *TestVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyModifiers);
        ASTCall *ConstructorCall = CreateCall(TestClass->getName(), Args, ASTCallKind::CALL_NEW);
        ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
        SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(Body, TestVar);
        testNewStmt->setExpr(NewExpr);

        // test.setA(1)
    	ASTNumberValue * IntValue = getASTBuilder().CreateNumberValue(SourceLocation(), "1");
    	llvm::SmallVector<ASTExpr *, 8> setAArgs;
    	setAArgs.push_back(getASTBuilder().CreateExpr(IntValue));
    	ASTCall *Call = CreateCall(setAMethod, setAArgs, ASTCallKind::CALL_DIRECT, getASTBuilder().CreateVarRef(TestVar));
    	SemaBuilderStmt * ExprStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
    	ExprStmt->setExpr(getASTBuilder().CreateExpr(Call));

        // delete test
        getASTBuilder().CreateDeleteStmt(Body, SourceLoc, getASTBuilder().CreateVarRef(TestVar));

		// validate and resolve
		EXPECT_TRUE(S->Resolve());

		// Generate Code
		llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // TestClass {
    	//   int a
    	//   TestClass() {
    	//     a = a()
    	//   }
        //   int a() { return 1 }
        // }
        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
        ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
                                                  TopModifiers, SuperClasses);

        // int a() { return 1 }
        ASTBlockStmt *aFuncBody = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *aFunc = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, IntTypeRef,
                                                          "a", TopModifiers, Params, aFuncBody);
        SemaBuilderStmt *aFuncReturn = getASTBuilder().CreateReturnStmt(aFuncBody, SourceLoc);
        ASTValueExpr *aFuncExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLocation(), "1"));
        aFuncReturn->setExpr(aFuncExpr);

        // int main() {
        //  TestClass test = new TestClass()
        //  int a = test.a()
        //  delete test
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // TestClass test = new TestClass()
        ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestClass);
        ASTVar *TestVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyModifiers);
        ASTCall *ConstructorCall = CreateCall(TestClass->getName(), Args, ASTCallKind::CALL_NEW);
        ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
        SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(Body, TestVar);
        testNewStmt->setExpr(NewExpr);

        // int a = test.a()
        ASTTypeRef *aType = aFunc->getReturnTypeRef();
        ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, aType, "a", EmptyModifiers);
        ASTCallExpr *aCallExpr = getASTBuilder().CreateExpr(CreateCall(aFunc, Args, ASTCallKind::CALL_DIRECT, getASTBuilder().CreateVarRef(TestVar)));
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
        ASTModule *Module = CreateModule();

        // TestClass {
        //   static int a
        // }
        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
        ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass", TopModifiers,
                                                  SuperClasses);

        // int a
    	llvm::SmallVector<ASTModifier *, 8> Modifiers;
    	Modifiers.push_back(getASTBuilder().CreateModifier(SourceLoc, ASTModifierKind::MOD_STATIC));
        ASTVar *aAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, TestClass, IntTypeRef, "a", Modifiers);

        // int func() {
        //  TestClass.a = 2
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        //  TestClass.a = 2
    	ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestClass);
    	ASTRef *test_a = getASTBuilder().CreateVarRef(aAttribute, TestClassType);
        SemaBuilderStmt *attrStmt = getASTBuilder().CreateAssignmentStmt(Body, test_a);
        ASTValueExpr *value2Expr = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLocation(), "2"));
        attrStmt->setExpr(value2Expr);

		// validate and resolve
		EXPECT_TRUE(S->Resolve());

		// Generate Code
		llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // TestClass {
        //   static int do() { return 1 }
        // }
        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
        ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
                                                  TopModifiers, SuperClasses);

        ASTBlockStmt *aFuncBody = getASTBuilder().CreateBlockStmt(SourceLoc);
    	llvm::SmallVector<ASTModifier *, 8> Modifiers;
    	Modifiers.push_back(getASTBuilder().CreateModifier(SourceLoc, ASTModifierKind::MOD_STATIC));
        llvm::SmallVector<ASTVar *, 8> Params;
        ASTFunction *aFunc = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, IntTypeRef,
                                                          "do", Modifiers, Params, aFuncBody);

        SemaBuilderStmt *aFuncReturn = getASTBuilder().CreateReturnStmt(aFuncBody, SourceLoc);
        ASTValueExpr *aFuncExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLocation(), "1"));
        aFuncReturn->setExpr(aFuncExpr);

        // int main() {
        //  int a = TestClass.do()
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // int a = TestClass.do()
        ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntTypeRef, "a", EmptyModifiers);
    	ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestClass);
        ASTCallExpr *aCallExpr = getASTBuilder().CreateExpr(CreateCall(aFunc, Args, ASTCallKind::CALL_DIRECT, TestClassType));
        SemaBuilderStmt *aStmt = getASTBuilder().CreateAssignmentStmt(Body, aVar);
        aStmt->setExpr(aCallExpr);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// Generate Code
    	llvm::Module * M = Generate();
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
    	ASTModule *Module = CreateModule();

    	// struct BaseStruct {
    	// int a
    	// }
    	//
    	// struct TestStruct : BaseStruct {
    	// int a
    	// }
    	llvm::SmallVector<ASTTypeRef *, 4> BaseSuperClasses;
    	ASTClass *BaseStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct",
			TopModifiers, BaseSuperClasses);

    	// int a
    	ASTVar *aAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, BaseStruct, IntTypeRef, "a", TopModifiers);

    	llvm::SmallVector<ASTTypeRef *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseStruct));
    	ASTClass *TestStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct",
			TopModifiers, TestSuperClasses);
    	getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// Generate Code
    	llvm::Module * M = Generate();
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

    	// BaseStruct
    	ASTClass *BaseStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTVar *aAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, BaseStruct, IntTypeRef, "a", TopModifiers);

    	// BaseStruct2
    	ASTClass *BaseStruct2 = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct2",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTVar *bAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, BaseStruct2, IntTypeRef, "b", TopModifiers);

    	// TestStruct
    	llvm::SmallVector<ASTTypeRef *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseStruct));
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseStruct2));
    	ASTClass *TestStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct",
			TopModifiers, TestSuperClasses);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// Generate Code
    	llvm::Module * M = Generate();
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
    	llvm::SmallVector<ASTTypeRef *, 4> BaseSuperClasses;
    	ASTClass *BaseStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTVar *aAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, BaseStruct, IntTypeRef, "a", TopModifiers);

    	// class TestClass
    	llvm::SmallVector<ASTTypeRef *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseStruct));
    	ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopModifiers, TestSuperClasses);

    	//   TestClass() {
    	//     this.a = 1
    	//   }
    	ASTBlockStmt *ConstructorBody = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTRef * this_a = getASTBuilder().CreateVarRef(aAttribute, getASTBuilder().CreateVarRef(SourceLoc, "this"));
    	SemaBuilderStmt *assignThis_a = getASTBuilder().CreateAssignmentStmt(ConstructorBody, this_a);
    	assignThis_a->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "1")));
    	ASTFunction *ConstructorMethod = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, VoidTypeRef,
															   "TestClass", TopModifiers, Params, ConstructorBody);



    	// func() { new TestClass() }
    	ASTBlockStmt *MainBody = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTCall *ConstructorCall = CreateCall(TestClass->getName(), Args, ASTCallKind::CALL_NEW);
    	ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
    	SemaBuilderStmt *testNewStmt = getASTBuilder().CreateExprStmt(MainBody, SourceLoc);
    	testNewStmt->setExpr(NewExpr);
    	getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, MainBody);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// Generate Code
    	llvm::Module * M = Generate();
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
    	llvm::SmallVector<ASTTypeRef *, 4> BaseSuperClasses;
    	ASTClass *BaseStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTVar *aAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, BaseStruct, IntTypeRef, "a", TopModifiers);

    	ASTClass *BaseStruct2 = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "BaseStruct2",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTVar *aAttribute2 = getASTBuilder().CreateClassAttribute(SourceLoc, BaseStruct2, IntTypeRef, "a", TopModifiers);
    	// int b
    	ASTVar *bAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, BaseStruct2, IntTypeRef, "b", TopModifiers);

    	// class TestClass
    	llvm::SmallVector<ASTTypeRef *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseStruct));
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseStruct2));
    	ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopModifiers, TestSuperClasses);
    	// int a
    	ASTVar *aAttribute3 = getASTBuilder().CreateClassAttribute(SourceLoc, TestClass, IntTypeRef, "a", TopModifiers);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// Generate Code
    	llvm::Module * M = Generate();
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
    	llvm::SmallVector<ASTTypeRef *, 4> BaseSuperClasses;
    	ASTClass *BaseClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "BaseClass",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTVar *aAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, BaseClass, IntTypeRef, "a", TopModifiers);
    	// void do()
    	ASTBlockStmt *DoBody = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTFunction *Do = getASTBuilder().CreateClassMethod(SourceLoc, BaseClass, VoidTypeRef, "do", TopModifiers, Params, DoBody);

    	// TestClass
    	llvm::SmallVector<ASTTypeRef *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseClass));
    	ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopModifiers, TestSuperClasses);
    	// void undo()
    	ASTBlockStmt *UndoBody = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTFunction *Undo = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, VoidTypeRef, "undo", TopModifiers, Params, UndoBody);

    	// func() { BaseClass a = new TestClass() }
    	ASTBlockStmt *MainBody = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTCall *ConstructorCall = CreateCall(TestClass->getName(), Args, ASTCallKind::CALL_NEW);
    	ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
    	ASTTypeRef *Base = getASTBuilder().CreateTypeRef(BaseClass);
    	ASTVar *aVar = getASTBuilder().CreateLocalVar(MainBody, SourceLoc, Base, "a", EmptyModifiers);
    	SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(MainBody, aVar);
    	testNewStmt->setExpr(NewExpr);
    	getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, MainBody);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// Generate Code
    	llvm::Module * M = Generate();
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
    	llvm::SmallVector<ASTTypeRef *, 4> BaseSuperClasses;
    	ASTClass *BaseClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "BaseClass",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTVar *aAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, BaseClass, IntTypeRef, "a", TopModifiers);
    	// void do()
    	ASTBlockStmt *DoBody = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTFunction *BaseClass_do = getASTBuilder().CreateClassMethod(SourceLoc, BaseClass, VoidTypeRef, "do", TopModifiers, Params, DoBody);

    	// BaseClass2
    	llvm::SmallVector<ASTTypeRef *, 4> Base2SuperClasses;
    	ASTClass *BaseClass2 = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "BaseClass2",
			TopModifiers, BaseSuperClasses);
    	// int a
    	ASTVar *bAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, BaseClass2, IntTypeRef, "b", TopModifiers);
    	// void do()
    	ASTBlockStmt *DoBody2 = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTFunction *BaseClass2_do = getASTBuilder().CreateClassMethod(SourceLoc, BaseClass2, VoidTypeRef, "do", TopModifiers, Params, DoBody2);
    	// void undo()
    	ASTBlockStmt *UndoBody = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTFunction *BaseClass2_undo = getASTBuilder().CreateClassMethod(SourceLoc, BaseClass2, VoidTypeRef, "undo", TopModifiers, Params, UndoBody);

    	// TestClass
    	llvm::SmallVector<ASTTypeRef *, 4> TestSuperClasses;
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseClass));
    	TestSuperClasses.push_back(getASTBuilder().CreateTypeRef(BaseClass2));
    	ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopModifiers, TestSuperClasses);

    	// void foo() {
    	//    BaseClass.do()
    	// }
    	ASTBlockStmt *DoBody3 = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTCallExpr *aCallExpr = getASTBuilder().CreateExpr(CreateCall(BaseClass_do, Args, ASTCallKind::CALL_DIRECT,
    		getASTBuilder().CreateTypeRef(BaseClass)));
    	SemaBuilderStmt *aStmt = getASTBuilder().CreateExprStmt(DoBody3, SourceLoc);
    	aStmt->setExpr(aCallExpr);
    	ASTFunction *TestClass_do = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, VoidTypeRef, "foo", TopModifiers, Params, DoBody3);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// Generate Code
    	llvm::Module * M = Generate();
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
    	llvm::SmallVector<ASTTypeRef *, 4> BaseInterfaces;
    	ASTClass *BaseInterface = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::INTERFACE,
    		"BaseInterface", TopModifiers, BaseInterfaces);
    	getASTBuilder().CreateClassMethod(SourceLoc, BaseInterface, VoidTypeRef, "do", TopModifiers, Params);


    	// BaseInterface2
    	llvm::SmallVector<ASTTypeRef *, 4> Base2Interfaces;
    	ASTClass *BaseInterface2 = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::INTERFACE,
    		"BaseInterface2", TopModifiers, Base2Interfaces);
    	getASTBuilder().CreateClassMethod(SourceLoc, BaseInterface2, VoidTypeRef, "undo", TopModifiers, Params);

    	llvm::SmallVector<ASTTypeRef *, 4> TestBaseInterfaces;
    	TestBaseInterfaces.push_back(getASTBuilder().CreateTypeRef(BaseInterface));
    	TestBaseInterfaces.push_back(getASTBuilder().CreateTypeRef(BaseInterface2));
    	ASTClass *TestInterface = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::INTERFACE,
    		"TestInterface", TopModifiers, TestBaseInterfaces);

    	// class TestClass
    	llvm::SmallVector<ASTTypeRef *, 4> TestBaseClasses;
    	TestBaseClasses.push_back(getASTBuilder().CreateTypeRef(TestInterface));
    	ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
			TopModifiers, TestBaseClasses);

    	// void do()
    	ASTBlockStmt *DoBody = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTFunction *TestClass_do = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, VoidTypeRef, "do", TopModifiers, Params, DoBody);

    	// void undo()
    	ASTBlockStmt *UndoBody = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTFunction *TestClass_undo = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, VoidTypeRef, "undo", TopModifiers, Params, UndoBody);

    	// main() { new TestClass() }
    	ASTBlockStmt *MainBody = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTCall *ConstructorCall = CreateCall(TestClass->getName(), Args, ASTCallKind::CALL_NEW);
    	ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);

    	ASTTypeRef *TestInterfaceTypeeRef = getASTBuilder().CreateTypeRef(TestInterface);
    	ASTVar *aVar = getASTBuilder().CreateLocalVar(MainBody, SourceLoc, TestInterfaceTypeeRef, "a", EmptyModifiers);
    	SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(MainBody, aVar);
    	testNewStmt->setExpr(NewExpr);
    	getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, MainBody);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// Generate Code
    	llvm::Module * M = Generate();
    	std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n");
    }

	TEST_F(CodeGenTest, CGClassExtendsStructAndInterface) {
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
