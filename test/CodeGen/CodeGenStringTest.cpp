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
#include "AST/ASTDeclStmt.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTModule.h"
#include "AST/ASTReturnStmt.h"
#include "AST/ASTValue.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGenTest.h"
#include "Sema/SemaBuilderModifiers.h"

#include <Sema/SemaNameSpace.h>

namespace {

    using namespace fly;

    TEST_F(CodeGenTest, CGDefaultStringLocalVar) {
        /**
         * Fly code:
         * void func() {
         *   string k
         * }
         */
        ASTModule *Module = CreateModule();

    	ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

    	// string k
    	ASTLocalVar *LocalVar_k = ASTBuilder::CreateLocalVar(SourceLoc, StringTypeRef, "k", EmptyModifiers);
    	ASTIdentifier *Ident_k = ASTBuilder::CreateIdentifier(LocalVar_k);
    	ASTDeclStmt *DeclStmt_k = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_k);

    	// Generate Code
    	Generate();

    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  store ptr null, ptr %2, align 8\n"
                        "  ret void\n"
                        "}\n");
    }

    TEST_F(CodeGenTest, CGDefaultStringLocalVarAssignEmpty) {
        /**
         * Fly code:
         * void func() {
         *   string k = ""
         * }
         */
        ASTModule *Module = CreateModule();

        llvm::SmallVector<ASTParam *, 8> Params;
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

    	// string k = ""
    	ASTLocalVar *LocalVar_k = ASTBuilder::CreateLocalVar(SourceLoc, StringTypeRef, "k", EmptyModifiers);
    	ASTIdentifier *Ident_k = ASTBuilder::CreateIdentifier(LocalVar_k);
    	ASTDeclStmt *DeclStmt_k = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_k);
    	ASTStringValue * EmptyString = ASTBuilder::CreateStringValue(SourceLoc, "");
    	ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, Ident_k, EmptyString);
    	DeclStmt_k->setExpr(AssignExpr);

    	// Generate Code
    	Generate();
    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  store ptr @0, ptr %2, align 8\n"
                        "  ret void\n"
                        "}\n");
     }

	TEST_F(CodeGenTest, CGDefaultStringLocalVarAssignValue) {
    	/**
		 * Fly code:
		 * void func() {
		 *   string k = "hello!"
		 * }
		 */
    	ASTModule *Module = CreateModule();

    	llvm::SmallVector<ASTParam *, 8> Params;
    	ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

    	// string k = ""
    	ASTLocalVar *LocalVar_k = ASTBuilder::CreateLocalVar(SourceLoc, StringTypeRef, "k", EmptyModifiers);
    	ASTIdentifier *Ident_k = ASTBuilder::CreateIdentifier(LocalVar_k);
    	ASTDeclStmt *DeclStmt_k = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_k);
    	ASTExpr *HelloStr = ASTBuilder::CreateStringValue(SourceLoc, "hello!");
    	ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, Ident_k, HelloStr);
    	DeclStmt_k->setExpr(AssignExpr);

    	// Generate Code
    	Generate();
    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  store ptr @0, ptr %2, align 8\n"
                        "  ret void\n"
                        "}\n");
    }

 } // anonymous namespace
