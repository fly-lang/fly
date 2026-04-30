//===--------------------------------------------------------------------------------------------------------------===//
// test/CodeGen/CodeGenUniqueTest.cpp - Unique pointer CodeGen tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBinary.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTValue.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGenTest.h"

#include <AST/ASTAttribute.h>
#include <AST/ASTBlockStmt.h>
#include <AST/ASTDeclStmt.h>
#include <AST/ASTExprStmt.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTMethod.h>
#include <AST/ASTParam.h>
#include <AST/ASTReturnStmt.h>

namespace {

    using namespace fly;

    /**
     * Fly code:
     * struct TestStruct { int a }
     * void func() {
     *   TestStruct t = new unique TestStruct()
     * }  // free(t) emitted automatically at scope exit
     */
    TEST_F(CodeGenTest, CGUniqueSmartAlloc) {
        ASTModule *Module = CreateModule();

        // struct TestStruct { int a }
        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT,
                                                       "TestStruct", TopModifiers, SuperClasses);
        ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

        // void func() {
        //   TestStruct t = new unique TestStruct()
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        ASTType *TestStructType = CreateType(TestStruct);
        ASTLocalVar *TVar = ASTBuilder::CreateLocalVar(SourceLoc, TestStructType, "t", EmptyModifiers);
        ASTDeclStmt *TDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TVar);
        ASTCall *CtorCall = ASTBuilder::CreateCall(SourceLoc, TestStruct->getName(), Args,
                                                   ASTCallKind::CALL_NEW_UNIQUE);
        ASTIdentifier *TIdent = ASTBuilder::CreateIdentifier(TVar);
        ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                         TIdent, CtorCall);
        TDeclStmt->setExpr(AssignExpr);

        Generate();
        llvm::Module *M = getModules()[0];
        std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i32, ptr, ptr }\n"
                          "%TestStruct = type { i32 }\n"
                          "\n"
                          "@error = external constant %error\n"
                          "\n"
                          "define ptr @TestStruct.init_ctor(ptr %0) {\n"
                          "entry:\n"
                          "  %1 = alloca ptr, align 8\n"
                          "  store ptr %0, ptr %1, align 8\n"
                          "  %2 = load ptr, ptr %1, align 8\n"
                          "  %3 = getelementptr inbounds %TestStruct, ptr %2, i32 0, i32 0\n"
                          "  store i32 0, ptr %3, align 4\n"
                          "  ret ptr %2\n"
                          "}\n"
                          "\n"
                          "define void @_F4func(ptr %0) {\n"
                          "entry:\n"
                          "  %1 = alloca ptr, align 8\n"
                          "  %2 = alloca ptr, align 8\n"
                          "  store ptr %0, ptr %1, align 8\n"
                          "  %3 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%TestStruct, ptr null, i32 1) to i64))\n"
                          "  %4 = call ptr @TestStruct.init_ctor(ptr %3)\n"
                          "  store ptr %4, ptr %2, align 8\n"
                          "  call void @free(ptr %4)\n"
                          "  ret void\n"
                          "}\n"
                          "\n"
                          "declare ptr @malloc(i64)\n"
                          "\n"
                          "declare void @free(ptr)\n");
    }

    /**
     * Fly code:
     * struct TestStruct { int a }
     * void func() {
     *   TestStruct t            // declaration without initializer
     *   t = new unique TestStruct()  // assigned in a second moment via ASTExprStmt
     * }  // free(t) emitted automatically at scope exit
     */
    TEST_F(CodeGenTest, CGUniqueSmartAllocLate) {
        ASTModule *Module = CreateModule();

        // struct TestStruct { int a }
        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT,
                                                       "TestStruct", TopModifiers, SuperClasses);
        ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

        // void func() {
        //   TestStruct t
        //   t = new unique TestStruct()
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // TestStruct t  (no initializer)
        ASTType *TestStructType = CreateType(TestStruct);
        ASTLocalVar *TVar = ASTBuilder::CreateLocalVar(SourceLoc, TestStructType, "t", EmptyModifiers);
        ASTBuilder::CreateDeclStmt(Body, SourceLoc, TVar);

        // t = new unique TestStruct()  (assigned later via ASTExprStmt)
        ASTCall *CtorCall = ASTBuilder::CreateCall(SourceLoc, TestStruct->getName(), Args,
                                                   ASTCallKind::CALL_NEW_UNIQUE);
        ASTIdentifier *TIdent = ASTBuilder::CreateIdentifier(TVar);
        ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                         TIdent, CtorCall);
        ASTExprStmt *ExprStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ExprStmt->setExpr(AssignExpr);

        Generate();
        llvm::Module *M = getModules()[0];
        std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i32, ptr, ptr }\n"
                          "%TestStruct = type { i32 }\n"
                          "\n"
                          "@error = external constant %error\n"
                          "\n"
                          "define ptr @TestStruct.init_ctor(ptr %0) {\n"
                          "entry:\n"
                          "  %1 = alloca ptr, align 8\n"
                          "  store ptr %0, ptr %1, align 8\n"
                          "  %2 = load ptr, ptr %1, align 8\n"
                          "  %3 = getelementptr inbounds %TestStruct, ptr %2, i32 0, i32 0\n"
                          "  store i32 0, ptr %3, align 4\n"
                          "  ret ptr %2\n"
                          "}\n"
                          "\n"
                          "define void @_F4func(ptr %0) {\n"
                          "entry:\n"
                          "  %1 = alloca ptr, align 8\n"
                          "  %2 = alloca ptr, align 8\n"
                          "  store ptr %0, ptr %1, align 8\n"
                          "  store ptr null, ptr %2, align 8\n"
                          "  %3 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%TestStruct, ptr null, i32 1) to i64))\n"
                          "  %4 = call ptr @TestStruct.init_ctor(ptr %3)\n"
                          "  store ptr %4, ptr %2, align 8\n"
                          "  call void @free(ptr %4)\n"
                          "  ret void\n"
                          "}\n"
                          "\n"
                          "declare ptr @malloc(i64)\n"
                          "\n"
                          "declare void @free(ptr)\n");
    }

    /**
     * Fly code:
     * struct TestStruct { int a }
     * void func() {
     *   TestStruct t = new shared TestStruct()
     * }  // shared_release(t) at scope exit; refcount==1→0 → free header
     */
    TEST_F(CodeGenTest, CGSharedSmartAlloc) {
        ASTModule *Module = CreateModule();

        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT,
                                                       "TestStruct", TopModifiers, SuperClasses);
        ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        ASTType *TestStructType = CreateType(TestStruct);
        ASTLocalVar *TVar = ASTBuilder::CreateLocalVar(SourceLoc, TestStructType, "t", EmptyModifiers);
        ASTDeclStmt *TDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TVar);
        ASTCall *CtorCall = ASTBuilder::CreateCall(SourceLoc, TestStruct->getName(), Args,
                                                   ASTCallKind::CALL_NEW_SHARED);
        ASTIdentifier *TIdent = ASTBuilder::CreateIdentifier(TVar);
        ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                         TIdent, CtorCall);
        TDeclStmt->setExpr(AssignExpr);

        Generate();
        llvm::Module *M = getModules()[0];
        std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i32, ptr, ptr }\n"
                          "%TestStruct = type { i32 }\n"
                          "\n"
                          "@error = external constant %error\n"
                          "\n"
                          "define ptr @TestStruct.init_ctor(ptr %0) {\n"
                          "entry:\n"
                          "  %1 = alloca ptr, align 8\n"
                          "  store ptr %0, ptr %1, align 8\n"
                          "  %2 = load ptr, ptr %1, align 8\n"
                          "  %3 = getelementptr inbounds %TestStruct, ptr %2, i32 0, i32 0\n"
                          "  store i32 0, ptr %3, align 4\n"
                          "  ret ptr %2\n"
                          "}\n"
                          "\n"
                          "define void @_F4func(ptr %0) {\n"
                          "entry:\n"
                          "  %1 = alloca ptr, align 8\n"
                          "  %2 = alloca ptr, align 8\n"
                          "  store ptr %0, ptr %1, align 8\n"
                          "  %3 = call ptr @malloc(i64 ptrtoint (ptr getelementptr ({ i64, %TestStruct }, ptr null, i32 1) to i64))\n"
                          "  store i64 1, ptr %3, align 8\n"
                          "  %4 = getelementptr i8, ptr %3, i64 8\n"
                          "  %5 = call ptr @TestStruct.init_ctor(ptr %4)\n"
                          "  store ptr %5, ptr %2, align 8\n"
                          "  %shrd_hdr = getelementptr i8, ptr %5, i64 -8\n"
                          "  %shrd_rc = load i64, ptr %shrd_hdr, align 8\n"
                          "  %shrd_rc1 = sub i64 %shrd_rc, 1\n"
                          "  store i64 %shrd_rc1, ptr %shrd_hdr, align 8\n"
                          "  %6 = icmp eq i64 %shrd_rc1, 0\n"
                          "  br i1 %6, label %shrd_free, label %shrd_done\n"
                          "\n"
                          "shrd_free:                                        ; preds = %entry\n"
                          "  call void @free(ptr %shrd_hdr)\n"
                          "  br label %shrd_done\n"
                          "\n"
                          "shrd_done:                                        ; preds = %shrd_free, %entry\n"
                          "  ret void\n"
                          "}\n"
                          "\n"
                          "declare ptr @malloc(i64)\n"
                          "\n"
                          "declare void @free(ptr)\n");
    }

    /**
     * Fly code:
     * struct TestStruct { int a }
     * void func() {
     *   TestStruct t = new shared TestStruct()  // refcount=1
     *   TestStruct u = t                        // retain → refcount=2
     * }  // release(u) → refcount=1; release(t) → refcount=0 → free
     */
    TEST_F(CodeGenTest, CGSharedSmartAllocCopy) {
        ASTModule *Module = CreateModule();

        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT,
                                                       "TestStruct", TopModifiers, SuperClasses);
        ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // TestStruct t = new shared TestStruct()
        ASTType *TestStructType = CreateType(TestStruct);
        ASTLocalVar *TVar = ASTBuilder::CreateLocalVar(SourceLoc, TestStructType, "t", EmptyModifiers);
        ASTDeclStmt *TDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TVar);
        ASTCall *CtorCall = ASTBuilder::CreateCall(SourceLoc, TestStruct->getName(), Args,
                                                   ASTCallKind::CALL_NEW_SHARED);
        ASTIdentifier *TIdent = ASTBuilder::CreateIdentifier(TVar);
        ASTBinary *TAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                      TIdent, CtorCall);
        TDeclStmt->setExpr(TAssign);

        // TestStruct u = t  (shared copy — no compile error, emits retain)
        ASTType *TestStructType2 = CreateType(TestStruct);
        ASTLocalVar *UVar = ASTBuilder::CreateLocalVar(SourceLoc, TestStructType2, "u", EmptyModifiers);
        ASTDeclStmt *UDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, UVar);
        ASTIdentifier *UIdent = ASTBuilder::CreateIdentifier(UVar);
        ASTIdentifier *TIdent2 = ASTBuilder::CreateIdentifier(TVar);
        ASTBinary *UAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                      UIdent, TIdent2);
        UDeclStmt->setExpr(UAssign);

        Generate();
        llvm::Module *M = getModules()[0];
        std::string output = getOutput(M);

        // The IR should:
        //  - malloc {i64, TestStruct}, store rc=1, call init_ctor on data ptr
        //  - emit retain (rc++) when u = t
        //  - emit release (rc--; if 0 free) for u at scope exit
        //  - emit release (rc--; if 0 free) for t at scope exit
        EXPECT_FALSE(output.empty());
        EXPECT_NE(output.find("store i64 1"), std::string::npos) << "shared alloc must store refcount=1";
        EXPECT_NE(output.find("add i64"), std::string::npos)     << "copy must emit retain (add)";
        EXPECT_NE(output.find("sub i64"), std::string::npos)     << "scope exit must emit release (sub)";
        EXPECT_NE(output.find("@free"), std::string::npos)       << "release path must declare @free";
    }

    /**
     * Fly code:
     * struct TestStruct { int a }
     * void func() {
     *   TestStruct outer_t = new unique TestStruct()
     *   {
     *     TestStruct inner_t = new unique TestStruct()
     *   }  // free(inner_t) here
     * }  // free(outer_t) here — inner freed BEFORE outer
     */
    TEST_F(CodeGenTest, CGUniqueNestedScopeCleanup) {
        ASTModule *Module = CreateModule();

        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT,
                                                       "TestStruct", TopModifiers, SuperClasses);
        ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // outer_t = new unique TestStruct()  (in outer scope)
        ASTType *OuterType = CreateType(TestStruct);
        ASTLocalVar *OuterVar = ASTBuilder::CreateLocalVar(SourceLoc, OuterType, "outer_t", EmptyModifiers);
        ASTDeclStmt *OuterDecl = ASTBuilder::CreateDeclStmt(Body, SourceLoc, OuterVar);
        ASTCall *OuterCtor = ASTBuilder::CreateCall(SourceLoc, TestStruct->getName(), Args,
                                                    ASTCallKind::CALL_NEW_UNIQUE);
        ASTBinary *OuterAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                          ASTBuilder::CreateIdentifier(OuterVar), OuterCtor);
        OuterDecl->setExpr(OuterAssign);

        // nested block  { inner_t = new unique TestStruct() }
        ASTBlockStmt *InnerBlock = ASTBuilder::CreateBlockStmt(Body, SourceLoc);
        Body->addContent(InnerBlock);

        ASTType *InnerType = CreateType(TestStruct);
        ASTLocalVar *InnerVar = ASTBuilder::CreateLocalVar(SourceLoc, InnerType, "inner_t", EmptyModifiers);
        ASTDeclStmt *InnerDecl = ASTBuilder::CreateDeclStmt(InnerBlock, SourceLoc, InnerVar);
        ASTCall *InnerCtor = ASTBuilder::CreateCall(SourceLoc, TestStruct->getName(), Args,
                                                    ASTCallKind::CALL_NEW_UNIQUE);
        ASTBinary *InnerAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                          ASTBuilder::CreateIdentifier(InnerVar), InnerCtor);
        InnerDecl->setExpr(InnerAssign);

        Generate();
        std::string output = getOutput(getModules()[0]);

        // inner_t must be freed (inner scope exit) BEFORE outer_t (function scope exit)
        size_t free_inner = output.find("call void @free(ptr %7)");
        size_t free_outer = output.find("call void @free(ptr %5)");
        ASSERT_NE(free_inner, std::string::npos) << "inner_t must be freed";
        ASSERT_NE(free_outer, std::string::npos) << "outer_t must be freed";
        EXPECT_LT(free_inner, free_outer) << "inner_t freed before outer_t";
    }

    /**
     * Fly code:
     * struct TestStruct { int a }
     * void func() {
     *   TestStruct t = new shared TestStruct()  // refcount = 1
     *   {
     *     TestStruct u = t                      // retain → refcount = 2
     *   }  // release(u): refcount 2→1, no free
     * }  // release(t): refcount 1→0 → free
     */
    TEST_F(CodeGenTest, CGSharedNestedScopeCopy) {
        ASTModule *Module = CreateModule();

        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT,
                                                       "TestStruct", TopModifiers, SuperClasses);
        ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // t = new shared TestStruct()  (outer scope)
        ASTType *TType = CreateType(TestStruct);
        ASTLocalVar *TVar = ASTBuilder::CreateLocalVar(SourceLoc, TType, "t", EmptyModifiers);
        ASTDeclStmt *TDecl = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TVar);
        ASTCall *TCtor = ASTBuilder::CreateCall(SourceLoc, TestStruct->getName(), Args,
                                                ASTCallKind::CALL_NEW_SHARED);
        ASTBinary *TAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                      ASTBuilder::CreateIdentifier(TVar), TCtor);
        TDecl->setExpr(TAssign);

        // nested block  { u = t }
        ASTBlockStmt *InnerBlock = ASTBuilder::CreateBlockStmt(Body, SourceLoc);
        Body->addContent(InnerBlock);

        ASTType *UType = CreateType(TestStruct);
        ASTLocalVar *UVar = ASTBuilder::CreateLocalVar(SourceLoc, UType, "u", EmptyModifiers);
        ASTDeclStmt *UDecl = ASTBuilder::CreateDeclStmt(InnerBlock, SourceLoc, UVar);
        ASTBinary *UAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                      ASTBuilder::CreateIdentifier(UVar),
                                                      ASTBuilder::CreateIdentifier(TVar));
        UDecl->setExpr(UAssign);

        Generate();
        std::string output = getOutput(getModules()[0]);

        // Shared alloc must initialize the refcount header to 1
        EXPECT_NE(output.find("store i64 1"), std::string::npos)
            << "shared alloc must store refcount=1";

        // Copy u=t must emit a retain (refcount increment via add)
        size_t add_pos = output.find("add i64");
        EXPECT_NE(add_pos, std::string::npos)
            << "copy must emit retain (refcount increment)";

        // Inner scope exit (u) and outer scope exit (t) must each emit a release (sub)
        size_t first_sub  = output.find("sub i64");
        size_t second_sub = output.find("sub i64", first_sub + 1);
        EXPECT_NE(first_sub,  std::string::npos) << "release for u (inner scope) must be emitted";
        EXPECT_NE(second_sub, std::string::npos) << "release for t (outer scope) must be emitted";

        // retain must be emitted BEFORE the first release
        EXPECT_LT(add_pos, first_sub) << "retain must appear before first release";

        // inner scope (u) must be released BEFORE outer scope (t)
        EXPECT_LT(first_sub, second_sub) << "u (inner scope) released before t (outer scope)";

        // Each release has a conditional free branch
        EXPECT_NE(output.find("shrd_free"), std::string::npos)
            << "conditional free branch must be emitted for release";
        size_t second_free_branch = output.find("shrd_free", output.find("shrd_free") + 1);
        EXPECT_NE(second_free_branch, std::string::npos)
            << "two conditional free branches must be emitted (one per release)";
    }

    /**
     * Fly code:
     * struct TestStruct { int a }
     * void func() {
     *   TestStruct t = new unique TestStruct()
     *   TestStruct u = t  // ERROR: cannot copy unique pointer
     * }
     */
    TEST_F(CodeGenTest, CGUniqueSmartAllocCopyError) {
        ASTModule *Module = CreateModule();

        // struct TestStruct { int a }
        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT,
                                                       "TestStruct", TopModifiers, SuperClasses);
        ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

        // void func() {
        //   TestStruct t = new unique TestStruct()
        //   TestStruct u = t  // ERROR
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // TestStruct t = new unique TestStruct()
        ASTType *TestStructType = CreateType(TestStruct);
        ASTLocalVar *TVar = ASTBuilder::CreateLocalVar(SourceLoc, TestStructType, "t", EmptyModifiers);
        ASTDeclStmt *TDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TVar);
        ASTCall *CtorCall = ASTBuilder::CreateCall(SourceLoc, TestStruct->getName(), Args,
                                                   ASTCallKind::CALL_NEW_UNIQUE);
        ASTIdentifier *TIdent = ASTBuilder::CreateIdentifier(TVar);
        ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                         TIdent, CtorCall);
        TDeclStmt->setExpr(AssignExpr);

        // TestStruct u = t  (copy of unique — must fail)
        ASTType *TestStructType2 = CreateType(TestStruct);
        ASTLocalVar *UVar = ASTBuilder::CreateLocalVar(SourceLoc, TestStructType2, "u", EmptyModifiers);
        ASTDeclStmt *UDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, UVar);
        ASTIdentifier *UIdent = ASTBuilder::CreateIdentifier(UVar);
        ASTIdentifier *TIdent2 = ASTBuilder::CreateIdentifier(TVar);
        ASTBinary *CopyExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                        UIdent, TIdent2);
        UDeclStmt->setExpr(CopyExpr);

        ASSERT_FALSE(Resolve());
    }

    /**
     * Fly code:
     * struct TestStruct { int a }
     * void func() {
     *   TestStruct t = new weak TestStruct()
     * }  // free(t) emitted immediately at scope exit (no refcount)
     */
    TEST_F(CodeGenTest, CGWeakSmartAlloc) {
        ASTModule *Module = CreateModule();

        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT,
                                                       "TestStruct", TopModifiers, SuperClasses);
        ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        ASTType *TestStructType = CreateType(TestStruct);
        ASTLocalVar *TVar = ASTBuilder::CreateLocalVar(SourceLoc, TestStructType, "t", EmptyModifiers);
        ASTDeclStmt *TDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TVar);
        ASTCall *CtorCall = ASTBuilder::CreateCall(SourceLoc, TestStruct->getName(), Args,
                                                   ASTCallKind::CALL_NEW_WEAK);
        ASTIdentifier *TIdent = ASTBuilder::CreateIdentifier(TVar);
        ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                         TIdent, CtorCall);
        TDeclStmt->setExpr(AssignExpr);

        Generate();
        std::string output = getOutput(getModules()[0]);

        // Weak alloc: no refcount header, plain malloc like unique, free at scope exit.
        EXPECT_NE(output.find("@malloc"), std::string::npos) << "weak alloc must call malloc";
        EXPECT_NE(output.find("@free"),   std::string::npos) << "weak alloc must emit free at scope exit";
        EXPECT_EQ(output.find("store i64 1"), std::string::npos) << "weak alloc must NOT store refcount";
        EXPECT_EQ(output.find("add i64"),     std::string::npos) << "weak alloc must NOT emit retain";
        EXPECT_EQ(output.find("sub i64"),     std::string::npos) << "weak alloc must NOT emit release";
    }

    /**
     * Fly code:
     * struct TestStruct { int a }
     * void func() {
     *   TestStruct t = new weak TestStruct()
     *   TestStruct u = t   // weak copy: each copy has its own SA → each calls free()
     * }  // u exits scope first → free(ptr); t exits scope → free(ptr) again (dangling)
     *    // Semantica A: il primo che esce libera, gli altri diventano dangling.
     *    // Il double-free è responsabilità del programmatore.
     */
    TEST_F(CodeGenTest, CGWeakSmartAllocCopy) {
        ASTModule *Module = CreateModule();

        llvm::SmallVector<ASTType *, 4> SuperClasses;
        ASTClass *TestStruct = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::STRUCT,
                                                       "TestStruct", TopModifiers, SuperClasses);
        ASTBuilder::CreateClassAttribute(SourceLoc, TestStruct, IntTypeRef, "a", TopModifiers);

        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // TestStruct t = new weak TestStruct()
        ASTType *TestStructType = CreateType(TestStruct);
        ASTLocalVar *TVar = ASTBuilder::CreateLocalVar(SourceLoc, TestStructType, "t", EmptyModifiers);
        ASTDeclStmt *TDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, TVar);
        ASTCall *CtorCall = ASTBuilder::CreateCall(SourceLoc, TestStruct->getName(), Args,
                                                   ASTCallKind::CALL_NEW_WEAK);
        ASTIdentifier *TIdent = ASTBuilder::CreateIdentifier(TVar);
        ASTBinary *TAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                      TIdent, CtorCall);
        TDeclStmt->setExpr(TAssign);

        // TestStruct u = t  (weak copy — allowed, no retain, no cleanup for u)
        ASTType *TestStructType2 = CreateType(TestStruct);
        ASTLocalVar *UVar = ASTBuilder::CreateLocalVar(SourceLoc, TestStructType2, "u", EmptyModifiers);
        ASTDeclStmt *UDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, UVar);
        ASTIdentifier *UIdent = ASTBuilder::CreateIdentifier(UVar);
        ASTIdentifier *TIdent2 = ASTBuilder::CreateIdentifier(TVar);
        ASTBinary *UAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                      UIdent, TIdent2);
        UDeclStmt->setExpr(UAssign);

        // Resolve must succeed (weak copy is allowed)
        ASSERT_TRUE(Resolve());

        Generate();
        std::string output = getOutput(getModules()[0]);

        // Semantica A: every SA entry (original + copy) calls free() at its own scope exit.
        // Two weak vars in the same scope → two free() calls on the same pointer.
        size_t first_free  = output.find("call void @free");
        size_t second_free = output.find("call void @free", first_free + 1);
        ASSERT_NE(first_free,  std::string::npos) << "t must be freed at scope exit";
        ASSERT_NE(second_free, std::string::npos) << "u must also emit free (each weak SA owns cleanup)";

        // No refcount operations: weak never retains or releases via refcount
        EXPECT_EQ(output.find("add i64"), std::string::npos) << "weak copy must NOT emit retain";
        EXPECT_EQ(output.find("sub i64"), std::string::npos) << "weak copy must NOT emit release";
    }

} // anonymous namespace
