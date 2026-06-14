//===--------------------------------------------------------------------------------------------------------------===//
// test/CodeGen/CodeGenStringTest.cpp - CodeGen String tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

// fly
#include "AST/ASTParam.h"
#include "AST/ASTBinary.h"
#include "AST/ASTExprStmt.h"
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

    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\nentry:\n  %1 = alloca ptr, align 8\n  %2 = alloca %string, align 8\n  store %string zeroinitializer, ptr %2, align 8\n  store ptr %0, ptr %1, align 8\n  ret void\n}\n");
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

    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\nentry:\n  %1 = alloca ptr, align 8\n  %2 = alloca %string, align 8\n  store %string zeroinitializer, ptr %2, align 8\n  store ptr %0, ptr %1, align 8\n  %str_heap = call ptr @malloc(i64 0)\n  call void @llvm.memcpy.p0.p0.i64(ptr %str_heap, ptr @1, i64 0, i1 false)\n  %3 = insertvalue %string undef, ptr %str_heap, 0\n  %4 = insertvalue %string %3, i32 0, 1\n  store %string %4, ptr %2, align 8\n  %5 = load %string, ptr %2, align 8\n  %hs_ptr = extractvalue %string %5, 0\n  call void @free(ptr %hs_ptr)\n  ret void\n}\ndeclare ptr @malloc(i64)\n; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)\ndeclare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0\ndeclare void @free(ptr)\n");
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

    	EXPECT_EQ(output, "define void @_F4func(ptr %0) {\nentry:\n  %1 = alloca ptr, align 8\n  %2 = alloca %string, align 8\n  store %string zeroinitializer, ptr %2, align 8\n  store ptr %0, ptr %1, align 8\n  %str_heap = call ptr @malloc(i64 6)\n  call void @llvm.memcpy.p0.p0.i64(ptr %str_heap, ptr @1, i64 6, i1 false)\n  %3 = insertvalue %string undef, ptr %str_heap, 0\n  %4 = insertvalue %string %3, i32 6, 1\n  store %string %4, ptr %2, align 8\n  %5 = load %string, ptr %2, align 8\n  %hs_ptr = extractvalue %string %5, 0\n  call void @free(ptr %hs_ptr)\n  ret void\n}\ndeclare ptr @malloc(i64)\n; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)\ndeclare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0\ndeclare void @free(ptr)\n");
    }

    TEST_F(CodeGenTest, CGStringConcatNonConst) {
        /**
         * Fly code:
         * void func(string a, string b) {
         *   string k = a + b
         * }
         */
        ASTModule *Module = CreateModule();

        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *Param_a = ASTBuilder::CreateParam(SourceLoc, StringTypeRef, "a", EmptyModifiers);
        ASTParam *Param_b = ASTBuilder::CreateParam(SourceLoc, StringTypeRef, "b", EmptyModifiers);
        Params.push_back(Param_a);
        Params.push_back(Param_b);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // string k = a + b
        ASTLocalVar *LocalVar_k = ASTBuilder::CreateLocalVar(SourceLoc, StringTypeRef, "k", EmptyModifiers);
        ASTIdentifier *Ident_k = ASTBuilder::CreateIdentifier(LocalVar_k);
        ASTDeclStmt *DeclStmt_k = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_k);
        ASTIdentifier *Ident_a = ASTBuilder::CreateIdentifier(Param_a);
        ASTIdentifier *Ident_b = ASTBuilder::CreateIdentifier(Param_b);
        ASTBinary *Concat = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_ADD, Ident_a, Ident_b);
        ASTBinary *Assign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, Ident_k, Concat);
        DeclStmt_k->setExpr(Assign);

        Generate();
        llvm::Module *M = getModules()[0];
        std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_Ss_Ss(ptr %0, ptr %1, ptr %2) {\nentry:\n  %3 = alloca ptr, align 8\n  %4 = alloca %string, align 8\n  store %string zeroinitializer, ptr %4, align 8\n  store ptr %0, ptr %3, align 8\n  %5 = load %string, ptr %1, align 8\n  %6 = load %string, ptr %2, align 8\n  %s1_ptr = extractvalue %string %5, 0\n  %s1_size = extractvalue %string %5, 1\n  %s2_ptr = extractvalue %string %6, 0\n  %s2_size = extractvalue %string %6, 1\n  %str_total = add i32 %s1_size, %s2_size\n  %str_total_ext = zext i32 %str_total to i64\n  %s1_size_ext = zext i32 %s1_size to i64\n  %s2_size_ext = zext i32 %s2_size to i64\n  %str_buf = call ptr @malloc(i64 %str_total_ext)\n  call void @llvm.memcpy.p0.p0.i64(ptr %str_buf, ptr %s1_ptr, i64 %s1_size_ext, i1 false)\n  %str_buf2 = getelementptr i8, ptr %str_buf, i64 %s1_size_ext\n  call void @llvm.memcpy.p0.p0.i64(ptr %str_buf2, ptr %s2_ptr, i64 %s2_size_ext, i1 false)\n  %7 = insertvalue %string undef, ptr %str_buf, 0\n  %8 = insertvalue %string %7, i32 %str_total, 1\n  store %string %8, ptr %4, align 8\n  %9 = load %string, ptr %4, align 8\n  %hs_ptr = extractvalue %string %9, 0\n  call void @free(ptr %hs_ptr)\n  ret void\n}\ndeclare ptr @malloc(i64)\n; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)\ndeclare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0\ndeclare void @free(ptr)\n");
    }

    TEST_F(CodeGenTest, CGStringConcatConst) {
        /**
         * Fly code:
         * void func(string a, string b) {
         *   const string k = a + b
         * }
         */
        ASTModule *Module = CreateModule();

        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *Param_a = ASTBuilder::CreateParam(SourceLoc, StringTypeRef, "a", EmptyModifiers);
        ASTParam *Param_b = ASTBuilder::CreateParam(SourceLoc, StringTypeRef, "b", EmptyModifiers);
        Params.push_back(Param_a);
        Params.push_back(Param_b);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // const string k = a + b
        llvm::SmallVector<ASTModifier *, 8> ConstModifiers;
        ConstModifiers.push_back(ASTBuilder::CreateModifier(SourceLoc, ASTModifierKind::MOD_CONSTANT));
        ASTLocalVar *LocalVar_k = ASTBuilder::CreateLocalVar(SourceLoc, StringTypeRef, "k", ConstModifiers);
        ASTIdentifier *Ident_k = ASTBuilder::CreateIdentifier(LocalVar_k);
        ASTDeclStmt *DeclStmt_k = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_k);
        ASTIdentifier *Ident_a = ASTBuilder::CreateIdentifier(Param_a);
        ASTIdentifier *Ident_b = ASTBuilder::CreateIdentifier(Param_b);
        ASTBinary *Concat = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_ADD, Ident_a, Ident_b);
        ASTBinary *Assign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, Ident_k, Concat);
        DeclStmt_k->setExpr(Assign);

        Generate();
        llvm::Module *M = getModules()[0];
        std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_Ss_Ss(ptr %0, ptr %1, ptr %2) {\nentry:\n  %3 = alloca ptr, align 8\n  %4 = alloca %string, align 8\n  store %string zeroinitializer, ptr %4, align 8\n  store ptr %0, ptr %3, align 8\n  %5 = load %string, ptr %1, align 8\n  %6 = load %string, ptr %2, align 8\n  %s1_ptr = extractvalue %string %5, 0\n  %s1_size = extractvalue %string %5, 1\n  %s2_ptr = extractvalue %string %6, 0\n  %s2_size = extractvalue %string %6, 1\n  %str_total = add i32 %s1_size, %s2_size\n  %str_total_ext = zext i32 %str_total to i64\n  %s1_size_ext = zext i32 %s1_size to i64\n  %s2_size_ext = zext i32 %s2_size to i64\n  %str_buf = call ptr @malloc(i64 %str_total_ext)\n  call void @llvm.memcpy.p0.p0.i64(ptr %str_buf, ptr %s1_ptr, i64 %s1_size_ext, i1 false)\n  %str_buf2 = getelementptr i8, ptr %str_buf, i64 %s1_size_ext\n  call void @llvm.memcpy.p0.p0.i64(ptr %str_buf2, ptr %s2_ptr, i64 %s2_size_ext, i1 false)\n  %7 = insertvalue %string undef, ptr %str_buf, 0\n  %8 = insertvalue %string %7, i32 %str_total, 1\n  store %string %8, ptr %4, align 8\n  ret void\n}\ndeclare ptr @malloc(i64)\n; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)\ndeclare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0\n");
    }

    TEST_F(CodeGenTest, CGStringReassignConcat) {
        /**
         * Fly code:
         * void func() {
         *   string s = "hello"    → s.ptr = heap1
         *   s = s + " world"      → free(heap1), then s.ptr = heap2
         *   // scope exit: free(heap2)
         * }
         */
        ASTModule *Module = CreateModule();

        llvm::SmallVector<ASTParam *, 8> Params;
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // string s = "hello"
        ASTLocalVar *LocalVar_s = ASTBuilder::CreateLocalVar(SourceLoc, StringTypeRef, "s", EmptyModifiers);
        ASTIdentifier *Ident_s_decl = ASTBuilder::CreateIdentifier(LocalVar_s);
        ASTDeclStmt *DeclStmt_s = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_s);
        ASTStringValue *HelloStr = ASTBuilder::CreateStringValue(SourceLoc, "hello");
        ASTBinary *DeclAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, Ident_s_decl, HelloStr);
        DeclStmt_s->setExpr(DeclAssign);

        // s = s + " world"
        ASTIdentifier *Ident_s_lhs = ASTBuilder::CreateIdentifier(SourceLoc, "s");
        ASTIdentifier *Ident_s_rhs = ASTBuilder::CreateIdentifier(SourceLoc, "s");
        ASTStringValue *WorldStr = ASTBuilder::CreateStringValue(SourceLoc, " world");
        ASTBinary *Concat = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_ADD, Ident_s_rhs, WorldStr);
        ASTBinary *ReAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, Ident_s_lhs, Concat);
        ASTExprStmt *ExprStmt_s = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ExprStmt_s->setExpr(ReAssign);

        Generate();
        llvm::Module *M = getModules()[0];
        std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func(ptr %0) {\nentry:\n  %1 = alloca ptr, align 8\n  %2 = alloca %string, align 8\n  store %string zeroinitializer, ptr %2, align 8\n  store ptr %0, ptr %1, align 8\n  %str_heap = call ptr @malloc(i64 5)\n  call void @llvm.memcpy.p0.p0.i64(ptr %str_heap, ptr @1, i64 5, i1 false)\n  %3 = insertvalue %string undef, ptr %str_heap, 0\n  %4 = insertvalue %string %3, i32 5, 1\n  store %string %4, ptr %2, align 8\n  %5 = load %string, ptr %2, align 8\n  %6 = extractvalue %string %5, 0\n  call void @free(ptr %6)\n  %s1_ptr = extractvalue %string %5, 0\n  %s1_size = extractvalue %string %5, 1\n  %str_total = add i32 %s1_size, 6\n  %str_total_ext = zext i32 %str_total to i64\n  %s1_size_ext = zext i32 %s1_size to i64\n  %str_buf = call ptr @malloc(i64 %str_total_ext)\n  call void @llvm.memcpy.p0.p0.i64(ptr %str_buf, ptr %s1_ptr, i64 %s1_size_ext, i1 false)\n  %str_buf2 = getelementptr i8, ptr %str_buf, i64 %s1_size_ext\n  call void @llvm.memcpy.p0.p0.i64(ptr %str_buf2, ptr @2, i64 6, i1 false)\n  %7 = insertvalue %string undef, ptr %str_buf, 0\n  %8 = insertvalue %string %7, i32 %str_total, 1\n  store %string %8, ptr %2, align 8\n  %9 = load %string, ptr %2, align 8\n  %hs_ptr = extractvalue %string %9, 0\n  call void @free(ptr %hs_ptr)\n  ret void\n}\ndeclare ptr @malloc(i64)\n; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)\ndeclare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0\ndeclare void @free(ptr)\n");
    }

 } // anonymous namespace
