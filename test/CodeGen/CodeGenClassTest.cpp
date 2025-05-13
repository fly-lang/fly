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
#include "AST/ASTVarRef.h"
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

        EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
						  "%TestStruct = type { i32 }\n"
        				  "\n"
        				  "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %malloccall = tail call i8* @malloc(%TestStruct ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to %TestStruct))\n"
                          "  %2 = bitcast i8* %malloccall to %TestStruct*\n"
                          "  call void @TestStruct_TestStruct(%TestStruct* %2)\n"
                          "  ret void\n"
                          "}\n"
                          "\n"
						  "define void @TestStruct_TestStruct(%TestStruct* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %TestStruct*, align 8\n"
						  "  %2 = alloca i32, align 4\n"
						  "  store %TestStruct* %0, %TestStruct** %1, align 8\n"
						  "  %3 = load %TestStruct*, %TestStruct** %1, align 8\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
						  "declare noalias i8* @malloc(i64)\n");
    }

// TEST_F(CodeGenTest, CGStruct2) {
//         ASTModule *Module = CreateModule();
//
//         // struct TestStruct {
//         //   int a
//         //   int b
//         //   const int c
//         // }
//
//         llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
//         ASTClass *TestStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct",
//                                                    TopScopes, SuperClasses);
//         ASTVar *aField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopScopes);
//         ASTVar *bField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "b", TopScopes);
//         ASTVar *cField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "c", TopScopes);
//
//         // int func() {
//         //  TestStruct test = new TestStruct();
//         //  int a = test.a
//         //  test.b = 2
//         //  return 1
//         // }
//         ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
//         ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopScopes, Params, Body);
//
//         // TestStruct test = new TestStruct()
//         ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestStruct);
//         ASTVar *TestVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyScopes);
//         ASTCall *ConstructorCall = CreateCall(TestStruct->getName(), Args, ASTCallKind::CALL_NEW);
//         ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
//         SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(Body, TestVar);
//         testNewStmt->setExpr(NewExpr);
//
//         // int a = test.a
//         ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntTypeRef, "a", EmptyScopes);
//         ASTVarRef *Instance = CreateVarRef(TestVar);
//         ASTVarRef *test_aVarRef = CreateVarRef(aField,Instance);
//         ASTVarRefExpr *test_aRefExpr = getASTBuilder().CreateExpr(test_aVarRef);
//         SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(Body, aVar);
//         aVarStmt->setExpr(test_aRefExpr);
//
//         // test.b = 2
//         ASTVarRef *Instance2 = CreateVarRef(TestVar);
//         ASTVarRef *test_bVarRef = CreateVarRef(bField, Instance2);
//         SemaBuilderStmt *test_bVarStmt = getASTBuilder().CreateAssignmentStmt(Body, test_bVarRef);
//         ASTExpr *Expr = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "2"));
//         test_bVarStmt->setExpr(Expr);
//
//         // delete test
//         ASTDeleteStmt *Delete = getASTBuilder().CreateDeleteStmt(Body, SourceLoc, (ASTVarRef *) Instance);
//
// 		// validate and resolve
// 		EXPECT_TRUE(S->Resolve());
//
// 		// Generate Code
// 		llvm::Module * M = Generate();
// 		std::string output = getOutput(M);
//
//         EXPECT_EQ(output, "%TestStruct = type { i32, i32, i32 }\n"
//                           "%error = type { i8, i32, i8* }\n"
//                           "\n"
//                           "define void @TestStruct_TestStruct(%TestStruct* %0) {\n"
//                           "entry:\n"
//                           "  %1 = alloca %TestStruct*, align 8\n"
//                           "  store %TestStruct* %0, %TestStruct** %1, align 8\n"
//                           "  %2 = load %TestStruct*, %TestStruct** %1, align 8\n"
//                           "  %3 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 0\n"
//                           "  store i32 0, i32* %3, align 4\n"
//                           "  %4 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 1\n"
//                           "  store i32 0, i32* %4, align 4\n"
//                           "  %5 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 2\n"
//                           "  store i32 0, i32* %5, align 4\n"
//                           "  ret void\n"
//                           "}\n"
//                           "\n"
//                           "define void @F_0(%error* %0) {\n"
//                           "entry:\n"
//                           "  %1 = alloca %error*, align 8\n"
//                           "  %2 = alloca %TestStruct*, align 8\n"
//                           "  %3 = alloca i32, align 4\n"
//                           "  store %error* %0, %error** %1, align 8\n"
//                           "  %malloccall = tail call i8* @malloc(%TestStruct trunc (i64 mul nuw (i64 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i64), i64 3) to %TestStruct))\n"
//                           "  %4 = bitcast i8* %malloccall to %TestStruct*\n"
//                           "  call void @TestStruct_TestStruct(%TestStruct* %4)\n"
//                           "  store %TestStruct* %4, %TestStruct** %2, align 8\n"
//                           "  %5 = load %TestStruct*, %TestStruct** %2, align 8\n"
//                           "  %6 = getelementptr inbounds %TestStruct, %TestStruct* %5, i32 0, i32 0\n"
//                           "  %7 = load i32, i32* %6, align 4\n"
//                           "  store i32 %7, i32* %3, align 4\n"
//                           "  %8 = getelementptr inbounds %TestStruct, %TestStruct* %5, i32 0, i32 1\n"
//                           "  store i32 2, i32* %8, align 4\n"
//                           "  %9 = load %TestStruct*, %TestStruct** %2, align 8\n"
//                           "  %10 = bitcast %TestStruct* %9 to i8*\n"
//                           "  tail call void @free(i8* %10)\n"
//                           "  ret void\n"
//                           "}\n"
//                           "\n"
//                           "declare noalias i8* @malloc(i64)\n"
//                           "\n"
//                           "declare void @free(i8*)\n");
//     }
//
//     TEST_F(CodeGenTest, CGClassMethods) {
//         ASTModule *Module = CreateModule();
//
//         // TestClass {
//         //   int a() { return 1 }
//         //   public int b() { return 1 }
//         //   private const int c { return 1 }
//         // }
//         llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
//         ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
//                                                   TopScopes, SuperClasses);
//
//         // int a() { return 1 }
//         ASTBlockStmt *aFuncBody = getASTBuilder().CreateBlockStmt(SourceLoc);
//         ASTFunction *aFunc = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, IntTypeRef,
//                                                           "a", TopScopes, Params, aFuncBody);
//
//         SemaBuilderStmt *aFuncReturn = getASTBuilder().CreateReturnStmt(aFuncBody, SourceLoc);
//         ASTValueExpr *aFuncExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLocation(), "1"));
//         aFuncReturn->setExpr(aFuncExpr);
//
//
//         // public int b() { return 1 }
//         ASTBlockStmt *bFuncBody = getASTBuilder().CreateBlockStmt(SourceLoc);
//         ASTFunction *bFunc = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, IntTypeRef,
//                                                           "b", TopScopes, Params, bFuncBody);
//         SemaBuilderStmt *bFuncReturn = getASTBuilder().CreateReturnStmt(bFuncBody, SourceLoc);
//         ASTValueExpr *bFuncExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLocation(), "1"));
//         bFuncReturn->setExpr(bFuncExpr);
//
//         // private const int c { return 1 }
//         ASTBlockStmt *cFuncBody = getASTBuilder().CreateBlockStmt(SourceLoc);
//         ASTFunction *cFunc = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, IntTypeRef,
//                                                           "c", TopScopes, Params, cFuncBody);
//         SemaBuilderStmt *cFuncReturn = getASTBuilder().CreateReturnStmt(cFuncBody, SourceLoc);
//         ASTValueExpr *cFuncExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLocation(), "1"));
//         cFuncReturn->setExpr(cFuncExpr);
//
//         // int main() {
//         //  TestClass test = new TestClass()
//         //  int a = test.a()
//         //  int b = test.b()
//         //  int c = test.c()
//         //  delete test
//         // }
//         ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
//         ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopScopes, Params, Body);
//
//         // TestClass test = new TestClass()
//         ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestClass);
//         ASTVar *TestVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyScopes);
//         ASTCall *ConstructorCall = CreateCall(TestClass->getName(), Args, ASTCallKind::CALL_NEW);
//         ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
//         SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(Body, TestVar);
//         testNewStmt->setExpr(NewExpr);
//
//         // int a = test.a()
//         ASTTypeRef *aType = aFunc->getReturnTypeRef();
//         ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, aType, "a", EmptyScopes);
//         ASTCallExpr *aCallExpr = getASTBuilder().CreateExpr(CreateCall(aFunc, Args, ASTCallKind::CALL_FUNCTION, CreateVarRef(TestVar)));
//         SemaBuilderStmt *aStmt = getASTBuilder().CreateAssignmentStmt(Body, aVar);
//         aStmt->setExpr(aCallExpr);
//
//         // int b = test.b()
//         ASTTypeRef *bType = bFunc->getReturnTypeRef();
//         ASTVar *bVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, bType, "b", EmptyScopes);
//         ASTCallExpr *bCallExpr = getASTBuilder().CreateExpr(CreateCall(bFunc, Args, ASTCallKind::CALL_FUNCTION, CreateVarRef(TestVar)));
//         SemaBuilderStmt *bStmt = getASTBuilder().CreateAssignmentStmt(Body, bVar);
//         bStmt->setExpr(bCallExpr);
//
//         // int c = test.c()
//         ASTTypeRef *cType = cFunc->getReturnTypeRef();
//         ASTVar *cVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, cType, "c", EmptyScopes);
//         ASTCallExpr *cCallExpr = getASTBuilder().CreateExpr(CreateCall(cFunc, Args, ASTCallKind::CALL_FUNCTION, CreateVarRef(TestVar)));
//         SemaBuilderStmt *cStmt = getASTBuilder().CreateAssignmentStmt(Body, cVar);
//         cStmt->setExpr(cCallExpr);
//
//         // delete test
//         ASTDeleteStmt *DeleteStmt = getASTBuilder().CreateDeleteStmt(Body, SourceLoc, CreateVarRef(TestVar));
//
//     	// validate and resolve
//     	EXPECT_TRUE(S->Resolve());
//
//     	// Generate Code
//     	llvm::Module * M = Generate();
//     	std::string output = getOutput(M);
//
//         EXPECT_EQ(output, "%error = type { i8, i32, i8* }\n"
//                           "%TestClass = type { %TestClass_vtable* }\n"
//                           "%TestClass_vtable = type { i32 (%error*, %TestClass*), i32 (%error*, %TestClass*), i32 (%error*, %TestClass*) }\n"
//                           "\n"
//                           "define void @TestClass_TestClass(%error* %0, %TestClass* %1) {\n"
//                           "entry:\n"
//                           "  %2 = alloca %error*, align 8\n"
//                           "  %3 = alloca %TestClass*, align 8\n"
//                           "  store %error* %0, %error** %2, align 8\n"
//                           "  store %TestClass* %1, %TestClass** %3, align 8\n"
//                           "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
//                           "  ret void\n"
//                           "}\n"
//                           "\n"
//                           "define i32 @TestClass_a(%error* %0, %TestClass* %1) {\n"
//                           "entry:\n"
//                           "  %2 = alloca %error*, align 8\n"
//                           "  %3 = alloca %TestClass*, align 8\n"
//                           "  store %error* %0, %error** %2, align 8\n"
//                           "  store %TestClass* %1, %TestClass** %3, align 8\n"
//                           "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
//                           "  ret i32 1\n"
//                           "}\n"
//                           "\n"
//                           "define i32 @TestClass_b(%error* %0, %TestClass* %1) {\n"
//                           "entry:\n"
//                           "  %2 = alloca %error*, align 8\n"
//                           "  %3 = alloca %TestClass*, align 8\n"
//                           "  store %error* %0, %error** %2, align 8\n"
//                           "  store %TestClass* %1, %TestClass** %3, align 8\n"
//                           "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
//                           "  ret i32 1\n"
//                           "}\n"
//                           "\n"
//                           "define i32 @TestClass_c(%error* %0, %TestClass* %1) {\n"
//                           "entry:\n"
//                           "  %2 = alloca %error*, align 8\n"
//                           "  %3 = alloca %TestClass*, align 8\n"
//                           "  store %error* %0, %error** %2, align 8\n"
//                           "  store %TestClass* %1, %TestClass** %3, align 8\n"
//                           "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
//                           "  ret i32 1\n"
//                           "}\n"
//                           "\n"
//                           "define void @F_0(%error* %0) {\n"
//                           "entry:\n"
//                           "  %1 = alloca %error*, align 8\n"
//                           "  %2 = alloca %TestClass*, align 8\n"
//                           "  %3 = alloca i32, align 4\n"
//                           "  %4 = alloca i32, align 4\n"
//                           "  %5 = alloca i32, align 4\n"
//                           "  store %error* %0, %error** %1, align 8\n"
//                           "  %6 = load %error*, %error** %1, align 8\n"
//                           "  %7 = tail call i8* @malloc(i8 ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i8))\n"
//                           "  call void @TestClass_TestClass(%error* %6, i8* %7)\n"
//                           "  store i8* %7, %TestClass** %2, align 8\n"
//                           "  %8 = load %TestClass*, %TestClass** %2, align 8\n"
//                           "  %9 = call i32 @TestClass_a(%error* %6, %TestClass* %8)\n"
//                           "  store i32 %9, i32* %3, align 4\n"
//                           "  %10 = load %TestClass*, %TestClass** %2, align 8\n"
//                           "  %11 = call i32 @TestClass_b(%error* %6, %TestClass* %10)\n"
//                           "  store i32 %11, i32* %4, align 4\n"
//                           "  %12 = load %TestClass*, %TestClass** %2, align 8\n"
//                           "  %13 = call i32 @TestClass_c(%error* %6, %TestClass* %12)\n"
//                           "  store i32 %13, i32* %5, align 4\n"
//                           "  %14 = load %TestClass*, %TestClass** %2, align 8\n"
//                           "  %15 = bitcast %TestClass* %14 to i8*\n"
//                           "  tail call void @free(i8* %15)\n"
//                           "  ret void\n"
//                           "}\n"
//                           "\n"
//                           "declare noalias i8* @malloc(i64)\n"
//                           "\n"
//                           "declare void @free(i8*)\n"
//                           );
//     }
//
//     TEST_F(CodeGenTest, CGClassAttributes) {
//         ASTModule *Module = CreateModule();
//
//         // TestClass {
//         //   int a
//         //   int getA() { return a }
//         // }
//         llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
//         ASTClass *TestClass = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass", TopScopes,
//                                                   SuperClasses);
//
//         // int a
//         ASTVar *aAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, TestClass, IntTypeRef, "a", TopScopes);
//
//         // int getA() { return a }
//         ASTBlockStmt *MethodBody = getASTBuilder().CreateBlockStmt(SourceLoc);
//         ASTFunction *getAMethod = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, IntTypeRef,
//                                                                "getA", TopScopes, Params, MethodBody);
//
//         SemaBuilderStmt *MethodReturn = getASTBuilder().CreateReturnStmt(MethodBody, SourceLoc);
//         MethodReturn->setExpr(getASTBuilder().CreateExpr(CreateVarRef(aAttribute)));
//
//         // int main() {
//         //  TestClass test = new TestClass()
//         //  int x = test.getA()
//         //  test.a = 2
//         //  delete test
//         // }
//         ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
//         ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopScopes, Params, Body);
//
//         // TestClass test = new TestClass()
//         ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestClass);
//         ASTVar *TestVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyScopes);
//         ASTCall *ConstructorCall = CreateCall(TestClass->getName(), Args, ASTCallKind::CALL_NEW);
//         ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
//         SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(Body, TestVar);
//         testNewStmt->setExpr(NewExpr);
//
//         // int x = test.m()
//         ASTTypeRef *xType = getAMethod->getReturnTypeRef();
//         ASTVar *xVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, xType, "x", EmptyScopes);
//         ASTCallExpr *xCallExpr = getASTBuilder().CreateExpr(CreateCall(getAMethod, Args, ASTCallKind::CALL_FUNCTION, CreateVarRef(TestVar)));
//         SemaBuilderStmt *xStmt = getASTBuilder().CreateAssignmentStmt(Body, xVar);
//         xStmt->setExpr(xCallExpr);
//
//         //  test.a = 2
//         SemaBuilderStmt *attrStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(aAttribute, CreateVarRef(TestVar)));
//         ASTValueExpr *value2Expr = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLocation(), "2"));
//         attrStmt->setExpr(value2Expr);
//
//         // delete test
//         ASTDeleteStmt *DeleteStmt = getASTBuilder().CreateDeleteStmt(Body, SourceLoc, CreateVarRef(TestVar));
//
// 		// validate and resolve
// 		EXPECT_TRUE(S->Resolve());
//
// 		// Generate Code
// 		llvm::Module * M = Generate();
// 		std::string output = getOutput(M);
//
//         EXPECT_EQ(output, "%error = type { i8, i32, i8* }\n"
//                           "%TestClass = type { %TestClass_vtable*, i32 }\n"
//                           "%TestClass_vtable = type { i32 (%error*, %TestClass*) }\n"
//                           "\n"
//                           "define void @TestClass_TestClass(%error* %0, %TestClass* %1) {\n"
//                           "entry:\n"
//                           "  %2 = alloca %error*, align 8\n"
//                           "  %3 = alloca %TestClass*, align 8\n"
//                           "  store %error* %0, %error** %2, align 8\n"
//                           "  store %TestClass* %1, %TestClass** %3, align 8\n"
//                           "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
//                           "  %5 = getelementptr inbounds %TestClass, %TestClass* %4, i32 0, i32 1\n"
//                           "  store i32 0, i32* %5, align 4\n"
//                           "  ret void\n"
//                           "}\n"
//                           "\n"
//                           "define i32 @TestClass_getA(%error* %0, %TestClass* %1) {\n"
//                           "entry:\n"
//                           "  %2 = alloca %error*, align 8\n"
//                           "  %3 = alloca %TestClass*, align 8\n"
//                           "  store %error* %0, %error** %2, align 8\n"
//                           "  store %TestClass* %1, %TestClass** %3, align 8\n"
//                           "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
//                           "  %5 = getelementptr inbounds %TestClass, %TestClass* %4, i32 0, i32 1\n"
//                           "  %6 = load i32, i32* %5, align 4\n"
//                           "  ret i32 %6\n"
//                           "}\n"
//                           "\n"
//                           "define void @F_0(%error* %0) {\n"
//                           "entry:\n"
//                           "  %1 = alloca %error*, align 8\n"
//                           "  %2 = alloca %TestClass*, align 8\n"
//                           "  %3 = alloca i32, align 4\n"
//                           "  store %error* %0, %error** %1, align 8\n"
//                           "  %4 = load %error*, %error** %1, align 8\n"
//                           // TestClass test = new TestClass()
//                           "  %malloccall = tail call i8* @malloc(%TestClass ptrtoint (%TestClass* getelementptr (%TestClass, %TestClass* null, i32 1) to %TestClass))\n"
//                           "  %5 = bitcast i8* %malloccall to %TestClass*\n"
//                           "  call void @TestClass_TestClass(%error* %4, %TestClass* %5)\n"
//                           "  store %TestClass* %5, %TestClass** %2, align 8\n"
//                           "  %6 = load %TestClass*, %TestClass** %2, align 8\n"
//                           // int a = test.a()
//                           "  %7 = call i32 @TestClass_getA(%error* %4, %TestClass* %6)\n"
//                           "  store i32 %7, i32* %3, align 4\n"
//                           // test.a = 2
//                           "  %8 = getelementptr inbounds %TestClass, %TestClass* %6, i32 0, i32 1\n"
//                           "  store i32 2, i32* %8, align 4\n"
//                           // delete test
//                           "  %9 = load %TestClass*, %TestClass** %2, align 8\n"
//                           "  %10 = bitcast %TestClass* %9 to i8*\n"
//                           "  tail call void @free(i8* %10)\n"
//                           "  ret void\n"
//                           "}\n"
//                           "\n"
//                           "declare noalias i8* @malloc(i64)\n"
//                           "\n"
//                           "declare void @free(i8*)\n"
//         );
//     }
} // anonymous namespace
