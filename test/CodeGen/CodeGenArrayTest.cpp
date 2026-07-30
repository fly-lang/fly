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
#include "AST/ASTCall.h"
#include "AST/ASTClass.h"
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

    	// A zero-sized DECLARATION zeroes the WHOLE fat pointer, so both fields are
    	// defined: data = null and size = 0 (B044). The FIRST zeroing now comes from
    	// the alloca itself — scope cleanup must find a defined slot even for a
    	// declaration an early `return` never reached — and the zero-size path stores
    	// it again, which is redundant but harmless.
    	//
    	// The release at scope exit is the point of this test: it loads the data
    	// pointer, and the NULL GUARD (arel_null) sends it straight to done. Without
    	// that guard this very case — an array with no buffer — would dereference
    	// null looking for a refcount header.
    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        "  store %array zeroinitializer, ptr %2, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  store %array zeroinitializer, ptr %2, align 8\n"
                        "  %3 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  %arel_data = load ptr, ptr %3, align 8\n"
                        "  %arel_null = icmp eq ptr %arel_data, null\n"
                        "  br i1 %arel_null, label %arel_done, label %arel_live\n"
                        "\n"
                        "arel_live:                                        ; preds = %entry\n"
                        "  %arel_hdr = getelementptr i8, ptr %arel_data, i64 -8\n"
                        "  %arel_rc = load i64, ptr %arel_hdr, align 8\n"
                        "  %arel_rc1 = sub i64 %arel_rc, 1\n"
                        "  store i64 %arel_rc1, ptr %arel_hdr, align 8\n"
                        "  %arel_zero = icmp eq i64 %arel_rc1, 0\n"
                        "  br i1 %arel_zero, label %arel_free, label %arel_done\n"
                        "\n"
                        "arel_done:                                        ; preds = %arel_free, %arel_live, %entry\n"
                        "  ret void\n"
                        "\n"
                        "arel_free:                                        ; preds = %arel_live\n"
                        "  call void @free(ptr %arel_hdr)\n"
                        "  br label %arel_done\n"
                        "}\n"
                        "declare void @free(ptr)\n");
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
    	// An empty literal allocates NOTHING, so the release below finds a null data
    	// pointer and short-circuits on the guard — the case that makes the guard
    	// mandatory rather than defensive.
    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        "  store %array zeroinitializer, ptr %2, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %3 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  store ptr null, ptr %3, align 8\n"
                        "  %4 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 1\n"
                        "  store i32 0, ptr %4, align 4\n"
                        "  %5 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  %arel_data = load ptr, ptr %5, align 8\n"
                        "  %arel_null = icmp eq ptr %arel_data, null\n"
                        "  br i1 %arel_null, label %arel_done, label %arel_live\n"
                        "\n"
                        "arel_live:                                        ; preds = %entry\n"
                        "  %arel_hdr = getelementptr i8, ptr %arel_data, i64 -8\n"
                        "  %arel_rc = load i64, ptr %arel_hdr, align 8\n"
                        "  %arel_rc1 = sub i64 %arel_rc, 1\n"
                        "  store i64 %arel_rc1, ptr %arel_hdr, align 8\n"
                        "  %arel_zero = icmp eq i64 %arel_rc1, 0\n"
                        "  br i1 %arel_zero, label %arel_free, label %arel_done\n"
                        "\n"
                        "arel_done:                                        ; preds = %arel_free, %arel_live, %entry\n"
                        "  ret void\n"
                        "\n"
                        "arel_free:                                        ; preds = %arel_live\n"
                        "  call void @free(ptr %arel_hdr)\n"
                        "  br label %arel_done\n"
                        "}\n"
                        "declare void @free(ptr)\n");
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
    	// An empty literal allocates NOTHING, so the release below finds a null data
    	// pointer and short-circuits on the guard — the case that makes the guard
    	// mandatory rather than defensive.
    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        "  store %array zeroinitializer, ptr %2, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %3 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  store ptr null, ptr %3, align 8\n"
                        "  %4 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 1\n"
                        "  store i32 0, ptr %4, align 4\n"
                        "  %5 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  %arel_data = load ptr, ptr %5, align 8\n"
                        "  %arel_null = icmp eq ptr %arel_data, null\n"
                        "  br i1 %arel_null, label %arel_done, label %arel_live\n"
                        "\n"
                        "arel_live:                                        ; preds = %entry\n"
                        "  %arel_hdr = getelementptr i8, ptr %arel_data, i64 -8\n"
                        "  %arel_rc = load i64, ptr %arel_hdr, align 8\n"
                        "  %arel_rc1 = sub i64 %arel_rc, 1\n"
                        "  store i64 %arel_rc1, ptr %arel_hdr, align 8\n"
                        "  %arel_zero = icmp eq i64 %arel_rc1, 0\n"
                        "  br i1 %arel_zero, label %arel_free, label %arel_done\n"
                        "\n"
                        "arel_done:                                        ; preds = %arel_free, %arel_live, %entry\n"
                        "  ret void\n"
                        "\n"
                        "arel_free:                                        ; preds = %arel_live\n"
                        "  call void @free(ptr %arel_hdr)\n"
                        "  br label %arel_done\n"
                        "}\n"
                        "declare void @free(ptr)\n");
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

    	// The buffer is a REFERENCE-COUNTED block: malloc(8 + 3*4 = 20) covers an i64
    	// header plus the elements, the whole block is zeroed, the count starts at 1,
    	// and the fat pointer stores the DATA address (8 bytes in). Everything that
    	// reads an array — for-in, the subscript — only ever sees that data pointer,
    	// which is why none of them needed changing.
    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        // The slot is zeroed at the alloca: scope cleanup releases every
                        // array local in the frame, including ones an early `return`
                        // never declared, and must find a defined pointer there.
                        "  store %array zeroinitializer, ptr %2, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %rcbuf = call ptr @malloc(i64 20)\n"
                        "  call void @llvm.memset.p0.i64(ptr %rcbuf, i8 0, i64 20, i1 false)\n"
                        "  store i64 1, ptr %rcbuf, align 8\n"
                        "  %rcbuf_data = getelementptr i8, ptr %rcbuf, i64 8\n"
                        "  %3 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  store ptr %rcbuf_data, ptr %3, align 8\n"
                        "  %4 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 1\n"
                        // The size is stored at the FIELD's width. It used to be an i64
                        // store into this i32 field — 8 bytes into 4 — surviving only on
                        // the struct's tail padding.
                        "  store i32 3, ptr %4, align 4\n"
                        // …and the scope-exit release: drop one reference and free the
                        // whole block (header included, hence the -8) when it reaches
                        // zero. This buffer used to be allocated and never freed.
                        "  %5 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  %arel_data = load ptr, ptr %5, align 8\n"
                        "  %arel_null = icmp eq ptr %arel_data, null\n"
                        "  br i1 %arel_null, label %arel_done, label %arel_live\n"
                        "\n"
                        "arel_live:                                        ; preds = %entry\n"
                        "  %arel_hdr = getelementptr i8, ptr %arel_data, i64 -8\n"
                        "  %arel_rc = load i64, ptr %arel_hdr, align 8\n"
                        "  %arel_rc1 = sub i64 %arel_rc, 1\n"
                        "  store i64 %arel_rc1, ptr %arel_hdr, align 8\n"
                        "  %arel_zero = icmp eq i64 %arel_rc1, 0\n"
                        "  br i1 %arel_zero, label %arel_free, label %arel_done\n"
                        "\n"
                        "arel_done:                                        ; preds = %arel_free, %arel_live, %entry\n"
                        "  ret void\n"
                        "\n"
                        "arel_free:                                        ; preds = %arel_live\n"
                        "  call void @free(ptr %arel_hdr)\n"
                        "  br label %arel_done\n"
                        "}\n"
                        "declare ptr @malloc(i64)\n"
                        "; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)\n"
                        "declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #0\n"
                        "declare void @free(ptr)\n");
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

        // Reference-counted block: 8 bytes of header + 3*4 of elements, zeroed, count
        // at 1. The element stores index from the DATA pointer, not from the raw
        // allocation — that offset is the whole discipline behind the change.
        EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        // The slot is zeroed at the alloca: scope cleanup releases every
                        // array local in the frame, including ones an early `return`
                        // never declared, and must find a defined pointer there.
                        "  store %array zeroinitializer, ptr %2, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %rcbuf = call ptr @malloc(i64 20)\n"
                        "  call void @llvm.memset.p0.i64(ptr %rcbuf, i8 0, i64 20, i1 false)\n"
                        "  store i64 1, ptr %rcbuf, align 8\n"
                        "  %rcbuf_data = getelementptr i8, ptr %rcbuf, i64 8\n"
                        "  %3 = getelementptr i32, ptr %rcbuf_data, i64 0\n"
                        "  store i32 1, ptr %3, align 4\n"
                        "  %4 = getelementptr i32, ptr %rcbuf_data, i64 1\n"
                        "  store i32 2, ptr %4, align 4\n"
                        "  %5 = getelementptr i32, ptr %rcbuf_data, i64 2\n"
                        "  store i32 3, ptr %5, align 4\n"
                        // …then the variable's %array fat pointer is filled in with the
                        // buffer AND its element count (B025) — the size field used to be
                        // left uninitialised.
                        "  %6 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  store ptr %rcbuf_data, ptr %6, align 8\n"
                        "  %7 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 1\n"
                        "  store i32 3, ptr %7, align 4\n"
                        // …and the scope-exit release, which this buffer never had.
                        "  %8 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  %arel_data = load ptr, ptr %8, align 8\n"
                        "  %arel_null = icmp eq ptr %arel_data, null\n"
                        "  br i1 %arel_null, label %arel_done, label %arel_live\n"
                        "\n"
                        "arel_live:                                        ; preds = %entry\n"
                        "  %arel_hdr = getelementptr i8, ptr %arel_data, i64 -8\n"
                        "  %arel_rc = load i64, ptr %arel_hdr, align 8\n"
                        "  %arel_rc1 = sub i64 %arel_rc, 1\n"
                        "  store i64 %arel_rc1, ptr %arel_hdr, align 8\n"
                        "  %arel_zero = icmp eq i64 %arel_rc1, 0\n"
                        "  br i1 %arel_zero, label %arel_free, label %arel_done\n"
                        "\n"
                        "arel_done:                                        ; preds = %arel_free, %arel_live, %entry\n"
                        "  ret void\n"
                        "\n"
                        "arel_free:                                        ; preds = %arel_live\n"
                        "  call void @free(ptr %arel_hdr)\n"
                        "  br label %arel_done\n"
                        "}\n"
                        "declare ptr @malloc(i64)\n"
                        "; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)\n"
                        "declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #0\n"
                        "declare void @free(ptr)\n");
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

    	// Same reference-counted block as the unsized literal: the declared size and the
    	// literal's element count agree, so nothing extra is emitted for the `[3]`.
    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        // The slot is zeroed at the alloca: scope cleanup releases every
                        // array local in the frame, including ones an early `return`
                        // never declared, and must find a defined pointer there.
                        "  store %array zeroinitializer, ptr %2, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %rcbuf = call ptr @malloc(i64 20)\n"
                        "  call void @llvm.memset.p0.i64(ptr %rcbuf, i8 0, i64 20, i1 false)\n"
                        "  store i64 1, ptr %rcbuf, align 8\n"
                        "  %rcbuf_data = getelementptr i8, ptr %rcbuf, i64 8\n"
                        "  %3 = getelementptr i32, ptr %rcbuf_data, i64 0\n"
                        "  store i32 1, ptr %3, align 4\n"
                        "  %4 = getelementptr i32, ptr %rcbuf_data, i64 1\n"
                        "  store i32 2, ptr %4, align 4\n"
                        "  %5 = getelementptr i32, ptr %rcbuf_data, i64 2\n"
                        "  store i32 3, ptr %5, align 4\n"
                        // …then the variable's %array fat pointer is filled in with the
                        // buffer AND its element count (B025) — the size field used to be
                        // left uninitialised.
                        "  %6 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  store ptr %rcbuf_data, ptr %6, align 8\n"
                        "  %7 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 1\n"
                        "  store i32 3, ptr %7, align 4\n"
                        // …and the scope-exit release, which this buffer never had.
                        "  %8 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  %arel_data = load ptr, ptr %8, align 8\n"
                        "  %arel_null = icmp eq ptr %arel_data, null\n"
                        "  br i1 %arel_null, label %arel_done, label %arel_live\n"
                        "\n"
                        "arel_live:                                        ; preds = %entry\n"
                        "  %arel_hdr = getelementptr i8, ptr %arel_data, i64 -8\n"
                        "  %arel_rc = load i64, ptr %arel_hdr, align 8\n"
                        "  %arel_rc1 = sub i64 %arel_rc, 1\n"
                        "  store i64 %arel_rc1, ptr %arel_hdr, align 8\n"
                        "  %arel_zero = icmp eq i64 %arel_rc1, 0\n"
                        "  br i1 %arel_zero, label %arel_free, label %arel_done\n"
                        "\n"
                        "arel_done:                                        ; preds = %arel_free, %arel_live, %entry\n"
                        "  ret void\n"
                        "\n"
                        "arel_free:                                        ; preds = %arel_live\n"
                        "  call void @free(ptr %arel_hdr)\n"
                        "  br label %arel_done\n"
                        "}\n"
                        "declare ptr @malloc(i64)\n"
                        "; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)\n"
                        "declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #0\n"
                        "declare void @free(ptr)\n");
    }
	TEST_F(CodeGenTest, CGArrayBindLValueRetains) {
    	/**
		 * Fly code:
		 * void func() {
		 *   int[3] k
		 *   int[] j = k   // second owner of ONE buffer
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

    	// int[] j = k
    	ASTArrayType *ArrayIntType2 = ASTBuilder::CreateArrayType(SourceLoc, IntTypeRef, nullptr);
    	ASTLocalVar *LocalVar_j = ASTBuilder::CreateLocalVar(SourceLoc, ArrayIntType2, "j", EmptyModifiers);
    	ASTDeclStmt *DeclStmt_j = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_j);
    	ASTIdentifier *Ident_j = ASTBuilder::CreateIdentifier(LocalVar_j);
    	ASTIdentifier *Ident_k = ASTBuilder::CreateIdentifier(SourceLoc, "k");
    	ASTBinary *Bind = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, Ident_j, Ident_k);
    	DeclStmt_j->setExpr(Bind);

    	// Generate Code
    	Generate();
    	llvm::Module *M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

    	// This is the whole point of the reference count. Binding an array lvalue copies
    	// the fat pointer — never the buffer — and RETAINS, so the one block now has two
    	// owners and two scope-exit releases. Only the second brings the count to zero
    	// and frees. Without the retain here the first release would free a buffer the
    	// other name still points at.
    	//
    	// The retain is null-guarded (aret_null) for the same reason the release is: an
    	// empty array has no buffer, and binding one is legal.
    	//
    	// Read the counts along the path: malloc stores 1, aret_rc1 makes it 2, the
    	// first release (k's) brings it back to 1 and does NOT free, the second (j's)
    	// reaches 0 and frees. One malloc, one free, two owners.
    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        "  store %array zeroinitializer, ptr %2, align 8\n"
                        "  %3 = alloca %array, align 8\n"
                        "  store %array zeroinitializer, ptr %3, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        // int[3] k — the buffer, born with a count of 1
                        "  %rcbuf = call ptr @malloc(i64 20)\n"
                        "  call void @llvm.memset.p0.i64(ptr %rcbuf, i8 0, i64 20, i1 false)\n"
                        "  store i64 1, ptr %rcbuf, align 8\n"
                        "  %rcbuf_data = getelementptr i8, ptr %rcbuf, i64 8\n"
                        "  %4 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  store ptr %rcbuf_data, ptr %4, align 8\n"
                        "  %5 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 1\n"
                        "  store i32 3, ptr %5, align 4\n"
                        // int[] j = k — both fields are read out of k's slot. No memcpy
                        // of the elements appears anywhere: the buffer is shared, not
                        // copied, which is the whole reason arrays live on the heap.
                        "  %6 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  %arr.src.data = load ptr, ptr %6, align 8\n"
                        "  %7 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 1\n"
                        "  %arr.src.size = load i32, ptr %7, align 4\n"
                        // …the RETAIN, emitted BEFORE the store (and before any release
                        // of the destination) so that `j = j` can never drive the count
                        // through zero on a buffer that is about to be stored back.
                        "  %aret_null = icmp eq ptr %arr.src.data, null\n"
                        "  br i1 %aret_null, label %aret_done, label %aret_live\n"
                        "\n"
                        "aret_live:                                        ; preds = %entry\n"
                        "  %aret_hdr = getelementptr i8, ptr %arr.src.data, i64 -8\n"
                        "  %aret_rc = load i64, ptr %aret_hdr, align 8\n"
                        "  %aret_rc1 = add i64 %aret_rc, 1\n"
                        "  store i64 %aret_rc1, ptr %aret_hdr, align 8\n"
                        "  br label %aret_done\n"
                        "\n"
                        "aret_done:                                        ; preds = %aret_live, %entry\n"
                        "  %8 = getelementptr inbounds nuw %array, ptr %3, i32 0, i32 0\n"
                        "  store ptr %arr.src.data, ptr %8, align 8\n"
                        "  %9 = getelementptr inbounds nuw %array, ptr %3, i32 0, i32 1\n"
                        "  store i32 %arr.src.size, ptr %9, align 4\n"
                        // scope exit, k first: 2 → 1, so arel_free is NOT taken
                        "  %10 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  %arel_data = load ptr, ptr %10, align 8\n"
                        "  %arel_null = icmp eq ptr %arel_data, null\n"
                        "  br i1 %arel_null, label %arel_done, label %arel_live\n"
                        "\n"
                        "arel_live:                                        ; preds = %aret_done\n"
                        "  %arel_hdr = getelementptr i8, ptr %arel_data, i64 -8\n"
                        "  %arel_rc = load i64, ptr %arel_hdr, align 8\n"
                        "  %arel_rc1 = sub i64 %arel_rc, 1\n"
                        "  store i64 %arel_rc1, ptr %arel_hdr, align 8\n"
                        "  %arel_zero = icmp eq i64 %arel_rc1, 0\n"
                        "  br i1 %arel_zero, label %arel_free, label %arel_done\n"
                        "\n"
                        // …then j: 1 → 0, and this one frees the block
                        "arel_done:                                        ; preds = %arel_free, %arel_live, %aret_done\n"
                        "  %11 = getelementptr inbounds nuw %array, ptr %3, i32 0, i32 0\n"
                        "  %arel_data1 = load ptr, ptr %11, align 8\n"
                        "  %arel_null4 = icmp eq ptr %arel_data1, null\n"
                        "  br i1 %arel_null4, label %arel_done3, label %arel_live2\n"
                        "\n"
                        "arel_free:                                        ; preds = %arel_live\n"
                        "  call void @free(ptr %arel_hdr)\n"
                        "  br label %arel_done\n"
                        "\n"
                        "arel_live2:                                       ; preds = %arel_done\n"
                        "  %arel_hdr5 = getelementptr i8, ptr %arel_data1, i64 -8\n"
                        "  %arel_rc6 = load i64, ptr %arel_hdr5, align 8\n"
                        "  %arel_rc17 = sub i64 %arel_rc6, 1\n"
                        "  store i64 %arel_rc17, ptr %arel_hdr5, align 8\n"
                        "  %arel_zero9 = icmp eq i64 %arel_rc17, 0\n"
                        "  br i1 %arel_zero9, label %arel_free8, label %arel_done3\n"
                        "\n"
                        "arel_done3:                                       ; preds = %arel_free8, %arel_live2, %arel_done\n"
                        "  ret void\n"
                        "\n"
                        "arel_free8:                                       ; preds = %arel_live2\n"
                        "  call void @free(ptr %arel_hdr5)\n"
                        "  br label %arel_done3\n"
                        "}\n"
                        "declare ptr @malloc(i64)\n"
                        "; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)\n"
                        "declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #0\n"
                        "declare void @free(ptr)\n");
    }

	TEST_F(CodeGenTest, CGArrayArgumentDoesNotRetain) {
    	/**
		 * Fly code:
		 * void take(int[] a) {}
		 * void func() {
		 *   int[3] k
		 *   take(k)     // BORROW: no retain, no release
		 * }
		 */
    	ASTModule *Module = CreateModule();

    	// void take(int[] a)
    	ASTArrayType *ParamArrayType = ASTBuilder::CreateArrayType(SourceLoc, IntTypeRef, nullptr);
    	ASTParam *Param_a = ASTBuilder::CreateParam(SourceLoc, ParamArrayType, "a", EmptyModifiers);
    	llvm::SmallVector<ASTParam *, 8> TakeParams;
    	TakeParams.push_back(Param_a);
    	ASTBlockStmt *TakeBody = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *Take = ASTBuilder::CreateFunction(Module, SourceLoc, "take", TopModifiers, TakeParams, TakeBody);

    	// void func()
    	ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

    	// int[3] k
    	ASTNumberValue *Value_3 = ASTBuilder::CreateNumberValue(SourceLoc, "3");
    	ASTArrayType *ArrayIntType = ASTBuilder::CreateArrayType(SourceLoc, IntTypeRef, Value_3);
    	ASTLocalVar *LocalVar_k = ASTBuilder::CreateLocalVar(SourceLoc, ArrayIntType, "k", EmptyModifiers);
    	ASTDeclStmt *DeclStmt_k = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_k);

    	// take(k)
    	llvm::SmallVector<ASTExpr *, 8> Args;
    	Args.push_back(ASTBuilder::CreateIdentifier(SourceLoc, "k"));
    	ASTCall *Call = ASTBuilder::CreateCall(SourceLoc, "take", Args, ASTCallKind::CALL_DIRECT);
    	ASTExprStmt *CallStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
    	CallStmt->setExpr(Call);

    	// Generate Code
    	Generate();
    	llvm::Module *M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

    	// An argument is a BORROW: the callee sees the buffer for the duration of the
    	// call and never outlives it, so no retain is emitted and the parameter gets no
    	// release. Retaining here would raise a count nobody ever lowers — a leak, and
    	// the counterpart to the mistake the previous test guards against.
    	//
    	// Two things to read: `take`'s body contains no arel_* blocks at all, and the
    	// call site hands over `ptr %2` — the caller's slot address — with no aret_*
    	// before it. The only reference-count traffic in the whole module is the single
    	// release of `k` at the end of func.
    	EXPECT_EQ(output, "define void @_F4take_A_i(ptr %0, ptr %1) {\n"
                        "entry:\n"
                        "  %2 = alloca ptr, align 8\n"
                        "  store ptr %0, ptr %2, align 8\n"
                        // no release here: the parameter is borrowed, not owned
                        "  ret void\n"
                        "}\n"
                        "define void @_F4func(ptr %0) {\n"
                        "entry:\n"
                        "  %1 = alloca ptr, align 8\n"
                        "  %2 = alloca %array, align 8\n"
                        "  store %array zeroinitializer, ptr %2, align 8\n"
                        "  store ptr %0, ptr %1, align 8\n"
                        "  %rcbuf = call ptr @malloc(i64 20)\n"
                        "  call void @llvm.memset.p0.i64(ptr %rcbuf, i8 0, i64 20, i1 false)\n"
                        "  store i64 1, ptr %rcbuf, align 8\n"
                        "  %rcbuf_data = getelementptr i8, ptr %rcbuf, i64 8\n"
                        "  %3 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  store ptr %rcbuf_data, ptr %3, align 8\n"
                        "  %4 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 1\n"
                        "  store i32 3, ptr %4, align 4\n"
                        // the argument: the slot goes straight through, no aret_* above it
                        "  %5 = load ptr, ptr %1, align 8\n"
                        "  call void @_F4take_A_i(ptr %5, ptr %2)\n"
                        // the one and only release, k's, still leaving the count at zero
                        "  %6 = getelementptr inbounds nuw %array, ptr %2, i32 0, i32 0\n"
                        "  %arel_data = load ptr, ptr %6, align 8\n"
                        "  %arel_null = icmp eq ptr %arel_data, null\n"
                        "  br i1 %arel_null, label %arel_done, label %arel_live\n"
                        "\n"
                        "arel_live:                                        ; preds = %entry\n"
                        "  %arel_hdr = getelementptr i8, ptr %arel_data, i64 -8\n"
                        "  %arel_rc = load i64, ptr %arel_hdr, align 8\n"
                        "  %arel_rc1 = sub i64 %arel_rc, 1\n"
                        "  store i64 %arel_rc1, ptr %arel_hdr, align 8\n"
                        "  %arel_zero = icmp eq i64 %arel_rc1, 0\n"
                        "  br i1 %arel_zero, label %arel_free, label %arel_done\n"
                        "\n"
                        "arel_done:                                        ; preds = %arel_free, %arel_live, %entry\n"
                        "  ret void\n"
                        "\n"
                        "arel_free:                                        ; preds = %arel_live\n"
                        "  call void @free(ptr %arel_hdr)\n"
                        "  br label %arel_done\n"
                        "}\n"
                        "declare ptr @malloc(i64)\n"
                        "; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)\n"
                        "declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #0\n"
                        "declare void @free(ptr)\n");
    }
	TEST_F(CodeGenTest, CGArrayOfClassPreallocates) {
    	/**
		 * Fly code:
		 * class Cell { Cell() {} }
		 * void func() {
		 *   Cell[3] cells   // three DISTINCT instances, one per index
		 * }
		 */
    	ASTModule *Module = CreateModule();

    	// class Cell with a no-argument constructor
    	llvm::SmallVector<ASTType *, 4> NoSuper;
    	ASTClass *Cell = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS,
    		"Cell", TopModifiers, NoSuper);
    	ASTBuilder::CreateDefaultConstructor(Cell);

    	// void func() { Cell[3] cells }
    	ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

    	ASTType *CellRef = CreateType(Cell);
    	ASTNumberValue *Value_3 = ASTBuilder::CreateNumberValue(SourceLoc, "3");
    	ASTArrayType *ArrayCellType = ASTBuilder::CreateArrayType(SourceLoc, CellRef, Value_3);
    	ASTLocalVar *LocalVar = ASTBuilder::CreateLocalVar(SourceLoc, ArrayCellType, "cells", EmptyModifiers);
    	ASTDeclStmt *DeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar);

    	// Generate Code
    	Generate();
    	llvm::Module *M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

    	// Two things are being pinned here.
    	//
    	// FIRST, the buffer is sized in POINTERS: malloc(8 + 3*8 = 32), not
    	// 8 + 3*sizeof(%Cell). An array of objects holds references, so releasing it
    	// drops the pointers and never the objects — and striding by the struct would
    	// have walked past every element after the first.
    	//
    	// SECOND, every index is CONSTRUCTED. Before this the buffer was a row of
    	// zeroed structs whose vtable slot was null, so the first method call on an
    	// element dereferenced null. The construction is a counted loop (anew.*), not
    	// three unrolled copies, because the size can be a runtime value.
    	auto has = [&](const char *Needle) {
    		return output.find(Needle) != std::string::npos;
    	};
    	EXPECT_TRUE(has("call ptr @malloc(i64 32)")) << output;
    	EXPECT_TRUE(has("anew.cond:")) << output;
    	EXPECT_TRUE(has("%anew.obj = call ptr @malloc")) << output;
    	EXPECT_TRUE(has("call ptr @Cell.init_ctor")) << output;
    	// …and the constructor itself runs on every instance.
    	EXPECT_TRUE(has("call void @Cell_F4Cell")) << output;
    	// …the pointer — not a copy of the object — is what lands in the slot.
    	EXPECT_TRUE(has("%anew.slot = getelementptr ptr")) << output;
    	// The array still releases its own buffer, and still only that: no free of any
    	// element appears in the cleanup.
    	EXPECT_TRUE(has("call void @free(ptr %arel_hdr)")) << output;
    }
	TEST_F(CodeGenTest, CGNestedArrayReleasesInnerBuffers) {
    	/**
		 * Fly code:
		 * void func() {
		 *   int[][] rows = {{1}, {2, 3}}
		 * }
		 */
    	ASTModule *Module = CreateModule();

    	ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

    	// int[][] rows = {{1}, {2, 3}}
    	ASTArrayType *InnerType = ASTBuilder::CreateArrayType(SourceLoc, IntTypeRef, nullptr);
    	ASTArrayType *OuterType = ASTBuilder::CreateArrayType(SourceLoc, InnerType, nullptr);
    	ASTLocalVar *LocalVar = ASTBuilder::CreateLocalVar(SourceLoc, OuterType, "rows", EmptyModifiers);
    	ASTDeclStmt *DeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar);

    	llvm::SmallVector<ASTExpr *, 8> Row0;
    	Row0.push_back(ASTBuilder::CreateNumberValue(SourceLoc, "1"));
    	llvm::SmallVector<ASTExpr *, 8> Row1;
    	Row1.push_back(ASTBuilder::CreateNumberValue(SourceLoc, "2"));
    	Row1.push_back(ASTBuilder::CreateNumberValue(SourceLoc, "3"));
    	llvm::SmallVector<ASTExpr *, 8> Rows;
    	Rows.push_back(ASTBuilder::CreateArrayValue(SourceLoc, Row0));
    	Rows.push_back(ASTBuilder::CreateArrayValue(SourceLoc, Row1));
    	ASTArrayValue *RowsValue = ASTBuilder::CreateArrayValue(SourceLoc, Rows);

    	ASTIdentifier *Ident = ASTBuilder::CreateIdentifier(LocalVar);
    	ASTBinary *Assign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, Ident, RowsValue);
    	DeclStmt->setExpr(Assign);

    	// Generate Code
    	Generate();
    	llvm::Module *M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

    	auto has = [&](const char *Needle) {
    		return output.find(Needle) != std::string::npos;
    	};

    	// A nested array is one buffer and one count PER LEVEL: three mallocs here, the
    	// outer one holding two %array fat pointers (8 header + 2*16 = 40 bytes).
    	EXPECT_TRUE(has("call ptr @malloc(i64 40)")) << output;

    	// The release walks the elements BEFORE freeing the buffer that holds them, and
    	// only once the outer count has reached zero — inside arel_free, not on every
    	// scope exit, or a second owner would free the inner buffers twice.
    	EXPECT_TRUE(has("arel.inner.cond:")) << output;
    	EXPECT_TRUE(has("%arel.elem = getelementptr %array")) << output;
    	// …and what each element receives is a DECREMENT of its own count, the same
    	// release applied one level down — not a free. That is why this does not
    	// contradict the ownership rule.
    	EXPECT_TRUE(has("%arel_rc1")) << output;
    	// Two levels of release, so two headers are freed: the inner buffer's, from
    	// inside the walk, and the outer buffer's, after it. One free would mean the
    	// inner buffers were being abandoned — the leak this step closes.
    	size_t Frees = 0;
    	for (size_t P = output.find("call void @free(ptr %arel_hdr");
    	     P != std::string::npos;
    	     P = output.find("call void @free(ptr %arel_hdr", P + 1))
    		Frees++;
    	EXPECT_EQ(Frees, 2u) << output;
    }
} // anonymous namespace
