//===--------------------------------------------------------------------------------------------------------------===//
// test/CodeGenErrorTest.cpp - Error tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

// fly
#include "AST/ASTClass.h"
#include "AST/ASTFunction.h"
#include "AST/ASTValue.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGenTest.h"

#include <AST/ASTDeclStmt.h>
#include <AST/ASTExprStmt.h>
#include <AST/ASTFailStmt.h>
#include <AST/ASTLocalVar.h>

namespace {

    using namespace fly;

	TEST_F(CodeGenTest, CGErrorHandler) {
        /**
         * Fly code:
         * void func() {
         *   handle {
         *     fail
         *   }
         * }
         */
        ASTModule *Module = CreateModule();

        // func() {
        //   handle fail
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);
        ASTBlockStmt *HandleBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTBuilder::CreateHandleStmt(Body, SourceLoc, HandleBlock);

        ASTFailStmt * Fail0Stmt = ASTBuilder::CreateFailStmt(HandleBlock, SourceLoc);

		// Generate Code
		Generate();
		llvm::Module * M = getModules()[0];
		std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n" // func() error handler
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = alloca %error*, align 8\n" // A error handler
                          "  br label %handle\n"
                          "\n"
                          "handle:                                           ; preds = %entry\n"
                          "  %3 = load %error*, %error** %2, align 8\n"
                          "  %4 = getelementptr inbounds %error, %error* %3, i32 0, i32 0\n"
                          "  store i32 1, i32* %4, align 4\n"
                          "  br label %safe\n"
                          "\n"
                          "safe:                                             ; preds = %handle\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGErrorFail0) {
        /**
         * Fly code:
         * int testFail() {
         *   fail
         * }
         * void main() {
         *   testFail()
         * }
         */
        ASTModule *Module = CreateModule();

        // int testFail() {
        //   fail
        // }
        ASTBlockStmt *Body0 = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail = ASTBuilder::CreateFunction(Module, SourceLoc, "testFail", TopModifiers, Params, Body0);
        ASTFailStmt * Fail0Stmt = ASTBuilder::CreateFailStmt(Body0, SourceLoc);

		// main() {
		//   testFail()
		// }
		ASTBlockStmt *MainBody = ASTBuilder::CreateBlockStmt(SourceLoc);
		ASTFunction *Main = ASTBuilder::CreateFunction(Module, SourceLoc, "main", TopModifiers, Params, MainBody);

		// call testFail0()
		ASTExprStmt * CallTestFail0 = ASTBuilder::CreateExprStmt(MainBody, SourceLoc);
		ASTCall *CallExpr0 = ASTBuilder::CreateCall(SourceLoc, TestFail->getName(), Args, ASTCallKind::CALL_DIRECT);
		CallTestFail0->setExpr(CallExpr0);

