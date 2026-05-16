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

	TEST_F(CodeGenTest, CGInterface) {
        /**
         * Fly code:
         * interface BaseInterface {
         *   do()
         * }
         */
        ASTModule *Module = CreateModule();

    	// interface BaseInterface {
    	//   do()
    	// }

    	// BaseInterface
    	llvm::SmallVector<ASTType *, 4> BaseInterfaces;
    	ASTClass *BaseInterface = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::INTERFACE,
    		"BaseInterface", TopModifiers, BaseInterfaces);
    	ASTBuilder::CreateClassMethod(SourceLoc, BaseInterface, "do", TopModifiers, Params);

    	// Generate Code
    	Generate();
		llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n%error = type { i32, ptr, ptr }\n"
    	                  "\n"
    	                  "@error = external constant %error\n"
    	                  "@vtable.BaseInterface = constant [1 x ptr] zeroinitializer\n");
    }

	TEST_F(CodeGenTest, CGInterfaceExtendsInterfaces) {
        /**
         * Fly code:
         * interface BaseInterface {
         *   do()
         * }
         * interface BaseInterface2 {
         *   undo()
         * }
         * interface TestInterface : BaseInterface, BaseInterface2 {
         *
         * }
         * TestClass : TestInterface {
		 *   do() {}
		 *   undo() {}
         */
        ASTModule *Module = CreateModule();

		// interface BaseInterface {
		//   do()
		// }

    	// BaseInterface
    	llvm::SmallVector<ASTType *, 4> BaseInterfaces;
    	ASTClass *BaseInterface = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::INTERFACE,
    		"BaseInterface", TopModifiers, BaseInterfaces);
    	ASTBuilder::CreateClassMethod(SourceLoc, BaseInterface, "do", TopModifiers, Params);

		// interface BaseInterface2 {
		//   undo()
		// }

    	// BaseInterface2
    	llvm::SmallVector<ASTType *, 4> Base2Interfaces;
    	ASTClass *BaseInterface2 = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::INTERFACE,
    		"BaseInterface2", TopModifiers, Base2Interfaces);
    	ASTBuilder::CreateClassMethod(SourceLoc, BaseInterface2, "undo", TopModifiers, Params);

		//
		// interface TestInterface : BaseInterface, BaseInterface2 {}
		//

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
    	ASTMethod *TestClass_do = ASTBuilder::CreateClassMethod(SourceLoc, TestClass, "do", TopModifiers, Params, DoBody);

    	// void undo()
    	ASTBlockStmt *UndoBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTMethod *TestClass_undo = ASTBuilder::CreateClassMethod(SourceLoc, TestClass, "undo", TopModifiers, Params, UndoBody);

    	// Generate Code
    	Generate();
		llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n%error = type { i32, ptr, ptr }\n"
    	                  "%TestClass = type { ptr, %TestInterface }\n"
    	                  "%TestInterface = type { ptr, %BaseInterface, %BaseInterface2 }\n"
    	                  "%BaseInterface = type { ptr }\n"
    	                  "%BaseInterface2 = type { ptr }\n"
    	                  "\n"
    	                  "@error = external constant %error\n"
    	                  "@vtable.BaseInterface = constant [1 x ptr] zeroinitializer\n"
    	                  "@vtable.BaseInterface2 = constant [1 x ptr] zeroinitializer\n"
    	                  "@vtable.TestInterface = constant [1 x ptr] zeroinitializer\n"
    	                  "@vtable.TestClass = constant [4 x ptr] [ptr null, ptr @TestClass_F2do, ptr @TestClass_F4undo, ptr @TestClass_F9TestClass]\n"
    	                  "@vtable.TestClass.TestInterface = constant [1 x ptr] [ptr inttoptr (i64 -8 to ptr)]\n"
    	                  "\n"
    	                  "define linkonce_odr ptr @TestClass.init_ctor(ptr %0) {\n"
    	                  "entry:\n"
    	                  "  %1 = alloca ptr, align 8\n"
    	                  "  store ptr %0, ptr %1, align 8\n"
    	                  "  %2 = load ptr, ptr %1, align 8\n"
    	                  "  %3 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 0\n"
    	                  "  store ptr @vtable.TestClass, ptr %3, align 8\n"
    	                  "  %4 = getelementptr inbounds %TestClass, ptr %2, i32 0, i32 1\n"
    	                  "  %5 = getelementptr inbounds nuw %TestInterface, ptr %4, i32 0, i32 0\n"
    	                  "  store ptr @vtable.TestClass.TestInterface, ptr %5, align 8\n"
    	                  "  ret ptr %2\n"
    	                  "}\n"
    	                  "\n"
    	                  "define void @TestClass_F2do(ptr %0, ptr %1) {\n"
    	                  "entry:\n"
    	                  "  %2 = alloca ptr, align 8\n"
    	                  "  %3 = alloca ptr, align 8\n"
    	                  "  store ptr %0, ptr %2, align 8\n"
    	                  "  store ptr %1, ptr %3, align 8\n"
    	                  "  ret void\n"
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
    	                  "}\n");
    }

} // anonymous namespace
