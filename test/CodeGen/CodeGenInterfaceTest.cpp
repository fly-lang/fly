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

    	EXPECT_EQ(output, "\n");
    }

	TEST_F(CodeGenTest, DISABLED_CGInterfaceExtendsInterfaces) {
        /**
         * Fly code:
         * interface BaseInterface {
         *   do()
         * }
         * interface BaseInterface2 {
         *   undo()
         * }
         * interface TestInterface : BaseInterface, BaseInterface2 {
         *   call()
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

    	EXPECT_EQ(output, "\n");
    }

} // anonymous namespace