		// Generate Code
		Generate();
		llvm::Module * M = getModules()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i32, i8*, i8* }\n"
						  "\n"
						  "@error = external constant %error\n"
						  "\n"
        				  "define void @_F8testFail(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = load %error*, %error** %1, align 8\n"
                          "  %3 = getelementptr inbounds %error, %error* %2, i32 0, i32 0\n"
                          "  store i32 1, i32* %3, align 4\n"
                          "  ret void\n"
                          "}\n\n"
						  "define i32 @_F4main() {\n"
						  "entry:\n"
						  "  %0 = alloca %error*, align 8\n"
						  "  %1 = load %error*, %error** %0, align 8\n"
						  "  %2 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
						  "  store i32 0, i32* %2, align 4\n"
						  "  %3 = getelementptr inbounds %error, %error* %1, i32 0, i32 1\n"
						  "  store i8* null, i8** %3, align 8\n"
						  "  %4 = getelementptr inbounds %error, %error* %1, i32 0, i32 2\n"
						  "  store i8* null, i8** %4, align 8\n"
						  "  call void @_F8testFail(%error* %1)\n"
						  "  %5 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
						  "  %6 = load i32, i32* %5, align 4\n"
						  "  ret i32 %6\n"
						  "}\n");
    }

    TEST_F(CodeGenTest, CGErrorFail1) {
        /**
         * Fly code:
         * int testFail() {
         *   fail true
         * }
         * void main() {
         *   testFail()
         * }
         */
        ASTModule *Module = CreateModule();

        // int testFail() {
        //   fail true
        // }
        ASTBlockStmt *Body1 = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail = ASTBuilder::CreateFunction(Module, SourceLoc, "testFail", TopModifiers, Params, Body1);
        ASTBoolValue *BoolVal = ASTBuilder::CreateBoolValue(SourceLoc, true);
        ASTFailStmt * Fail1Stmt = ASTBuilder::CreateFailStmt(Body1, SourceLoc);
        Fail1Stmt->setFirstExpr(BoolVal);

		// main() {
		//   testFail()
		// }
		ASTBlockStmt *MainBody = ASTBuilder::CreateBlockStmt(SourceLoc);
		ASTFunction *Main = ASTBuilder::CreateFunction(Module, SourceLoc, "main", TopModifiers, Params, MainBody);

		// call testFail1()
		ASTExprStmt * CallTestFail1 = ASTBuilder::CreateExprStmt(MainBody, SourceLoc);
		ASTCall *CallExpr1 = ASTBuilder::CreateCall(SourceLoc, TestFail->getName(), Args, ASTCallKind::CALL_DIRECT);
		CallTestFail1->setExpr(CallExpr1);

		// Generate Code
		Generate();
		llvm::Module * M = getModules()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i32, i8*, i8* }\n"
						  "\n"
						  "@error = external constant %error\n"
						  "\n"
                          "define void @_F8testFail(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = load %error*, %error** %1, align 8\n"
                          "  %3 = getelementptr inbounds %error, %error* %2, i32 0, i32 0\n"
                          "  store i32 1, i32* %3, align 4\n"
                          "  ret void\n"
                          "}\n\n"
                          "define i32 @_F4main() {\n"
						  "entry:\n"
						  "  %0 = alloca %error*, align 8\n"
						  "  %1 = load %error*, %error** %0, align 8\n"
						  "  %2 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
						  "  store i32 0, i32* %2, align 4\n"
						  "  %3 = getelementptr inbounds %error, %error* %1, i32 0, i32 1\n"
						  "  store i8* null, i8** %3, align 8\n"
						  "  %4 = getelementptr inbounds %error, %error* %1, i32 0, i32 2\n"
						  "  store i8* null, i8** %4, align 8\n"
						  "  call void @_F8testFail(%error* %1)\n"
						  "  %5 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
						  "  %6 = load i32, i32* %5, align 4\n"
						  "  ret i32 %6\n"
						  "}\n");
    }

	TEST_F(CodeGenTest, CGErrorFail2) {
        /**
         * Fly code:
         * testFail() {
         *   fail 10
         * }
         * main() {
         *   testFail()
         * }
         */
        ASTModule *Module = CreateModule();

        // int testFail2() {
        //   fail 10
        // }
        ASTBlockStmt *Body2 = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail2 = ASTBuilder::CreateFunction(Module, SourceLoc, "testFail", TopModifiers, Params, Body2);
        ASTNumberValue *IntVal = ASTBuilder::CreateNumberValue(SourceLoc, "10");
        ASTFailStmt * Fail2Stmt = ASTBuilder::CreateFailStmt(Body2, SourceLoc);
        Fail2Stmt->setFirstExpr(IntVal);

        // main() {
        //   testFail()
        // }
        ASTBlockStmt *MainBody = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Main = ASTBuilder::CreateFunction(Module, SourceLoc, "main", TopModifiers, Params, MainBody);

        // call testFail()
        ASTExprStmt * CallTestFail2 = ASTBuilder::CreateExprStmt(MainBody, SourceLoc);
        ASTCall *CallExpr2 = ASTBuilder::CreateCall(SourceLoc, TestFail2->getName(), Args, ASTCallKind::CALL_DIRECT);
        CallTestFail2->setExpr(CallExpr2);

		// Generate Code
		Generate();
		llvm::Module * M = getModules()[0];
		std::string output = getOutput(M);

		EXPECT_EQ(output, "\n%error = type { i32, i8*, i8* }\n"
						  "\n"
						  "@error = external constant %error\n"
						  "\n"
						  "define void @_F8testFail(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = load %error*, %error** %1, align 8\n"
                          "  %3 = getelementptr inbounds %error, %error* %2, i32 0, i32 0\n"
                          "  store i32 10, i32* %3, align 4\n"
                          "  ret void\n"
                          "}\n\n"
                          "define i32 @_F4main() {\n"
                          "entry:\n"
                          "  %0 = alloca %error*, align 8\n"
                          "  %1 = load %error*, %error** %0, align 8\n"
                          "  %2 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
                          "  store i32 0, i32* %2, align 4\n"
                          "  %3 = getelementptr inbounds %error, %error* %1, i32 0, i32 1\n"
                          "  store i8* null, i8** %3, align 8\n"
                          "  %4 = getelementptr inbounds %error, %error* %1, i32 0, i32 2\n"
                          "  store i8* null, i8** %4, align 8\n"
                          "  call void @_F8testFail(%error* %1)\n"
                          "  %5 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
                          "  %6 = load i32, i32* %5, align 4\n"
                          "  ret i32 %6\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGErrorFail3) {
        /**
         * Fly code:
         * int testFail() {
         *   fail "Error"
         * }
         * void main() {
         *   testFail()
         * }
         */
        ASTModule *Module = CreateModule();

        // int testFail() {
        //  fail "Error"
        // }
        ASTBlockStmt *Body3 = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail3 = ASTBuilder::CreateFunction(Module, SourceLoc, "testFail", TopModifiers, Params, Body3);
        ASTStringValue *StrVal = ASTBuilder::CreateStringValue(SourceLoc, "Error");
        ASTFailStmt * Fail3Stmt = ASTBuilder::CreateFailStmt(Body3, SourceLoc);
        Fail3Stmt->setFirstExpr(StrVal);

        // main() {
        //   testFail()
        // }
        ASTBlockStmt *MainBody = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Main = ASTBuilder::CreateFunction(Module, SourceLoc, "main", TopModifiers, Params, MainBody);

        // call testFail()
        ASTExprStmt * CallTestFail3 = ASTBuilder::CreateExprStmt(MainBody, SourceLoc);
        ASTCall *CallExpr3 = ASTBuilder::CreateCall(SourceLoc, TestFail3->getName(), Args, ASTCallKind::CALL_DIRECT);
        CallTestFail3->setExpr(CallExpr3);

		// Generate Code
		Generate();
		llvm::Module * M = getModules()[0];
		std::string output = getOutput(M);

		EXPECT_EQ(output, "\n%error = type { i32, i8*, i8* }\n"
						  "\n"
						  "@error = external constant %error\n"
						  "@0 = private unnamed_addr constant [6 x i8] c\"Error\\00\", align 1\n"
						  "\n"
						  "define void @_F8testFail(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = load %error*, %error** %1, align 8\n"
                          "  %3 = getelementptr inbounds %error, %error* %2, i32 0, i32 1\n"
                          "  store i8* getelementptr inbounds ([6 x i8], [6 x i8]* @0, i32 0, i32 0), i8** %3, align 8\n"
                          "  ret void\n"
                          "}\n\n"
                          "define i32 @_F4main() {\n"
                          "entry:\n"
                          "  %0 = alloca %error*, align 8\n"
                          "  %1 = load %error*, %error** %0, align 8\n"
                          "  %2 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
                          "  store i32 0, i32* %2, align 4\n"
                          "  %3 = getelementptr inbounds %error, %error* %1, i32 0, i32 1\n"
                          "  store i8* null, i8** %3, align 8\n"
                          "  %4 = getelementptr inbounds %error, %error* %1, i32 0, i32 2\n"
                          "  store i8* null, i8** %4, align 8\n"
                          "  call void @_F8testFail(%error* %1)\n"
                          "  %5 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
                          "  %6 = load i32, i32* %5, align 4\n"
                          "  ret i32 %6\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, DISABLED_CGErrorFail4) {
        /**
         * Fly code:
         * struct TestStruct {
         *   int a
         * }
         * int testFail() {
         *   fail new TestStruct()
         * }
         * void main() {
         *   testFail()
         * }
         */
        ASTModule *Module = CreateModule();

        // struct TestStruct {
    	//  int a
    	// }
        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct", TopModifiers, SuperClasses);
        ASTAttribute *aField = ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

    	// int testFail() {
    	//  fail new TestStruct()
    	// }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail4 = ASTBuilder::CreateFunction(Module, SourceLoc, "testFail", TopModifiers, Params, Body);
        // TestStruct test = new TestStruct()
        ASTCall *ConstructorCall = ASTBuilder::CreateCall(SourceLoc, TestStruct->getName(), Args, ASTCallKind::CALL_NEW);
        // fail new TestStruct()
        ASTFailStmt * Fail4Stmt = ASTBuilder::CreateFailStmt(Body, SourceLoc);
        Fail4Stmt->setFirstExpr(ConstructorCall);

        // main() {
        //   testFail()
        // }
        ASTBlockStmt *MainBody = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Main = ASTBuilder::CreateFunction(Module, SourceLoc, "main", TopModifiers, Params, MainBody);

        // call testFail()
        ASTExprStmt * CallTestFail4 = ASTBuilder::CreateExprStmt(MainBody, SourceLoc);
        ASTCall *CallExpr4 = ASTBuilder::CreateCall(SourceLoc, TestFail4->getName(), Args, ASTCallKind::CALL_DIRECT);
        CallTestFail4->setExpr(CallExpr4);

		// Generate Code
		Generate();
		llvm::Module * M = getModules()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i32, i8*, i8* }\n"
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
