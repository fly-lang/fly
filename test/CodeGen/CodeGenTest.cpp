//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGenTest.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenClass.h"
#include "Sema/SemaBuilderModifiers.h"
#include "AST/ASTModule.h"
#include "AST/ASTVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTDeclStmt.h"
#include "AST/ASTOp.h"

#include <AST/ASTBuilderIfStmt.h>
#include <AST/ASTBuilderLoopStmt.h>
#include <AST/ASTBuilderSwitchStmt.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTParam.h>
#include <AST/ASTReturnStmt.h>
#include <AST/ASTValue.h>


namespace {

    using namespace fly;

    TEST_F(CodeGenTest, CGDefaultValueLocalVar) {
        /**
         * Fly code:
         * void func() {
         *   bool a
         *   byte b
         *   short c
         *   ushort d
         *   int e
         *   uint f
         *   long g
         *   ulong h
         *   float i
         *   double j
         * }
         */
        ASTModule *Module = CreateModule();
    	ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // default bool a = false
    	ASTLocalVar *LocalVar_a = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "a", EmptyModifiers);
    	ASTIdentifier * Identifier_a = ASTBuilder::CreateIdentifier(LocalVar_a);
    	ASTDeclStmt * DeclStmt_a = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_a);

        // default byte b = 0
    	ASTLocalVar *LocalVar_b = ASTBuilder::CreateLocalVar(SourceLoc, ByteTypeRef, "b", EmptyModifiers);
    	ASTIdentifier * Identifier_b = ASTBuilder::CreateIdentifier(LocalVar_b);
    	ASTDeclStmt * DeclStmt_b = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_b);

        // default short c = 0
    	ASTLocalVar *LocalVar_c = ASTBuilder::CreateLocalVar(SourceLoc, ShortTypeRef, "c", EmptyModifiers);
    	ASTIdentifier * Identifier_c = ASTBuilder::CreateIdentifier(LocalVar_c);
    	ASTDeclStmt * DeclStmt_c = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_c);

        // default ushort d = 0
    	ASTLocalVar *LocalVar_d = ASTBuilder::CreateLocalVar(SourceLoc, UShortTypeRef, "d", EmptyModifiers);
    	ASTIdentifier * Identifier_d = ASTBuilder::CreateIdentifier(LocalVar_d);
    	ASTDeclStmt * DeclStmt_d = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_d);

        // default int e = 0
    	ASTLocalVar *LocalVar_e = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "e", EmptyModifiers);
    	ASTIdentifier * Identifier_e = ASTBuilder::CreateIdentifier(LocalVar_e);
    	ASTDeclStmt * DeclStmt_e = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_e);

        // default uint f = 0
    	ASTLocalVar *LocalVar_f = ASTBuilder::CreateLocalVar(SourceLoc, UIntTypeRef, "f", EmptyModifiers);
    	ASTIdentifier * Identifier_f = ASTBuilder::CreateIdentifier(LocalVar_f);
    	ASTDeclStmt * DeclStmt_f = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_f);

        // default long g = 0
    	ASTLocalVar *LocalVar_g = ASTBuilder::CreateLocalVar(SourceLoc, LongTypeRef, "g", EmptyModifiers);
    	ASTIdentifier * Identifier_g = ASTBuilder::CreateIdentifier(LocalVar_g);
    	ASTDeclStmt * DeclStmt_g = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_g);

        // default ulong h = 0
    	ASTLocalVar *LocalVar_h = ASTBuilder::CreateLocalVar(SourceLoc, ULongTypeRef, "h", EmptyModifiers);
    	ASTIdentifier * Identifier_h = ASTBuilder::CreateIdentifier(LocalVar_h);
    	ASTDeclStmt * DeclStmt_h = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_h);

        // default float i = 0.0
    	ASTLocalVar *LocalVar_i = ASTBuilder::CreateLocalVar(SourceLoc, FloatTypeRef, "i", EmptyModifiers);
    	ASTIdentifier * Identifier_i = ASTBuilder::CreateIdentifier(LocalVar_i);
    	ASTDeclStmt * DeclStmt_i = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_i);

        // default double j = 0.0
    	ASTLocalVar *LocalVar_j = ASTBuilder::CreateLocalVar(SourceLoc, DoubleTypeRef, "j", EmptyModifiers);
    	ASTIdentifier * Identifier_j = ASTBuilder::CreateIdentifier(LocalVar_j);
    	ASTDeclStmt * DeclStmt_j = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_j);

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

    	EXPECT_EQ(output, "define void @_F4func(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  %2 = alloca i8, align 1\n"
						  "  %3 = alloca i8, align 1\n"
						  "  %4 = alloca i16, align 2\n"
						  "  %5 = alloca i16, align 2\n"
						  "  %6 = alloca i32, align 4\n"
						  "  %7 = alloca i32, align 4\n"
						  "  %8 = alloca i64, align 8\n"
						  "  %9 = alloca i64, align 8\n"
						  "  %10 = alloca float, align 4\n"
						  "  %11 = alloca double, align 8\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  // "  store i8 0, i8* %2, align 1\n"
						  // "  store i8 0, i8* %3, align 1\n"
						  // "  store i16 0, i16* %4, align 2\n"
						  // "  store i16 0, i16* %5, align 2\n"
						  // "  store i32 0, i32* %6, align 4\n"
						  // "  store i16 0, i16* %5, align 2\n"
						  // "  store i64 0, i64* %8, align 8\n"
						  // "  store i64 0, i64* %9, align 8\n"
						  // "  store double 0.000000e+00, float* %10, align 8\n"
						  // "  store double 0.000000e+00, double* %11, align 8\n"
						  "  ret void\n"
						  "}\n");
    }

    TEST_F(CodeGenTest, CGFuncParamTypes) {
        /**
         * Fly code:
         * void func(int a, float b, bool c, long d, double e, byte f, short g, ushort h, uint i, ulong j) {
         * }
         */
        ASTModule *Module = CreateModule();

        llvm::SmallVector<ASTParam *, 8> Params;
        Params.push_back(ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers));
        Params.push_back(ASTBuilder::CreateParam(SourceLoc, FloatTypeRef, "b", EmptyModifiers));
        Params.push_back(ASTBuilder::CreateParam(SourceLoc, BoolTypeRef, "c", EmptyModifiers));
        Params.push_back(ASTBuilder::CreateParam(SourceLoc, LongTypeRef, "d", EmptyModifiers));
        Params.push_back(ASTBuilder::CreateParam(SourceLoc, DoubleTypeRef, "e", EmptyModifiers));
        Params.push_back(ASTBuilder::CreateParam(SourceLoc, ByteTypeRef, "f", EmptyModifiers));
        Params.push_back(ASTBuilder::CreateParam(SourceLoc, ShortTypeRef, "g", EmptyModifiers));
        Params.push_back(ASTBuilder::CreateParam(SourceLoc, UShortTypeRef, "h", EmptyModifiers));
        Params.push_back(ASTBuilder::CreateParam(SourceLoc, UIntTypeRef, "i", EmptyModifiers));
        Params.push_back(ASTBuilder::CreateParam(SourceLoc, ULongTypeRef, "j", EmptyModifiers));
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);

        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);
        // func(int a, float b, bool c, long d, double e, byte f, short g, ushort h, uint i, ulong l) {
        // }

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i_f_b_l_d_y_s_us_ui_ul(%error* %0, i32 %1, float %2, i1 %3, i64 %4, double %5, i8 %6, i16 %7, i16 %8, i32 %9, i64 %10) {\n"
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

    TEST_F(CodeGenTest, GCLocalVarAssignAfter) {
        /**
         * Fly code:
         * float func() {
         *   float g
         *   g = 1.0
         *   return g
         * }
         */
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, FloatTypeRef, "func", TopModifiers, Params, Body);

    	// float g
    	ASTLocalVar *LocalVar_g = ASTBuilder::CreateLocalVar(SourceLoc, FloatTypeRef, "g", EmptyModifiers);
    	ASTDeclStmt *DeclStmt_g = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_g);

        // g = 1.0
        ASTIdentifier *VarRef_g = ASTBuilder::CreateIdentifier(LocalVar_g);
         ASTExprStmt * GVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *ExprG = ASTBuilder::CreateNumberValue(SourceLoc, "1.0");
        ASTBinaryOp *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, VarRef_g, ExprG);
        GVarStmt->setExpr(AssignExpr);

        // return g
        ASTReturnStmt * Return = ASTBuilder::CreateReturnStmt(Body, SourceLoc);
        Return->setExpr(ASTBuilder::CreateIdentifier(LocalVar_g));

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define float @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca float, align 4\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  store double 1.000000e+00, float* %2, align 8\n"
                          "  %3 = load float, float* %2, align 4\n"
                          "  ret float %3\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGValue) {
        /**
         * Fly code:
         * void func() {
         *   int a = 1
         * }
         */
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // int a = 1
        ASTLocalVar *LocalVar = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "a", EmptyModifiers);
		ASTIdentifier * Identifier = ASTBuilder::CreateIdentifier(LocalVar);
        ASTDeclStmt * DeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar);
        ASTExpr *ValueExpr = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTBinaryOp *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, Identifier, ValueExpr);
        DeclStmt->setExpr(AssignExpr);

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  store i32 1, i32* %2, align 4\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGFuncCall) {
        /**
         * Fly code:
         * int test() {
         *   return 1
         * }
         * void func() {
         *   int a = test()
         * }
         */
        ASTModule *Module = CreateModule();

        // test()
        ASTBlockStmt *BodyTest = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Test = ASTBuilder::CreateFunction(Module, SourceLoc, IntTypeRef, "test", TopModifiers, Params, BodyTest);

        // func()
        ASTBlockStmt *BodyFunc = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, IntTypeRef, "func", TopModifiers, Params, BodyFunc);

        // call test()
        ASTExprStmt * ExprStmt = ASTBuilder::CreateExprStmt(BodyFunc, SourceLoc);
        ASTCall *TestCall = ASTBuilder::CreateCall(SourceLoc, Test->getName(), Args, ASTCallKind::CALL_DIRECT);
        ExprStmt->setExpr(TestCall);

        //return test()
        ASTReturnStmt * Return = ASTBuilder::CreateReturnStmt(BodyFunc, SourceLoc);
        ASTCall *ReturnExpr = TestCall;
        Return->setExpr(TestCall);

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n"
						  "%error = type { i8, i32, i8* }\n"
						  "\n"
						  "@error = external constant %error\n"
						  "\n"
						  "define i32 @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = load %error*, %error** %1, align 8\n"
                          "  %3 = call i32 @_F4test(%error* %2)\n"
                          "  %4 = call i32 @_F4test(%error* %2)\n"
                          "  ret i32 %4\n"
                          "}\n"
                          "\n"
						  "define i32 @_F4test(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "}\n");
    }

    /**
     * func(int a, int b, int c) {
     *  return 1 + a * b / (c - 2)
     * }
     */
    TEST_F(CodeGenTest, CGGroupExpr) {
        ASTModule *Module = CreateModule();

        // func()
        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *aParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTParam *bParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "b", EmptyModifiers);
        Params.push_back(bParam);
        ASTParam *cParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "c", EmptyModifiers);
        Params.push_back(cParam);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, IntTypeRef, "func", TopModifiers, Params, Body);

        ASTReturnStmt *Return = ASTBuilder::CreateReturnStmt(Body, SourceLoc);
        // Create this expression: 1 + a * b / (c - 2)
        // E1 + (E2 * E3) / (E4 - E5)
        // E1 + (G2 / G3)
        // E1 + G1
        ASTNumberValue *E1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *E2 = ASTBuilder::CreateIdentifier(aParam);
        ASTIdentifier *E3 = ASTBuilder::CreateIdentifier(bParam);
        ASTIdentifier *E4 = ASTBuilder::CreateIdentifier(cParam);
        ASTNumberValue *E5 = ASTBuilder::CreateNumberValue(SourceLoc, "2");

        ASTBinaryOp *G2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_MUL, E2, E3);
        ASTBinaryOp *G3 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_SUB, E4, E5);
        ASTBinaryOp *G1 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_DIV, G2, G3);
        ASTBinaryOp *Group = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ADD, E1, G1);

        Return->setExpr(Group);

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define i32 @_F4func_i_i_i(%error* %0, i32 %1, i32 %2, i32 %3) {\n"
                          "entry:\n"
                          "  %4 = alloca %error*, align 8\n"
                          "  %5 = alloca i32, align 4\n"
                          "  %6 = alloca i32, align 4\n"
                          "  %7 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %4, align 8\n"
                          "  store i32 %1, i32* %5, align 4\n"
                          "  store i32 %2, i32* %6, align 4\n"
                          "  store i32 %3, i32* %7, align 4\n"
                          "  %8 = load i32, i32* %5, align 4\n"
                          "  %9 = load i32, i32* %6, align 4\n"
                          "  %10 = mul i32 %8, %9\n"
                          "  %11 = load i32, i32* %7, align 4\n"
                          "  %12 = sub i32 %11, 2\n"
                          "  %13 = sdiv i32 %10, %12\n"
                          "  %14 = add i32 1, %13\n"
                          "  ret i32 %14\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGArithOp) {
        /**
         * Fly code:
         * void func(int a, int b, int c) {
         *   a = 0
         *   b = 0
         *   c = a + b
         *   c = a - b
         *   c = a * b
         *   c = a / b
         *   c = a % b
         *   c = a & b
         *   c = a | b
         *   c = a ^ b
         *   c = a << b
         *   c = a >> b
         *   ++c
         *   c++
         *   --c
         *   c--
         * }
         */
        ASTModule *Module = CreateModule();

        // func()
        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *aParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTParam *bParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "b", EmptyModifiers);
        Params.push_back(bParam);
        ASTParam *cParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "c", EmptyModifiers);
        Params.push_back(cParam);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // a = 0
        ASTExprStmt * aVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *Assign0Expr = ASTBuilder::CreateNumberValue(SourceLoc, "0");
        ASTBinaryOp *aAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Assign0Expr);
        aVarStmt->setExpr(aAssignExpr);
        // b = 0
        ASTExprStmt *bVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *Assign0Expr2 = ASTBuilder::CreateNumberValue(SourceLoc, "0");
        ASTBinaryOp *bAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(bParam), Assign0Expr2);
        bVarStmt->setExpr(bAssignExpr);

        // c = a + b
         ASTExprStmt * cAddVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *AddExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ADD,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinaryOp *cAddAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), AddExpr);
        cAddVarStmt->setExpr(cAddAssignExpr);

        // c = a - b
         ASTExprStmt * cSubVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *SubExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_SUB,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinaryOp *cSubAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), SubExpr);
        cSubVarStmt->setExpr(cSubAssignExpr);

        // c = a * b
         ASTExprStmt * cMulVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *MulExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_MUL,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinaryOp *cMulAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), MulExpr);
        cMulVarStmt->setExpr(cMulAssignExpr);

        // c = a / b
         ASTExprStmt * cDivVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *DivExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_DIV,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinaryOp *cDivAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), DivExpr);
        cDivVarStmt->setExpr(cDivAssignExpr);

        // c = a % b
         ASTExprStmt * cModVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *ModExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_MOD,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinaryOp *cModAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), ModExpr);
        cModVarStmt->setExpr(cModAssignExpr);

        // c = a & b
         ASTExprStmt * cAndVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *AndExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_AND,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinaryOp *cAndAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), AndExpr);
        cAndVarStmt->setExpr(cAndAssignExpr);

        // c = a | b
         ASTExprStmt * cOrVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *OrExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_OR,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinaryOp *cOrAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), OrExpr);
        cOrVarStmt->setExpr(cOrAssignExpr);

        // c = a xor b
         ASTExprStmt * cXorVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *XorExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_XOR,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinaryOp *cXorAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), XorExpr);
        cXorVarStmt->setExpr(cXorAssignExpr);

        // c = a << b
         ASTExprStmt * cShlVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *ShlExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_SHIFT_L,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinaryOp *cShlAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), ShlExpr);
        cShlVarStmt->setExpr(cShlAssignExpr);

        // c = a >> b
         ASTExprStmt * cShrVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *ShrExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_SHIFT_R,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinaryOp *cShrAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), ShrExpr);
        cShrVarStmt->setExpr(cShrAssignExpr);

        // ++c
        ASTExprStmt * cPreIncVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTUnaryOp *PreIncExpr = ASTBuilder::CreateUnary(SourceLoc, ASTUnaryOpKind::OP_UNARY_PRE_INCR,
                ASTBuilder::CreateIdentifier(cParam));
        cPreIncVarStmt->setExpr(PreIncExpr);

        // c++
        ASTExprStmt * cPostIncVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTUnaryOp *PostIncExpr = ASTBuilder::CreateUnary(SourceLoc, ASTUnaryOpKind::OP_UNARY_POST_INCR,
                ASTBuilder::CreateIdentifier(cParam));
        cPostIncVarStmt->setExpr(PostIncExpr);

        // --c
        ASTExprStmt * cPreDecVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTUnaryOp *PreDecExpr = ASTBuilder::CreateUnary(SourceLoc, ASTUnaryOpKind::OP_UNARY_PRE_DECR,
                ASTBuilder::CreateIdentifier(cParam));
        cPreDecVarStmt->setExpr(PreDecExpr);

        // c--
        ASTExprStmt * cPostDecVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTUnaryOp *PostDecExpr = ASTBuilder::CreateUnary(SourceLoc, ASTUnaryOpKind::OP_UNARY_POST_DECR,
                ASTBuilder::CreateIdentifier(cParam));
        cPostDecVarStmt->setExpr(PostDecExpr);

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i_i_i(%error* %0, i32 %1, i32 %2, i32 %3) {\n"
                          "entry:\n"
                          "  %4 = alloca %error*, align 8\n"
                          "  %5 = alloca i32, align 4\n"
                          "  %6 = alloca i32, align 4\n"
                          "  %7 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %4, align 8\n"
                          "  store i32 %1, i32* %5, align 4\n"
                          "  store i32 %2, i32* %6, align 4\n"
                          "  store i32 %3, i32* %7, align 4\n"
                          "  store i32 0, i32* %5, align 4\n"
                          "  store i32 0, i32* %6, align 4\n"
                          "  %8 = load i32, i32* %5, align 4\n"
                          "  %9 = load i32, i32* %6, align 4\n"
                          "  %10 = add i32 %8, %9\n"
                          "  store i32 %10, i32* %7, align 4\n"
                          "  %11 = sub i32 %8, %9\n"
                          "  store i32 %11, i32* %7, align 4\n"
                          "  %12 = mul i32 %8, %9\n"
                          "  store i32 %12, i32* %7, align 4\n"
                          "  %13 = sdiv i32 %8, %9\n"
                          "  store i32 %13, i32* %7, align 4\n"
                          "  %14 = srem i32 %8, %9\n"
                          "  store i32 %14, i32* %7, align 4\n"
                          "  %15 = and i32 %8, %9\n"
                          "  store i32 %15, i32* %7, align 4\n"
                          "  %16 = or i32 %8, %9\n"
                          "  store i32 %16, i32* %7, align 4\n"
                          "  %17 = xor i32 %8, %9\n"
                          "  store i32 %17, i32* %7, align 4\n"
                          "  %18 = shl i32 %8, %9\n"
                          "  store i32 %18, i32* %7, align 4\n"
                          "  %19 = ashr i32 %8, %9\n"
                          "  store i32 %19, i32* %7, align 4\n"
                          // Unary Operations
                          "  %20 = load i32, i32* %7, align 4\n"
                          "  %21 = add nsw i32 %20, 1\n"
                          "  store i32 %21, i32* %7, align 4\n"
                          "  %22 = load i32, i32* %7, align 4\n"
                          "  %23 = add nsw i32 %22, 1\n"
                          "  store i32 %23, i32* %7, align 4\n"
                          "  %24 = load i32, i32* %7, align 4\n"
                          "  %25 = add nsw i32 %24, -1\n"
                          "  store i32 %25, i32* %7, align 4\n"
                          "  %26 = load i32, i32* %7, align 4\n"
                          "  %27 = add nsw i32 %26, -1\n"
                          "  store i32 %27, i32* %7, align 4\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGComparatorOp) {
        /**
         * Fly code:
         * void func(int a, int b, bool c) {
         *   a = 0
         *   b = 0
         *   c = a == b
         *   c = a != b
         *   c = a > b
         *   c = a >= b
         *   c = a < b
         *   c = a <= b
         * }
         */
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        ASTDeclStmt *aDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, aVar);
        ASTLocalVar *bVar = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "b", EmptyModifiers);
        ASTDeclStmt *bDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, bVar);
        ASTLocalVar *cVar = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "c", EmptyModifiers);
        ASTDeclStmt *cDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, cVar);

        // a = 0
        ASTExprStmt * aVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr1 = ASTBuilder::CreateNumberValue(SourceLoc, "0");
        ASTBinaryOp *aAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aVar), Expr1);
        aVarStmt->setExpr(aAssignExpr);

        // b = 0
        ASTExprStmt * bVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr2 = ASTBuilder::CreateNumberValue(SourceLoc, "0");
        ASTBinaryOp *bAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(bVar), Expr2);
        bVarStmt->setExpr(bAssignExpr);

        // c = a == b
         ASTExprStmt * cEqVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr3 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                ASTBuilder::CreateIdentifier(aVar),
                ASTBuilder::CreateIdentifier(bVar));
        ASTBinaryOp *cEqAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cVar), Expr3);
        cEqVarStmt->setExpr(cEqAssignExpr);

        // c = a != b
    	ASTExprStmt * cNeqVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr4 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_NE,
                ASTBuilder::CreateIdentifier(aVar),
                ASTBuilder::CreateIdentifier(bVar));
        ASTBinaryOp *cNeqAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cVar), Expr4);
        cNeqVarStmt->setExpr(cNeqAssignExpr);

        // c = a > b
         ASTExprStmt * cGtVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr5 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_GT,
                ASTBuilder::CreateIdentifier(aVar),
                ASTBuilder::CreateIdentifier(bVar));
        ASTBinaryOp *cGtAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cVar), Expr5);
        cGtVarStmt->setExpr(cGtAssignExpr);

        // c = a >= b
         ASTExprStmt * cGteVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr6 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_GTE,
                ASTBuilder::CreateIdentifier(aVar),
                ASTBuilder::CreateIdentifier(bVar));
        ASTBinaryOp *cGteAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cVar), Expr6);
        cGteVarStmt->setExpr(cGteAssignExpr);

        // c = a < b
         ASTExprStmt * cLtVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr7 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_LT,
                ASTBuilder::CreateIdentifier(aVar),
                ASTBuilder::CreateIdentifier(bVar));
        ASTBinaryOp *cLtAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cVar), Expr7);
        cLtVarStmt->setExpr(cLtAssignExpr);

        // c = a <= b
         ASTExprStmt * cLteVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr8 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_LTE,
                ASTBuilder::CreateIdentifier(aVar),
                ASTBuilder::CreateIdentifier(bVar));
        ASTBinaryOp *cLteAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cVar), Expr8);
        cLteVarStmt->setExpr(cLteAssignExpr);

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca i32, align 4\n"
                          "  %3 = alloca i32, align 4\n"
                          "  %4 = alloca i8, align 1\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  store i32 0, i32* %2, align 4\n"
                          "  store i32 0, i32* %3, align 4\n"
                          "  %5 = load i32, i32* %2, align 4\n"
                          "  %6 = load i32, i32* %3, align 4\n"
                          "  %7 = icmp eq i32 %5, %6\n"
                          "  %8 = zext i1 %7 to i8\n"
                          "  store i8 %8, i8* %4, align 1\n"
                          "  %9 = icmp ne i32 %5, %6\n"
                          "  %10 = zext i1 %9 to i8\n"
                          "  store i8 %10, i8* %4, align 1\n"
                          "  %11 = icmp sgt i32 %5, %6\n"
                          "  %12 = zext i1 %11 to i8\n"
                          "  store i8 %12, i8* %4, align 1\n"
                          "  %13 = icmp sge i32 %5, %6\n"
                          "  %14 = zext i1 %13 to i8\n"
                          "  store i8 %14, i8* %4, align 1\n"
                          "  %15 = icmp slt i32 %5, %6\n"
                          "  %16 = zext i1 %15 to i8\n"
                          "  store i8 %16, i8* %4, align 1\n"
                          "  %17 = icmp sle i32 %5, %6\n"
                          "  %18 = zext i1 %17 to i8\n"
                          "  store i8 %18, i8* %4, align 1\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGLogicOp) {
        /**
         * Fly code:
         * void func(bool a, bool b, bool c) {
         *   a = false
         *   b = false
         *   c = a && b
         *   c = a || b
         * }
         */
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "a", EmptyModifiers);
        ASTDeclStmt *aDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, aVar);
        ASTLocalVar *bVar = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "b", EmptyModifiers);
        ASTDeclStmt *bDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, bVar);
        ASTLocalVar *cVar = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "c", EmptyModifiers);
        ASTDeclStmt *cDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, cVar);

        // a = false
        ASTExprStmt * aVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTBoolValue *Expr1 = ASTBuilder::CreateBoolValue(SourceLoc, false);
        ASTBinaryOp *aAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aVar), Expr1);
        aVarStmt->setExpr(aAssignExpr);

        // b = false
        ASTExprStmt * bVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTBoolValue *Expr2 = ASTBuilder::CreateBoolValue(SourceLoc, false);
        ASTBinaryOp *bAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(bVar), Expr2);
        bVarStmt->setExpr(bAssignExpr);

        // c = a and b
         ASTExprStmt * cAndVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr3 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_LOGIC_AND,
                ASTBuilder::CreateIdentifier(aVar),
                ASTBuilder::CreateIdentifier(bVar));
        ASTBinaryOp *cAndAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cVar), Expr3);
        cAndVarStmt->setExpr(cAndAssignExpr);

        // c = a or b
         ASTExprStmt * cOrVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr4 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_LOGIC_OR,
                ASTBuilder::CreateIdentifier(aVar),
                ASTBuilder::CreateIdentifier(bVar));
        ASTBinaryOp *cOrAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cVar), Expr4);
        cOrVarStmt->setExpr(cOrAssignExpr);

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca i8, align 1\n"
                          "  %3 = alloca i8, align 1\n"
                          "  %4 = alloca i8, align 1\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  store i8 0, i8* %2, align 1\n"
                          "  store i8 0, i8* %3, align 1\n"
                          "  %5 = load i8, i8* %2, align 1\n"
                          "  %6 = trunc i8 %5 to i1\n"
                          "  br i1 %6, label %and, label %and1\n"
                          "\n"
                          "and:                                              ; preds = %entry\n"
                          "  %7 = load i8, i8* %3, align 1\n"
                          "  %8 = trunc i8 %7 to i1\n"
                          "  br label %and1\n"
                          "\n"
                          "and1:                                             ; preds = %and, %entry\n"
                          "  %9 = phi i1 [ false, %entry ], [ %8, %and ]\n"
                          "  %10 = zext i1 %9 to i8\n"
                          "  store i8 %10, i8* %4, align 1\n"
                          "  %11 = load i8, i8* %2, align 1\n"
                          "  %12 = trunc i8 %11 to i1\n"
                          "  br i1 %12, label %or2, label %or\n"
                          "\n"
                          "or:                                               ; preds = %and1\n"
                          "  %13 = load i8, i8* %3, align 1\n"
                          "  %14 = trunc i8 %13 to i1\n"
                          "  br label %or2\n"
                          "\n"
                          "or2:                                              ; preds = %or, %and1\n"
                          "  %15 = phi i1 [ true, %and1 ], [ %14, %or ]\n"
                          "  %16 = zext i1 %15 to i8\n"
                          "  store i8 %16, i8* %4, align 1\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGTernaryOp) {
        /**
         * Fly code:
         * void func(bool a, bool b, bool c) {
         *   a = false
         *   b = false
         *   c = a == b ? a : b
         * }
         */
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "a", EmptyModifiers);
        ASTDeclStmt *aDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, aVar);
        ASTLocalVar *bVar = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "b", EmptyModifiers);
        ASTDeclStmt *bDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, bVar);
        ASTLocalVar *cVar = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "c", EmptyModifiers);
        ASTDeclStmt *cDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, cVar);

        // a = false
        ASTExprStmt * aVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTBoolValue *Expr1 = ASTBuilder::CreateBoolValue(SourceLoc, false);
        ASTBinaryOp *aAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aVar), Expr1);
        aVarStmt->setExpr(aAssignExpr);

        // b = false
        ASTExprStmt * bVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTBoolValue *Expr2 = ASTBuilder::CreateBoolValue(SourceLoc, false);
        ASTBinaryOp *bAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(bVar), Expr2);
        bVarStmt->setExpr(bAssignExpr);

        // c = a == b ? a : b
         ASTExprStmt * cVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTBinaryOp *Cond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                ASTBuilder::CreateIdentifier(aVar),
                ASTBuilder::CreateIdentifier(bVar));

        ASTTernaryOp *TernaryExpr = ASTBuilder::CreateTernary(Cond, SourceLoc,
                                                                    ASTBuilder::CreateIdentifier(aVar),
                                                                    SourceLoc,
                                                                    ASTBuilder::CreateIdentifier(bVar));
        ASTBinaryOp *cAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cVar), TernaryExpr);
        cVarStmt->setExpr(cAssignExpr);

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca i8, align 1\n"
                          "  %3 = alloca i8, align 1\n"
                          "  %4 = alloca i8, align 1\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  store i8 0, i8* %2, align 1\n"
                          "  store i8 0, i8* %3, align 1\n"
                          "  %5 = load i8, i8* %2, align 1\n"
                          "  %6 = load i8, i8* %3, align 1\n"
                          "  %7 = icmp eq i8 %5, %6\n"
                          "  br i1 %7, label %terntrue, label %ternfalse\n"
                          "\n"
                          "terntrue:                                         ; preds = %entry\n"
                          "  %8 = load i8, i8* %2, align 1\n"
                          "  %9 = trunc i8 %8 to i1\n"
                          "  br label %ternend\n"
                          "\n"
                          "ternfalse:                                        ; preds = %entry\n"
                          "  %10 = load i8, i8* %3, align 1\n"
                          "  %11 = trunc i8 %10 to i1\n"
                          "  br label %ternend\n"
                          "\n"
                          "ternend:                                          ; preds = %ternfalse, %terntrue\n"
                          "  %12 = phi i1 [ %9, %terntrue ], [ %11, %ternfalse ]\n"
                          "  %13 = zext i1 %12 to i8\n"
                          "  store i8 %13, i8* %4, align 1\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGIfBlock) {
        /**
         * Fly code:
         * void func(int a) {
         *   a = 0
         *   if (a == 1) {
         *     a = 2
         *   }
         * }
         */
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // int a = 0
        ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        ASTExprStmt * aVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTNumberValue *Expr1 = ASTBuilder::CreateNumberValue(SourceLoc, "0");
        ASTBinaryOp *aAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aVar), Expr1);
        aVarStmt->setExpr(aAssignExpr);

        // if (a == 1)
        ASTIdentifier *aVarRef = ASTBuilder::CreateIdentifier(aVar);
        ASTNumberValue *Value1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTBinaryOp *IfCond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ, aVarRef, Value1);

        // Create/Add if block
        ASTBuilderIfStmt *IfBuilder = ASTBuilderIfStmt::Create(Body);
        ASTBlockStmt *IfBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);

        // { a = 2 }
        ASTExprStmt * a2VarStmt = ASTBuilder::CreateExprStmt(IfBlock, SourceLoc);
        ASTNumberValue *Expr2 = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        ASTBinaryOp *a2AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aVar), Expr2);
        a2VarStmt->setExpr(a2AssignExpr);

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  store i32 0, i32* %2, align 4\n"
                          "  %3 = load i32, i32* %2, align 4\n"
                          "  %4 = icmp eq i32 %3, 1\n"
                          "  br i1 %4, label %ifthen, label %endif\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 2, i32* %2, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "endif:                                            ; preds = %ifthen, %entry\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGIfElseBlock) {
        /**
         * Fly code:
         * void func(int a) {
         *   if (a == 1) {
         *     a = 2
         *   } else {
         *     a = 3
         *   }
         * }
         */
        ASTModule *Module = CreateModule();

        // func()
        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *aParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // if (a == 1)
        ASTNumberValue *Value1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *aVarRef = ASTBuilder::CreateIdentifier(aParam);
        ASTBinaryOp *IfCond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                aVarRef, Value1);

        // Create/Add if block
        ASTBuilderIfStmt *IfBuilder = ASTBuilderIfStmt::Create(Body);
        ASTBlockStmt *IfBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);

        // { a = 1 }
        ASTExprStmt * aVarStmt = ASTBuilder::CreateExprStmt(IfBlock, SourceLoc);
        ASTNumberValue *Expr1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTBinaryOp *aAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Expr1);
        aVarStmt->setExpr(aAssignExpr);

        // else {a = 2}
        ASTBlockStmt *ElseBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        IfBuilder->Else(SourceLoc, ElseBlock);
        ASTExprStmt *aVarStmt2 = ASTBuilder::CreateExprStmt(ElseBlock, SourceLoc);
        ASTNumberValue *Expr2 = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        ASTBinaryOp *a2AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Expr2);
        aVarStmt2->setExpr(a2AssignExpr);

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i(%error* %0, i32 %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store i32 %1, i32* %3, align 4\n"
                          "  %4 = load i32, i32* %3, align 4\n"
                          "  %5 = icmp eq i32 %4, 1\n"
                          "  br i1 %5, label %ifthen, label %else\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 1, i32* %3, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "else:                                             ; preds = %entry\n"
                          "  store i32 2, i32* %3, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "endif:                                            ; preds = %else, %ifthen\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGIfElsifElseBlock) {
        /**
         * Fly code:
         * void func(int a) {
         *   if (a == 1) {
         *     a = 2
         *   } elsif (a == 2) {
         *     a = 3
         *   } elsif (a == 3) {
         *     a = 4
         *   } else {
         *     a = 5
         *   }
         * }
         */
        ASTModule *Module = CreateModule();

        // func()

        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *aParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // if (a == 1)
        ASTBuilderIfStmt *IfBuilder = ASTBuilderIfStmt::Create(Body);
        ASTNumberValue *Value1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *aVarRef = ASTBuilder::CreateIdentifier(aParam);
        ASTBinaryOp *IfCond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                aVarRef, Value1);
        ASTBlockStmt *IfBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);


        // { a = 11 }
        ASTExprStmt * aVarStmt = ASTBuilder::CreateExprStmt(IfBlock, SourceLoc);
        ASTNumberValue *Expr1 = ASTBuilder::CreateNumberValue(SourceLoc, "11");
        ASTBinaryOp *aAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Expr1);
        aVarStmt->setExpr(aAssignExpr);

        // elsif (a == 2)
        ASTBlockStmt *ElsifBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTNumberValue *Value2 = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        ASTBinaryOp *ElsifCond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                aVarRef, Value2);
        IfBuilder->ElseIf(SourceLoc, ElsifCond, ElsifBlock);
        // { a = 22 }
        ASTExprStmt *aVarStmt2 = ASTBuilder::CreateExprStmt(ElsifBlock, SourceLoc);
        ASTNumberValue *Expr2 = ASTBuilder::CreateNumberValue(SourceLoc, "22");
        ASTBinaryOp *a2AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Expr2);
        aVarStmt2->setExpr(a2AssignExpr);

        // elsif (a == 3) { a = 33 }
        ASTBlockStmt *ElsifBlock2 = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTNumberValue *Value3 = ASTBuilder::CreateNumberValue(SourceLoc, "3");
        ASTBinaryOp *ElsifCond2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                aVarRef, Value3);
        IfBuilder->ElseIf(SourceLoc, ElsifCond2, ElsifBlock2);
        ASTExprStmt *aVarStmt3 = ASTBuilder::CreateExprStmt(ElsifBlock2, SourceLoc);
        ASTNumberValue *Expr3 = ASTBuilder::CreateNumberValue(SourceLoc, "33");
        ASTBinaryOp *a3AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Expr3);
        aVarStmt3->setExpr(a3AssignExpr);

        // else {a == 44}
        ASTBlockStmt *ElseBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        IfBuilder->Else(SourceLoc, ElseBlock);
        ASTExprStmt *aVarStmt4 = ASTBuilder::CreateExprStmt(ElseBlock, SourceLoc);
        ASTNumberValue *Expr4 = ASTBuilder::CreateNumberValue(SourceLoc, "44");
        ASTBinaryOp *a4AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Expr4);
        aVarStmt4->setExpr(a4AssignExpr);

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i(%error* %0, i32 %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store i32 %1, i32* %3, align 4\n"
                          "  %4 = load i32, i32* %3, align 4\n"
                          "  %5 = icmp eq i32 %4, 1\n"
                          "  br i1 %5, label %ifthen, label %elsif\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 11, i32* %3, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif:                                            ; preds = %entry\n"
                          "  %6 = load i32, i32* %3, align 4\n"
                          "  %7 = icmp eq i32 %6, 2\n"
                          "  br i1 %7, label %elsifthen, label %elsif1\n"
                          "\n"
                          "elsifthen:                                        ; preds = %elsif\n"
                          "  store i32 22, i32* %3, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif1:                                           ; preds = %elsif\n"
                          "  %8 = load i32, i32* %3, align 4\n"
                          "  %9 = icmp eq i32 %8, 3\n"
                          "  br i1 %9, label %elsifthen2, label %else\n"
                          "\n"
                          "elsifthen2:                                       ; preds = %elsif1\n"
                          "  store i32 33, i32* %3, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "else:                                             ; preds = %elsif1\n"
                          "  store i32 44, i32* %3, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "endif:                                            ; preds = %else, %elsifthen2, %elsifthen, %ifthen\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGIfElsif) {
        /**
         * Fly code:
         * void func(int a) {
         *   if (a == 1) {
         *     a = 2
         *   } elsif (a == 2) {
         *     a = 3
         *   } elsif (a == 3) {
         *     a = 4
         *   }
         * }
         */
        ASTModule *Module = CreateModule();

        // main()
        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *aParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // if a == 1
        ASTBuilderIfStmt *IfBuilder = ASTBuilderIfStmt::Create(Body);
        ASTBlockStmt *IfBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTNumberValue *Value1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *aVarRef = ASTBuilder::CreateIdentifier(aParam);
        ASTBinaryOp *IfCond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ, aVarRef, Value1);

        // { a = 11 }
        IfBuilder->If(SourceLoc, IfCond, IfBlock);
        ASTExprStmt * aVarStmt = ASTBuilder::CreateExprStmt(IfBlock, SourceLoc);
        ASTNumberValue *Expr1 = ASTBuilder::CreateNumberValue(SourceLoc, "11");
        ASTBinaryOp *aAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Expr1);
        aVarStmt->setExpr(aAssignExpr);

        // elsif (a == 2)
        ASTBlockStmt *ElsifBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTNumberValue *Value2 = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        ASTBinaryOp *ElsifCond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                aVarRef, Value2);
        IfBuilder->ElseIf(SourceLoc, ElsifCond, ElsifBlock);
        // { a = 22 }
        ASTExprStmt * aVarStmt2 = ASTBuilder::CreateExprStmt(ElsifBlock, SourceLoc);
        ASTNumberValue *Expr2 = ASTBuilder::CreateNumberValue(SourceLoc, "22");
        ASTBinaryOp *a2AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Expr2);
        aVarStmt2->setExpr(a2AssignExpr);

        // elsif (a == 3) { a = 33 }
        ASTBlockStmt *ElsifBlock2 = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTNumberValue *Value3 = ASTBuilder::CreateNumberValue(SourceLoc, "3");
        ASTBinaryOp *ElsifCond2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                aVarRef, Value3);
        IfBuilder->ElseIf(SourceLoc, ElsifCond2, ElsifBlock2);
        ASTExprStmt *aVarStmt3 = ASTBuilder::CreateExprStmt(ElsifBlock2, SourceLoc);
        ASTNumberValue *Expr3 = ASTBuilder::CreateNumberValue(SourceLoc, "33");
        ASTBinaryOp *a3AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Expr3);
        aVarStmt3->setExpr(a3AssignExpr);

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i(%error* %0, i32 %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store i32 %1, i32* %3, align 4\n"
                          "  %4 = load i32, i32* %3, align 4\n"
                          "  %5 = icmp eq i32 %4, 1\n"
                          "  br i1 %5, label %ifthen, label %elsif\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 11, i32* %3, align 4\n"
                          "  br label %endif\n"
                          "\nelsif:                                            ; preds = %entry\n"
                          "  %6 = load i32, i32* %3, align 4\n"
                          "  %7 = icmp eq i32 %6, 2\n"
                          "  br i1 %7, label %elsifthen, label %elsif1\n"
                          "\n"
                          "elsifthen:                                        ; preds = %elsif\n"
                          "  store i32 22, i32* %3, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif1:                                           ; preds = %elsif\n"
                          "  %8 = load i32, i32* %3, align 4\n"
                          "  %9 = icmp eq i32 %8, 3\n"
                          "  br i1 %9, label %elsifthen2, label %endif\n"
                          "\n"
                          "elsifthen2:                                       ; preds = %elsif1\n"
                          "  store i32 33, i32* %3, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "endif:                                            ; preds = %elsifthen2, %elsif1, %elsifthen, %ifthen\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGSwitch) {
        /**
         * Fly code:
         * void func(int a) {
         *   switch (a) {
         *     case 1:
         *       a = 2
         *     case 2:
         *       a = 3
         *     default:
         *       a = 4
         *   }
         * }
         */
        ASTModule *Module = CreateModule();

        // main()
        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *aParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // switch a
        ASTBuilderSwitchStmt *SwitchBuilder = ASTBuilderSwitchStmt::Create(Body);
        ASTIdentifier *aVarRefExpr = ASTBuilder::CreateIdentifier(aParam);
        SwitchBuilder->Switch(SourceLoc, aVarRefExpr);

        // case 1: a = 1 break
        ASTBlockStmt *Case1Block = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTNumberValue *Case1Value = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        SwitchBuilder->Case(SourceLoc, Case1Value, Case1Block);
        ASTExprStmt *aVarStmt1 = ASTBuilder::CreateExprStmt(Case1Block, SourceLoc);
        ASTNumberValue *Expr1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTBinaryOp *Assign1 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Expr1);
        aVarStmt1->setExpr(Assign1);

        // case 2: a = 2 break
        ASTBlockStmt *Case2Block = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTNumberValue *Case2Value = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        SwitchBuilder->Case(SourceLoc, Case2Value, Case2Block);
        ASTExprStmt *aVarStmt2 = ASTBuilder::CreateExprStmt(Case2Block, SourceLoc);
        ASTNumberValue *Expr2 = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        ASTBinaryOp *Assign2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Expr2);
        aVarStmt2->setExpr(Assign2);

        // default: a = 3
        ASTBlockStmt *DefaultBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        SwitchBuilder->Default(SourceLoc, DefaultBlock);
        ASTExprStmt *aVarStmt3 = ASTBuilder::CreateExprStmt(DefaultBlock, SourceLoc);
        ASTNumberValue *Expr3 = ASTBuilder::CreateNumberValue(SourceLoc, "3");
        ASTBinaryOp *Assign3 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Expr3);
        aVarStmt3->setExpr(Assign3);

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i(%error* %0, i32 %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store i32 %1, i32* %3, align 4\n"
                          "  %4 = load i32, i32* %3, align 4\n"
                          "  switch i32 %4, label %default [\n"
                          "    i32 1, label %case\n"
                          "    i32 2, label %case1\n"
                          "  ]\n"
                          "\n"
                          "case:                                             ; preds = %entry\n"
                          "  store i32 1, i32* %3, align 4\n"
                          "  br label %case1\n"
                          "\n"
                          "case1:                                            ; preds = %entry, %case\n"
                          "  store i32 2, i32* %3, align 4\n"
                          "  br label %endswitch\n"
                          "\n"
                          "default:                                          ; preds = %entry\n"
                          "  store i32 3, i32* %3, align 4\n"
                          "  br label %endswitch\n"
                          "\n"
                          "endswitch:                                        ; preds = %default, %case1\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGWhile) {
        /**
         * Fly code:
         * void func(int a) {
         *   while (a < 10) {
         *     a = a + 1
         *   }
         * }
         */
        ASTModule *Module = CreateModule();

        // main()
        llvm::SmallVector<ASTParam *, 8> Params;

        ASTParam *aParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // while a == 1
        ASTBuilderLoopStmt *LoopBuilder = ASTBuilderLoopStmt::CreateLoop(Body, SourceLoc);
        ASTNumberValue *Value1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *aVarRef = ASTBuilder::CreateIdentifier(aParam);
        ASTBinaryOp *Cond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ, aVarRef, Value1);
        ASTBlockStmt *BlockStmt = ASTBuilder::CreateBlockStmt(SourceLoc);
        LoopBuilder->Loop(Cond, BlockStmt);

        // { a = 1 }
        ASTExprStmt * aVarStmt = ASTBuilder::CreateExprStmt(BlockStmt, SourceLoc);
        ASTNumberValue *Expr1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTBinaryOp *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Expr1);
        aVarStmt->setExpr(AssignExpr);

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i(%error* %0, i32 %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store i32 %1, i32* %3, align 4\n"
                          "  br label %loopcond\n"
                          "\n"
                          "loopcond:                                         ; preds = %loop, %entry\n"
                          "  %4 = load i32, i32* %3, align 4\n"
                          "  %5 = icmp eq i32 %4, 1\n"
                          "  br i1 %5, label %loop, label %loopend\n"
                          "\n"
                          "loop:                                             ; preds = %loopcond\n"
                          "  store i32 1, i32* %3, align 4\n"
                          "  br label %loopcond\n"
                          "\n"
                          "loopend:                                          ; preds = %loopcond\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGFor) {
        /**
         * Fly code:
         * void func(int a) {
         *   for (int i = 0; i < 10; i++) {
         *     a = a + 1
         *   }
         * }
         */
        ASTModule *Module = CreateModule();

        // main()
        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *aParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // for int i = 1; i < 1; ++i
        ASTBuilderLoopStmt *LoopBuilder = ASTBuilderLoopStmt::CreateLoop(Body, SourceLoc);

        // Init
        // int i = 1
        ASTBlockStmt *InitBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        LoopBuilder->Init(InitBlock);
        ASTLocalVar *iVar = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "i", EmptyModifiers);
        ASTIdentifier *iVarRef = ASTBuilder::CreateIdentifier(iVar);
        ASTExprStmt *iVarStmt = ASTBuilder::CreateExprStmt(InitBlock, SourceLoc);
        ASTNumberValue *Value1Expr = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTBinaryOp *iAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, iVarRef, Value1Expr);
        iVarStmt->setExpr(iAssignExpr);

        // Condition
        // i < 1
        ASTBinaryOp *Cond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_LTE,
                iVarRef, Value1Expr);
        ASTBlockStmt *LoopBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        LoopBuilder->Loop(Cond, LoopBlock);

        // Post
        // ++i
        ASTBlockStmt *PostBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        LoopBuilder->Post(PostBlock);
        ASTUnaryOp *IncExpr = ASTBuilder::CreateUnary(SourceLoc, ASTUnaryOpKind::OP_UNARY_PRE_INCR,
                ASTBuilder::CreateIdentifier(iVar));
        ASTExprStmt *iVarIncStmt = ASTBuilder::CreateExprStmt(PostBlock, SourceLoc);
        iVarIncStmt->setExpr(IncExpr);

        // Loop Block
        // { a = 1 }
        ASTExprStmt * aVarStmt = ASTBuilder::CreateExprStmt(LoopBlock, SourceLoc);
        ASTNumberValue *Expr1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTBinaryOp *aAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Expr1);
        aVarStmt->setExpr(aAssignExpr);

    	// CreateVTable Code
    	llvm::Module * M = Generate()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i(%error* %0, i32 %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca i32, align 4\n"
                          "  %4 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store i32 %1, i32* %3, align 4\n"
                          "  store i32 1, i32* %4, align 4\n"
                          "  br label %loopcond\n"
                          "\n"
                          "loopcond:                                         ; preds = %looppost, %entry\n"
                          "  %5 = load i32, i32* %4, align 4\n"
                          "  %6 = icmp sle i32 %5, 1\n"
                          "  br i1 %6, label %loop, label %loopend\n"
                          "\n"
                          "loop:                                             ; preds = %loopcond\n"
                          "  store i32 1, i32* %3, align 4\n"
                          "  br label %looppost\n"
                          "\n"
                          "looppost:                                         ; preds = %loop\n"
                          "  %7 = load i32, i32* %4, align 4\n"
                          "  %8 = add nsw i32 %7, 1\n"
                          "  store i32 %8, i32* %4, align 4\n"
                          "  br label %loopcond\n"
                          "\n"
                          "loopend:                                          ; preds = %loopcond\n"
                          "  ret void\n"
                          "}\n");
    }

	TEST_F(CodeGenTest, CGMain) {
        /**
         * Fly code:
         * void main() {
         * }
         */
        ASTModule *Module = CreateModule();

        // main() {
        //
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, VoidTypeRef, "main", TopModifiers, Params, Body);

		// CreateVTable Code
		llvm::Module * M = Generate()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "\n%error = type { i8, i32, i8* }\n"
						  "\n"
						  "@error = external constant %error\n"
        				  "\n"
        				  "define i32 @_F4main() {\n"
                          "entry:\n"
                          "  %0 = alloca %error*, align 8\n"
                          "  %1 = load %error*, %error** %0, align 8\n"
                          "  %2 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
                          "  store i8 0, i8* %2, align 1\n"
                          "  %3 = getelementptr inbounds %error, %error* %1, i32 0, i32 1\n"
                          "  store i32 0, i32* %3, align 4\n"
                          "  %4 = getelementptr inbounds %error, %error* %1, i32 0, i32 2\n"
                          "  store i8* null, i8** %4, align 8\n"
                          "  %5 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
                          "  %6 = load i8, i8* %5, align 1\n"
                          "  %7 = icmp ne i8 %6, 0\n"
                          "  %8 = zext i1 %7 to i32\n"
                          "  ret i32 %8\n"
                          "}\n");
    }

} // anonymous namespace
