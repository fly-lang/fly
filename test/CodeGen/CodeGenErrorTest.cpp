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
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"

#include <AST/ASTBuilderStmt.h>
#include <AST/ASTCallExpr.h>
#include <AST/ASTExprStmt.h>
#include <AST/ASTFailStmt.h>
#include <AST/ASTValueExpr.h>

namespace {

    using namespace fly;



	TEST_F(CodeGenTest, CGErrorHandler) {
        ASTModule *Module = CreateModule();

        // func() {
        //   error A = handle fail
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);
        ASTVar *ErrorA = getASTBuilder().CreateLocalVar(Body, SourceLoc, ErrorTypeRef, "A", EmptyModifiers);
        ASTIdentifier *ErrorVarRef = getASTBuilder().CreateVarRef(ErrorA);
        ASTBlockStmt *HandleBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        getASTBuilder().CreateHandleStmt(Body, SourceLoc, HandleBlock, ErrorVarRef);

        ASTBuilderStmt *Fail0Stmt = getASTBuilder().CreateFailStmt(HandleBlock, SourceLoc);

		// validate and resolve
		EXPECT_TRUE(S->Resolve());

		// Generate Code
		llvm::Module * M = Generate();
		std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n" // func() error handler
                          "  %2 = alloca %error*, align 8\n" // A error handler
                          "  store %error* %0, %error** %1, align 8\n"
                          "  br label %handle\n"
                          "\n"
                          "handle:                                           ; preds = %entry\n"
                          "  %3 = load %error*, %error** %2, align 8\n"
                          "  %4 = getelementptr inbounds %error, %error* %3, i32 0, i32 0\n"
                          "  store i8 1, i8* %4, align 1\n"
                          "  %5 = getelementptr inbounds %error, %error* %3, i32 0, i32 1\n"
                          "  store i32 1, i32* %5, align 4\n"
                          "  br label %safe\n"
                          "\n"
                          "safe:                                             ; preds = %handle\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGErrorFail0) {
        ASTModule *Module = CreateModule();

        // int testFail() {
        //   fail
        // }
        ASTBlockStmt *Body0 = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail = getASTBuilder().CreateFunction(Module, SourceLoc, IntTypeRef, "testFail", TopModifiers, Params, Body0);
        ASTBuilderStmt *Fail0Stmt = getASTBuilder().CreateFailStmt(Body0, SourceLoc);

