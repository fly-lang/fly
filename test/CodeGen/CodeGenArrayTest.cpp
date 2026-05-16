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
         *   int[] k // Error: incomplete type
         * }
         */
        ASTModule *Module = CreateModule();

        // Build function with an array parameter: void func(int[] k) {}
        llvm::SmallVector<ASTParam *, 8> LocalParams;
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, LocalParams, Body);

    	ASTArrayType *ArrayIntType = ASTBuilder::CreateArrayType(SourceLoc, IntTypeRef, nullptr);
    	ASTLocalVar *LocalVar_k = ASTBuilder::CreateLocalVar(SourceLoc, ArrayIntType, "k", EmptyModifiers);
    	ASTDeclStmt *DeclStmt_k = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_k);

        // Generate Code
    	ASSERT_FALSE(Resolve());
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
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, LocalParams, Body);

    	ASTNumberValue *ZeroValue = ASTBuilder::CreateNumberValue(SourceLoc, "0");
    	ASTArrayType *ArrayIntType = ASTBuilder::CreateArrayType(SourceLoc, IntTypeRef, ZeroValue);
    	ASTLocalVar *LocalVar_k = ASTBuilder::CreateLocalVar(SourceLoc, ArrayIntType, "k", EmptyModifiers);
    	ASTDeclStmt *DeclStmt_k = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_k);

    	// Generate Code
    	Generate();
    	llvm::Module *M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  store ptr null, ptr %2, align 8\n"
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
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

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

    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  store ptr null, ptr %2, align 8\n"
                        "  ret void\n"
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
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

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

    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  store ptr null, ptr %2, align 8\n"
                        "  ret void\n"
                        "}\n");
    }

	TEST_F(CodeGenTest, CGArrayLocalVar3) {
    	/**
		 * Fly code:
		 * void func() {
		 *   int[3] k // array of size 3
		 * }
		 */
    	ASTModule *Module = CreateModule();

    	ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

    	// int[3] k
    	ASTNumberValue *Value_3 = ASTBuilder::CreateNumberValue(SourceLoc, "3");
    	ASTArrayType *ArrayIntType = ASTBuilder::CreateArrayType(SourceLoc, IntTypeRef, Value_3);
    	ASTLocalVar *LocalVar_k = ASTBuilder::CreateLocalVar(SourceLoc, ArrayIntType, "k", EmptyModifiers);
    	ASTDeclStmt *DeclStmt_k = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_k);

    	// Generate Code
    	Generate();
    	llvm::Module *M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %3 = call ptr @malloc(i64 12)\n"
                        "  call void @llvm.memset.p0.i8(ptr %3, i8 0, i8 12, i1 false)\n"
                        "  %4 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  store ptr %3, ptr %4, align 8\n"
                        "  %5 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 1\n"
                        "  store i64 3, ptr %5, align 8\n"
                        "  ret void\n"
                        "}\n"
                        "declare ptr @malloc(i64)\n"
                        "; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)\n"
                        "declare void @llvm.memset.p0.i8(ptr nocapture writeonly, i8, i8, i1 immarg) #0\n");
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
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

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
        std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %3 = call ptr @malloc(i64 12)\n"
                        "  %4 = getelementptr i32, ptr %3, i64 0\n"
                        "  store i32 1, ptr %4, align 4\n"
                        "  %5 = getelementptr i32, ptr %3, i64 1\n"
                        "  store i32 2, ptr %5, align 4\n"
                        "  %6 = getelementptr i32, ptr %3, i64 2\n"
                        "  store i32 3, ptr %6, align 4\n"
                        "  ret void\n"
                        "}\n"
                        "declare ptr @malloc(i64)\n");
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
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

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
    	std::string output = getOutput(M->getFunctionList());

    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %3 = call ptr @malloc(i64 12)\n"
                        "  %4 = getelementptr i32, ptr %3, i64 0\n"
                        "  store i32 1, ptr %4, align 4\n"
                        "  %5 = getelementptr i32, ptr %3, i64 1\n"
                        "  store i32 2, ptr %5, align 4\n"
                        "  %6 = getelementptr i32, ptr %3, i64 2\n"
                        "  store i32 3, ptr %6, align 4\n"
                        "  ret void\n"
                        "}\n"
                        "declare ptr @malloc(i64)\n");
    }
} // anonymous namespace
