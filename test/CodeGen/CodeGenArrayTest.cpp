//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

// fly
#include "AST/ASTType.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTOp.h"
#include "AST/ASTValue.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTReturnStmt.h"

#include "CodeGenTest.h"
#include "CodeGen/CodeGenModule.h"
#include "Sema/SemaBuilderModifiers.h"
#include "AST/ASTModule.h"
#include "AST/ASTVar.h"
#include <Sema/SemaNameSpace.h>


namespace {

    using namespace fly;

    TEST_F(CodeGenTest, CGArrayLocalVar) {
        /**
         * Fly code:
         * void func() {
         *   int[] k = {}
         * }
         */
        ASTModule *Module = CreateModule();

        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // default int[] k = {}
        ASTArrayType *ArrayIntType = getASTBuilder().CreateArrayType(SourceLoc, IntTypeRef, nullptr);
        ASTLocalVar *LocalVar_k = getASTBuilder().CreateLocalVar(Body, SourceLoc, ArrayIntType, "k", EmptyModifiers);
        ASTExprStmt *VarStmt_k = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        llvm::SmallVector<ASTValue *, 8> EmptyVals;
        ASTArrayValue *EmptyArr = getASTBuilder().CreateArrayValue(SourceLoc, EmptyVals);
        ASTIdentifier *kIdent = getASTBuilder().CreateIdentifier(LocalVar_k);
        ASTBinaryOp *AssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, kIdent, EmptyArr);
        VarStmt_k->setExpr(AssignExpr);

        // Generate Code
        llvm::Module *M = Generate()[0];
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

    TEST_F(CodeGenTest, CGFuncArrayParam) {
        /**
         * Fly code:
         * void func(int[] k) {
         * }
         */
        ASTModule *Module = CreateModule();

        // Build function with an array parameter: void func(int[] k) {}
        llvm::SmallVector<ASTParam *, 8> LocalParams;
        ASTType *ArrayIntTypeRef = getASTBuilder().CreateArrayType(SourceLoc, IntTypeRef, nullptr);
        LocalParams.push_back(getASTBuilder().CreateParam(SourceLoc, ArrayIntTypeRef, "k", EmptyModifiers));
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);

        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, LocalParams, Body);

        // Generate Code
        llvm::Module *M = Generate()[0];
        std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F0_ia(%error* %0, i32 %1, float %2, i1 %3, i64 %4, double %5, i8 %6, i16 %7, i16 %8, i32 %9, i64 %10) {\n"
                          "entry:\n"
                          "  %11 = alloca %error*, align 8\n"
                          "  %12 = alloca i32, align 4\n"
                          "  %13 = alloca float, align 4\n"
                          "  %14 = alloca i8, align 1\n"
                          "  %15 = alloca i64, align 8\n"
                          "  %16 = alloca double, align 8\n"
                          "  %17 = alloca i8, align 1\n"
                          "  %18 = alloca i16, align 2\n"
                          "  %19 = alloca i16, align 2\n"
                          "  %20 = alloca i32, align 4\n"
                          "  %21 = alloca i64, align 8\n"
                          "  store %error* %0, %error** %11, align 8\n"
                          "  store i32 %1, i32* %12, align 4\n"
                          "  store float %2, float* %13, align 4\n"
                          "  %22 = zext i1 %3 to i8\n"
                          "  store i8 %22, i8* %14, align 1\n"
                          "  store i64 %4, i64* %15, align 8\n"
                          "  store double %5, double* %16, align 8\n"
                          "  store i8 %6, i8* %17, align 1\n"
                          "  store i16 %7, i16* %18, align 2\n"
                          "  store i16 %8, i16* %19, align 2\n"
                          "  store i32 %9, i32* %20, align 4\n"
                          "  store i64 %10, i64* %21, align 8\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, GCArrayLocalVarAssignAfter) {
        /**
         * Fly code:
         * void func() {
         *   int[] g
         *   g = 1.0
         *   return g
         * }
         */
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // int[] g
        ASTType *ArrayIntTypeRef = getASTBuilder().CreateArrayType(SourceLoc, IntTypeRef, nullptr);
        ASTLocalVar *LocalVar_g = getASTBuilder().CreateLocalVar(Body, SourceLoc, ArrayIntTypeRef, "g", EmptyModifiers);

        // g = 1.0
        ASTIdentifier *VarRef_g = getASTBuilder().CreateIdentifier(LocalVar_g);
        ASTExprStmt *GVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTNumberValue *ExprG = getASTBuilder().CreateNumberValue(SourceLoc, "1.0");
        ASTBinaryOp *AssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, VarRef_g, ExprG);
        GVarStmt->setExpr(AssignExpr);

        // return g
        ASTReturnStmt *Return = getASTBuilder().CreateReturnStmt(Body, SourceLoc);
        Return->setExpr(getASTBuilder().CreateIdentifier(LocalVar_g));

        // Generate Code
        llvm::Module *M = Generate()[0];
        std::string output = getOutput(M);

        EXPECT_EQ(output, "define i32 @F_0(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  store i32 1, i32* %2, align 4\n"
                          "  store float 1.000000e+00, float* @G, align 4\n"
                          "  %3 = load i32, i32* %2, align 4\n"
                          "  ret i32 %3\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGArrayValue) {
        /**
         * Fly code:
         * void func() {
         *   int[] a = {1, 2, 3}
         * }
         */
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // int[] a = {1,2,3}
        ASTLocalVar *LocalVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntTypeRef, "a", EmptyModifiers);
        ASTExprStmt *VarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTNumberValue *ValueExpr = getASTBuilder().CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *aIdent = getASTBuilder().CreateIdentifier(LocalVar);
        ASTBinaryOp *AssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, aIdent, ValueExpr);
        VarStmt->setExpr(AssignExpr);

        // Generate Code
        llvm::Module *M = Generate()[0];
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
