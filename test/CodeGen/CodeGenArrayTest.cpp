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

    	// A bare DECLARATION (no initializer) takes StoreDefaultValue, which only
    	// nulls the slot — unlike an assigned literal, which goes through
    	// StoreArrayValue and writes both {data, size} fields (see the next test).
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
    	llvm::SmallVector<ASTExpr *, 8> EmptyVals;
    	ASTArrayValue *EmptyArr = ASTBuilder::CreateArrayValue(SourceLoc, EmptyVals);
    	ASTIdentifier *kIdent = ASTBuilder::CreateIdentifier(LocalVar_k);
    	ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, kIdent, EmptyArr);
    	DeclStmt_k->setExpr(AssignExpr);

    	// Generate Code
    	Generate();
    	llvm::Module *M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

    	// The %array fat pointer is initialised in BOTH fields (B025): an empty
    	// literal gets {data = null, size = 0}. Writing only the data pointer left
    	// the size garbage, so every for-in over the variable read a random count.
    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %3 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  store ptr null, ptr %3, align 8\n"
                        "  %4 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 1\n"
                        "  store i32 0, ptr %4, align 4\n"
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
    	llvm::SmallVector<ASTExpr *, 8> EmptyVals;
    	ASTArrayValue *EmptyArr = ASTBuilder::CreateArrayValue(SourceLoc, EmptyVals);
    	ASTIdentifier *kIdent = ASTBuilder::CreateIdentifier(LocalVar_k);
    	ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, kIdent, EmptyArr);
    	DeclStmt_k->setExpr(AssignExpr);

    	// Generate Code
    	Generate();
    	llvm::Module *M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

    	// The %array fat pointer is initialised in BOTH fields (B025): an empty
    	// literal gets {data = null, size = 0}. Writing only the data pointer left
    	// the size garbage, so every for-in over the variable read a random count.
    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %3 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  store ptr null, ptr %3, align 8\n"
                        "  %4 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 1\n"
                        "  store i32 0, ptr %4, align 4\n"
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

    	// The memset length is i32: the size literal is typed int (CreateNumberValue
    	// types every in-range literal as int, no more minimal byte/short widths).
    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %3 = call ptr @malloc(i64 12)\n"
                        "  call void @llvm.memset.p0.i32(ptr %3, i8 0, i32 12, i1 false)\n"
                        "  %4 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  store ptr %3, ptr %4, align 8\n"
                        "  %5 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 1\n"
                        "  store i64 3, ptr %5, align 8\n"
                        "  ret void\n"
                        "}\n"
                        "declare ptr @malloc(i64)\n"
                        "; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)\n"
                        "declare void @llvm.memset.p0.i32(ptr nocapture writeonly, i8, i32, i1 immarg) #0\n");
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
    	llvm::SmallVector<ASTExpr *, 8> Vals;
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
                        // …then the variable's %array fat pointer is filled in with the
                        // buffer AND its element count (B025) — the size field used to be
                        // left uninitialised.
                        "  %7 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  store ptr %3, ptr %7, align 8\n"
                        "  %8 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 1\n"
                        "  store i32 3, ptr %8, align 4\n"
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
    	llvm::SmallVector<ASTExpr *, 8> Vals;
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
                        // …then the variable's %array fat pointer is filled in with the
                        // buffer AND its element count (B025) — the size field used to be
                        // left uninitialised.
                        "  %7 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  store ptr %3, ptr %7, align 8\n"
                        "  %8 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 1\n"
                        "  store i32 3, ptr %8, align 4\n"
                        "  ret void\n"
                        "}\n"
                        "declare ptr @malloc(i64)\n");
    }
} // anonymous namespace