		// main() {
		//   testFail()
		// }
		ASTBlockStmt *MainBody = getASTBuilder().CreateBlockStmt(SourceLoc);
		ASTFunction *Main = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "main", TopModifiers, Params, MainBody);

		// call testFail0()
		ASTBuilderStmt *CallTestFail0 = getASTBuilder().CreateExprStmt(MainBody, SourceLoc);
		ASTCallExpr *CallExpr0 = getASTBuilder().CreateExpr(CreateCall(TestFail, Args, ASTCallKind::CALL_DIRECT));
		CallTestFail0->setExpr(CallExpr0);

		// validate and resolve
		EXPECT_TRUE(S->Resolve());

		// Generate Code
		llvm::Module * M = Generate();
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
						  "\n"
						  "@error = external constant %error\n"
						  "\n"
        				  "define i32 @_F8testFail(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = load %error*, %error** %1, align 8\n"
                          "  %3 = getelementptr inbounds %error, %error* %2, i32 0, i32 0\n"
                          "  store i8 1, i8* %3, align 1\n"
                          "  %4 = getelementptr inbounds %error, %error* %2, i32 0, i32 1\n"
                          "  store i32 1, i32* %4, align 4\n"
                          "  ret i32 0\n"
                          "}\n\n"
						  "define i32 @_F4main() {\n"
						  "entry:\n"
						  "  %0 = alloca %error*, align 8\n"
						  "  %1 = load %error*, %error** %0, align 8\n"
						  "  %2 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
						  "  store i8 0, i8* %2, align 1\n"
						  "  %3 = getelementptr inbounds %error, %error* %1, i32 0, i32 1\n"
						  "  store i32 0, i32* %3, align 4\n"
						  "  %4 = getelementptr inbounds %error, %error* %1, i32 0, i32 2\n"
						  "  store i8* null, i8** %4, align 8\n"
						  "  %5 = call i32 @_F8testFail(%error* %1)\n"
						  "  %6 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
						  "  %7 = load i8, i8* %6, align 1\n"
						  "  %8 = icmp ne i8 %7, 0\n"
						  "  %9 = zext i1 %8 to i32\n"
						  "  ret i32 %9\n"
						  "}\n");
    }

    TEST_F(CodeGenTest, CGErrorFail1) {
        ASTModule *Module = CreateModule();

        // int testFail() {
        //   fail true
        // }
        ASTBlockStmt *Body1 = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail = getASTBuilder().CreateFunction(Module, SourceLoc, IntTypeRef, "testFail", TopModifiers, Params, Body1);
        ASTBoolValue *BoolVal = getASTBuilder().CreateBoolValue(SourceLoc, true);
        ASTBuilderStmt *Fail1Stmt = getASTBuilder().CreateFailStmt(Body1, SourceLoc);
        Fail1Stmt->setExpr(getASTBuilder().CreateExpr(BoolVal));

		// main() {
		//   testFail()
		// }
		ASTBlockStmt *MainBody = getASTBuilder().CreateBlockStmt(SourceLoc);
		ASTFunction *Main = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "main", TopModifiers, Params, MainBody);

		// call testFail1()
		ASTBuilderStmt *CallTestFail1 = getASTBuilder().CreateExprStmt(MainBody, SourceLoc);
		ASTCallExpr *CallExpr1 = getASTBuilder().CreateExpr(CreateCall(TestFail, Args, ASTCallKind::CALL_DIRECT));
		CallTestFail1->setExpr(CallExpr1);

		// validate and resolve
		EXPECT_TRUE(S->Resolve());

		// Generate Code
		llvm::Module * M = Generate();
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
						  "\n"
						  "@error = external constant %error\n"
						  "\n"
                          "define i32 @_F8testFail(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = load %error*, %error** %1, align 8\n"
                          "  %3 = getelementptr inbounds %error, %error* %2, i32 0, i32 0\n"
                          "  store i8 1, i8* %3, align 1\n"
                          "  %4 = getelementptr inbounds %error, %error* %2, i32 0, i32 1\n"
                          "  store i1 true, i32* %4, align 1\n"
                          "  ret i32 0\n"
                          "}\n\n"
                          "define i32 @_F4main() {\n"
						  "entry:\n"
						  "  %0 = alloca %error*, align 8\n"
						  "  %1 = load %error*, %error** %0, align 8\n"
						  "  %2 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
						  "  store i8 0, i8* %2, align 1\n"
						  "  %3 = getelementptr inbounds %error, %error* %1, i32 0, i32 1\n"
						  "  store i32 0, i32* %3, align 4\n"
						  "  %4 = getelementptr inbounds %error, %error* %1, i32 0, i32 2\n"
						  "  store i8* null, i8** %4, align 8\n"
						  "  %5 = call i32 @_F8testFail(%error* %1)\n"
						  "  %6 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
						  "  %7 = load i8, i8* %6, align 1\n"
						  "  %8 = icmp ne i8 %7, 0\n"
						  "  %9 = zext i1 %8 to i32\n"
						  "  ret i32 %9\n"
						  "}\n");
    }

	TEST_F(CodeGenTest, CGErrorFail2) {
        ASTModule *Module = CreateModule();

        // int testFail2() {
        //   fail 10
        // }
        ASTBlockStmt *Body2 = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail2 = getASTBuilder().CreateFunction(Module, SourceLoc, IntTypeRef, "testFail", TopModifiers, Params, Body2);
        ASTNumberValue *IntVal = getASTBuilder().CreateNumberValue(SourceLoc, "10");
        ASTBuilderStmt *Fail2Stmt = getASTBuilder().CreateFailStmt(Body2, SourceLoc);
        Fail2Stmt->setExpr(getASTBuilder().CreateExpr(IntVal));

        // main() {
        //   testFail()
        // }
        ASTBlockStmt *MainBody = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Main = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "main", TopModifiers, Params, MainBody);

        // call testFail()
        ASTBuilderStmt *CallTestFail2 = getASTBuilder().CreateExprStmt(MainBody, SourceLoc);
        ASTCallExpr *CallExpr2 = getASTBuilder().CreateExpr(CreateCall(TestFail2, Args, ASTCallKind::CALL_DIRECT));
        CallTestFail2->setExpr(CallExpr2);

		// validate and resolve
		EXPECT_TRUE(S->Resolve());

		// Generate Code
		llvm::Module * M = Generate();
		std::string output = getOutput(M);

		EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
						  "\n"
						  "@error = external constant %error\n"
						  "\n"
						  "define i32 @_F8testFail(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = load %error*, %error** %1, align 8\n"
                          "  %3 = getelementptr inbounds %error, %error* %2, i32 0, i32 0\n"
                          "  store i8 1, i8* %3, align 1\n"
                          "  %4 = getelementptr inbounds %error, %error* %2, i32 0, i32 1\n"
                          "  store i32 10, i32* %4, align 4\n"
                          "  ret i32 0\n"
                          "}\n\n"
                          "define i32 @_F4main() {\n"
                          "entry:\n"
                          "  %0 = alloca %error*, align 8\n"
                          "  %1 = load %error*, %error** %0, align 8\n"
                          "  %2 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
                          "  store i8 0, i8* %2, align 1\n"
                          "  %3 = getelementptr inbounds %error, %error* %1, i32 0, i32 1\n"
                          "  store i32 0, i32* %3, align 4\n"
                          "  %4 = getelementptr inbounds %error, %error* %1, i32 0, i32 2\n"
                          "  store i8* null, i8** %4, align 8\n"
                          "  %5 = call i32 @_F8testFail(%error* %1)\n"
                          "  %6 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
                          "  %7 = load i8, i8* %6, align 1\n"
                          "  %8 = icmp ne i8 %7, 0\n"
                          "  %9 = zext i1 %8 to i32\n"
                          "  ret i32 %9\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGErrorFail3) {
        ASTModule *Module = CreateModule();

        // int testFail() {
        //  fail "Error"
        // }
        ASTBlockStmt *Body3 = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail3 = getASTBuilder().CreateFunction(Module, SourceLoc, IntTypeRef, "testFail", TopModifiers, Params, Body3);
        ASTStringValue *StrVal = getASTBuilder().CreateStringValue(SourceLoc, "Error");
        ASTBuilderStmt *Fail3Stmt = getASTBuilder().CreateFailStmt(Body3, SourceLoc);
        Fail3Stmt->setExpr(getASTBuilder().CreateExpr(StrVal));

        // main() {
        //   testFail()
        // }
        ASTBlockStmt *MainBody = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Main = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "main", TopModifiers, Params, MainBody);

        // call testFail()
        ASTBuilderStmt *CallTestFail3 = getASTBuilder().CreateExprStmt(MainBody, SourceLoc);
        ASTCallExpr *CallExpr3 = getASTBuilder().CreateExpr(CreateCall(TestFail3, Args, ASTCallKind::CALL_DIRECT));
        CallTestFail3->setExpr(CallExpr3);

		// validate and resolve
		EXPECT_TRUE(S->Resolve());

		// Generate Code
		llvm::Module * M = Generate();
		std::string output = getOutput(M);

		EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
						  "\n"
						  "@error = external constant %error\n"
						  "@0 = private unnamed_addr constant [6 x i8] c\"Error\\00\", align 1\n"
						  "\n"
						  "define i32 @_F8testFail(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = load %error*, %error** %1, align 8\n"
                          "  %3 = getelementptr inbounds %error, %error* %2, i32 0, i32 0\n"
                          "  store i8 2, i8* %3, align 1\n"
                          "  %4 = getelementptr inbounds %error, %error* %2, i32 0, i32 2\n"
                          "  store i8* getelementptr inbounds ([6 x i8], [6 x i8]* @0, i32 0, i32 0), i8** %4, align 8\n"
                          "  ret i32 0\n"
                          "}\n\n"
                          "define i32 @_F4main() {\n"
                          "entry:\n"
                          "  %0 = alloca %error*, align 8\n"
                          "  %1 = load %error*, %error** %0, align 8\n"
                          "  %2 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
                          "  store i8 0, i8* %2, align 1\n"
                          "  %3 = getelementptr inbounds %error, %error* %1, i32 0, i32 1\n"
                          "  store i32 0, i32* %3, align 4\n"
                          "  %4 = getelementptr inbounds %error, %error* %1, i32 0, i32 2\n"
                          "  store i8* null, i8** %4, align 8\n"
                          "  %5 = call i32 @_F8testFail(%error* %1)\n"
                          "  %6 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
                          "  %7 = load i8, i8* %6, align 1\n"
                          "  %8 = icmp ne i8 %7, 0\n"
                          "  %9 = zext i1 %8 to i32\n"
                          "  ret i32 %9\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGErrorFail4) {
        ASTModule *Module = CreateModule();

        // struct TestStruct {
    	//  int a
    	// }
        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestStruct = getASTBuilder().CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct", TopModifiers, SuperClasses);
        ASTVar *aField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

    	// int testFail() {
    	//  fail new TestStruct()
    	// }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail4 = getASTBuilder().CreateFunction(Module, SourceLoc, IntTypeRef, "testFail", TopModifiers, Params, Body);
        // TestStruct test = new TestStruct()
        ASTCall *ConstructorCall = CreateCall(TestStruct->getName(), Args, ASTCallKind::CALL_NEW);
        // fail new TestStruct()
        ASTBuilderStmt *Fail4Stmt = getASTBuilder().CreateFailStmt(Body, SourceLoc);
        Fail4Stmt->setExpr(getASTBuilder().CreateExpr(ConstructorCall));

        // main() {
        //   testFail()
        // }
        ASTBlockStmt *MainBody = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Main = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "main", TopModifiers, Params, MainBody);

        // call testFail()
        ASTBuilderStmt *CallTestFail4 = getASTBuilder().CreateExprStmt(MainBody, SourceLoc);
        ASTCallExpr *CallExpr4 = getASTBuilder().CreateExpr(CreateCall(TestFail4, Args, ASTCallKind::CALL_DIRECT));
        CallTestFail4->setExpr(CallExpr4);

		// validate and resolve
		EXPECT_TRUE(S->Resolve());

		// Generate Code
		llvm::Module * M = Generate();
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
        				  "%TestStruct = type { i32 }\n"
        				  "\n"
						  "@error = external constant %error\n"
						  "\n"
        				  "define i32 @_F8testFail(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = call i8* @malloc(i64 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i64))\n"
                          "  %3 = bitcast i8* %2 to %TestStruct*\n"
                          "  %4 = call %TestStruct* @TestStruct.init_ctor(%TestStruct* %3)\n"
                          "  %5 = load %error*, %error** %1, align 8\n"
                          "  %6 = getelementptr inbounds %error, %error* %5, i32 0, i32 0\n"
                          "  store i8 3, i8* %6, align 1\n"
                          "  %7 = getelementptr inbounds %error, %error* %5, i32 0, i32 2\n"
                          "  store %TestStruct* %4, i8** %7, align 8\n"
                          "  ret i32 0\n"
                          "}\n"
                          "\n"
                          "define i32 @_F4main() {\n"
                          "entry:\n"
                          "  %0 = alloca %error*, align 8\n"
                          "  %1 = load %error*, %error** %0, align 8\n"
                          "  %2 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
                          "  store i8 0, i8* %2, align 1\n"
                          "  %3 = getelementptr inbounds %error, %error* %1, i32 0, i32 1\n"
                          "  store i32 0, i32* %3, align 4\n"
                          "  %4 = getelementptr inbounds %error, %error* %1, i32 0, i32 2\n"
                          "  store i8* null, i8** %4, align 8\n"
                          "  %5 = call i32 @_F8testFail(%error* %1)\n"
                          "  %6 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
                          "  %7 = load i8, i8* %6, align 1\n"
                          "  %8 = icmp ne i8 %7, 0\n"
                          "  %9 = zext i1 %8 to i32\n"
                          "  ret i32 %9\n"
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
} // anonymous namespace
