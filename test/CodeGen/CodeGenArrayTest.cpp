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
#include "AST/ASTType.h"
#include "AST/ASTValue.h"
#include "AST/ASTVar.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGenTest.h"

namespace {

    using namespace fly;

    TEST_F(CodeGenTest, CGArrayLocalVar) {
        /**
         * Fly code:
         * void func() {
         *   int[] k // Zero size array
         * }
         */
        ASTModule *Module = CreateModule();

        // Build function with an array parameter: void func(int[] k) {}
        llvm::SmallVector<ASTParam *, 8> LocalParams;
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, LocalParams, Body);

    	ASTArrayType *ArrayIntType = ASTBuilder::CreateArrayType(SourceLoc, IntTypeRef, nullptr);
    	ASTLocalVar *LocalVar_k = ASTBuilder::CreateLocalVar(SourceLoc, ArrayIntType, "k", EmptyModifiers);
    	ASTDeclStmt *DeclStmt_k = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_k);

        // Generate Code
    	Generate();
        llvm::Module *M = getModules()[0];
        std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca i32*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  store i32* null, i32** %2, align 8\n"
                          "  ret void\n"
                          "}\n");
    }

	TEST_F(CodeGenTest, CGArrayLocalVaZero) {
    	/**
		 * Fly code:
		 * void func() {
		 *   int[0] k // Zero size array like int[0]
		 * }
		 */
    	ASTModule *Module = CreateModule();

    	// Build function with an array parameter: void func(int[] k) {}
    	llvm::SmallVector<ASTParam *, 8> LocalParams;
    	ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, LocalParams, Body);

    	ASTNumberValue *ZeroValue = ASTBuilder::CreateNumberValue(SourceLoc, "0");
    	ASTArrayType *ArrayIntType = ASTBuilder::CreateArrayType(SourceLoc, IntTypeRef, ZeroValue);
    	ASTLocalVar *LocalVar_k = ASTBuilder::CreateLocalVar(SourceLoc, ArrayIntType, "k", EmptyModifiers);
    	ASTDeclStmt *DeclStmt_k = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_k);

    	// Generate Code
    	Generate();
    	llvm::Module *M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

    	EXPECT_EQ(output, "define void @_F4func(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  %2 = alloca i32*, align 8\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  store i32* null, i32** %2, align 8\n"
						  "  ret void\n"
						  "}\n");
    }

	TEST_F(CodeGenTest, CGArrayLocalVarAssignEmpty) {
    	/**
		 * Fly code:
		 * void func() {
		 *   int[] k = {} // zero size array like int[0]
		 * }
		 */
    	ASTModule *Module = CreateModule();

    	ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

    	// int[] k = {}
    	ASTArrayType *ArrayIntType = ASTBuilder::CreateArrayType(SourceLoc, IntTypeRef, nullptr);
    	ASTLocalVar *LocalVar_k = ASTBuilder::CreateLocalVar(SourceLoc, ArrayIntType, "k", EmptyModifiers);
    	ASTDeclStmt *DeclStmt_k = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_k);
    	llvm::SmallVector<ASTValue *, 8> EmptyVals;
    	ASTArrayValue *EmptyArr = ASTBuilder::CreateArrayValue(SourceLoc, EmptyVals);
    	ASTIdentifier *kIdent = ASTBuilder::CreateIdentifier(LocalVar_k);
    	ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, kIdent, EmptyArr);
    	DeclStmt_k->setExpr(AssignExpr);

    	// Generate Code
    	Generate();
    	llvm::Module *M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

    	EXPECT_EQ(output, "define void @_F0(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  %12 = alloca i64*, align 8\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  store i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), [0 x i8]* %13, align 8\n"
						  "  store i8 48, i8* %14, align 1\n"
						  "}\n");
    }

	TEST_F(CodeGenTest, CGArrayLocalVarAssignZero) {
    	/**
		 * Fly code:
		 * void func() {
		 *   int[0] k = {} // zero size array like int[0]
		 * }
		 */
    	ASTModule *Module = CreateModule();

    	ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

    	// int[0] k = {}
    	ASTNumberValue *Value_0 = ASTBuilder::CreateNumberValue(SourceLoc, "0");
    	ASTArrayType *ArrayIntType = ASTBuilder::CreateArrayType(SourceLoc, IntTypeRef, Value_0);
    	ASTLocalVar *LocalVar_k = ASTBuilder::CreateLocalVar(SourceLoc, ArrayIntType, "k", EmptyModifiers);
    	ASTDeclStmt *DeclStmt_k = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_k);
    	llvm::SmallVector<ASTValue *, 8> EmptyVals;
    	ASTArrayValue *EmptyArr = ASTBuilder::CreateArrayValue(SourceLoc, EmptyVals);
    	ASTIdentifier *kIdent = ASTBuilder::CreateIdentifier(LocalVar_k);
    	ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, kIdent, EmptyArr);
    	DeclStmt_k->setExpr(AssignExpr);

    	// Generate Code
    	Generate();
    	llvm::Module *M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

    	EXPECT_EQ(output, "define void @_F0(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  %12 = alloca i64*, align 8\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  store i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), [0 x i8]* %13, align 8\n"
						  "  store i8 48, i8* %14, align 1\n"
						  "}\n");
    }

    TEST_F(CodeGenTest, CGArrayLocalVarAssignValues) {
        /**
         * Fly code:
         * void func() {
         *   int[] a = {1, 2, 3} // array with values
         * }
         */
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // int[] a = {1,2,3}
    	ASTArrayType *ArrayIntType = ASTBuilder::CreateArrayType(SourceLoc, IntTypeRef, nullptr);
        ASTLocalVar *LocalVar = ASTBuilder::CreateLocalVar(SourceLoc, ArrayIntType, "a", EmptyModifiers);
        ASTDeclStmt *DeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar);
    	llvm::SmallVector<ASTValue *, 8> Vals;
        Vals.push_back(ASTBuilder::CreateNumberValue(SourceLoc, "1"));
    	Vals.push_back(ASTBuilder::CreateNumberValue(SourceLoc, "2"));
    	Vals.push_back(ASTBuilder::CreateNumberValue(SourceLoc, "3"));
    	ASTArrayValue *ArrValues = ASTBuilder::CreateArrayValue(SourceLoc, Vals);
        ASTIdentifier *aIdent = ASTBuilder::CreateIdentifier(LocalVar);
        ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, aIdent, ArrValues);
        DeclStmt->setExpr(AssignExpr);

        // Generate Code
    	Generate();
        llvm::Module *M = getModules()[0];
        std::string output = getOutput(M);

        EXPECT_EQ(output, "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  store i32 1, i32* %2, align 4\n"
                          "  ret void\n"
                          "}\n");
    }

	TEST_F(CodeGenTest, CGArrayLocalVarAssignSizeValues) {
    	/**
		 * Fly code:
		 * void func() {
		 *   int[3] a = {1, 2, 3} // array with values
		 * }
		 */
    	ASTModule *Module = CreateModule();

    	// func()
    	ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

    	// int[3] a = {1,2,3}
    	ASTNumberValue *Value_3 = ASTBuilder::CreateNumberValue(SourceLoc, "3");
    	ASTArrayType *ArrayIntType = ASTBuilder::CreateArrayType(SourceLoc, IntTypeRef, Value_3);
    	ASTLocalVar *LocalVar = ASTBuilder::CreateLocalVar(SourceLoc, ArrayIntType, "a", EmptyModifiers);
    	ASTDeclStmt *DeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar);
    	llvm::SmallVector<ASTValue *, 8> Vals;
    	Vals.push_back(ASTBuilder::CreateNumberValue(SourceLoc, "1"));
    	Vals.push_back(ASTBuilder::CreateNumberValue(SourceLoc, "2"));
    	Vals.push_back(ASTBuilder::CreateNumberValue(SourceLoc, "3"));
    	ASTArrayValue *ArrValues = ASTBuilder::CreateArrayValue(SourceLoc, Vals);
    	ASTIdentifier *aIdent = ASTBuilder::CreateIdentifier(LocalVar);
    	ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, aIdent, ArrValues);
    	DeclStmt->setExpr(AssignExpr);

    	// Generate Code
    	Generate();
    	llvm::Module *M = getModules()[0];
    	std::string output = getOutput(M);

    	EXPECT_EQ(output, "define void @_F0(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  %2 = alloca i32, align 4\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  store i32 1, i32* %2, align 4\n"
						  "  ret void\n"
						  "}\n");
    }
} // anonymous namespace
