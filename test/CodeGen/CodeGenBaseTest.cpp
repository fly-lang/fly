//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTDeclStmt.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTFunction.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTModule.h"
#include "AST/ASTVar.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGenTest.h"
#include "Sema/SemaBuilderModifiers.h"

#include <AST/ASTBinary.h>
#include <AST/ASTBuilderIfStmt.h>
#include <AST/ASTBuilderLoopStmt.h>
#include <AST/ASTBuilderSwitchStmt.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTParam.h>
#include <AST/ASTTernary.h>
#include <AST/ASTUnary.h>
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
    	ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

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
    	Generate();
    	llvm::Module * M = getModules()[0];
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
						  "  store i8 0, i8* %2, align 1\n"
						  "  store i8 0, i8* %3, align 1\n"
						  "  store i16 0, i16* %4, align 2\n"
						  "  store i16 0, i16* %5, align 2\n"
						  "  store i32 0, i32* %6, align 4\n"
						  "  store i32 0, i32* %7, align 4\n"
						  "  store i64 0, i64* %8, align 8\n"
						  "  store i64 0, i64* %9, align 8\n"
						  "  store float 0.000000e+00, float* %10, align 4\n"
						  "  store double 0.000000e+00, double* %11, align 8\n"
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

        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);
        // func(int a, float b, bool c, long d, double e, byte f, short g, ushort h, uint i, ulong l) {
        // }

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i_f_b_l_d_y_s_us_ui_ul(%error* %0, i32* %1, float* %2, i1* %3, i64* %4, double* %5, i8* %6, i16* %7, i16* %8, i32* %9, i64* %10) {\n"
                          "entry:\n"
                          "  %11 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %11, align 8\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGConstParamReadOnly) {
        /**
         * Fly code:
         * void func(const int a, int b) {
         * }
         *
         * Expected: const parameter 'a' should have 'readonly' attribute in LLVM IR
         */
        ASTModule *Module = CreateModule();

        // Create const modifier
        llvm::SmallVector<ASTModifier *, 8> ConstModifiers;
        ConstModifiers.push_back(ASTBuilder::CreateModifier(SourceLoc, ASTModifierKind::MOD_CONSTANT));

        llvm::SmallVector<ASTParam *, 8> Params;
        Params.push_back(ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", ConstModifiers));  // const int a
        Params.push_back(ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "b", EmptyModifiers));  // int b
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);

        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

    	// Generate Code
    	Generate();
    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

        // The const parameter should have 'readonly' attribute
        EXPECT_EQ(output, "define void @_F4func_i_i(%error* %0, i32* readonly %1, i32* %2) {\n"
                          "entry:\n"
                          "  %3 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %3, align 8\n"
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
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

    	// float g
    	ASTLocalVar *LocalVar_g = ASTBuilder::CreateLocalVar(SourceLoc, FloatTypeRef, "g", EmptyModifiers);
    	ASTDeclStmt *DeclStmt_g = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_g);

        // g = 1.0
        ASTIdentifier *VarRef_g = ASTBuilder::CreateIdentifier(LocalVar_g);
         ASTExprStmt * GVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *ExprG = ASTBuilder::CreateNumberValue(SourceLoc, "1.0");
        ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, VarRef_g, ExprG);
        GVarStmt->setExpr(AssignExpr);

        // return g
        ASTReturnStmt * Return = ASTBuilder::CreateReturnStmt(Body, SourceLoc);

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca float, align 4\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  store float 0.000000e+00, float* %2, align 4\n"
                          "  store double 1.000000e+00, float* %2, align 8\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGLocalVarAssign) {
        /**
         * Fly code:
         * void func() {
         *   int a = 1
         * }
         */
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // int a = 1
        ASTLocalVar *LocalVar_a = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "a", EmptyModifiers);
		ASTIdentifier *Identifier_a = ASTBuilder::CreateIdentifier(LocalVar_a);
        ASTDeclStmt *DeclStmt_a = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_a);
        ASTExpr *ValueExpr_a1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTBinary *AssignExpr_a = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, Identifier_a, ValueExpr_a1);
        DeclStmt_a->setExpr(AssignExpr_a);

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
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
         *   return
         * }
         * void func() {
         *   test()
         *   return
         * }
         */
        ASTModule *Module = CreateModule();

        // test()
        ASTBlockStmt *BodyTest = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Test = ASTBuilder::CreateFunction(Module, SourceLoc, "test", TopModifiers, Params, BodyTest);

        // func()
        ASTBlockStmt *BodyFunc = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, BodyFunc);

        //return test()
        ASTReturnStmt * Return = ASTBuilder::CreateReturnStmt(BodyFunc, SourceLoc);
        ASTCall *ReturnExpr = ASTBuilder::CreateCall(SourceLoc, Test->getName(), Args, ASTCallKind::CALL_DIRECT);

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M);

    	EXPECT_EQ(output, "\n"
						  "%error = type { i8, i32, i8* }\n"
						  "\n"
						  "@error = external constant %error\n"
						  "\n"
						  "define void @_F4test(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  ret void\n"
						  "}\n"
						  "\n"
						  "define void @_F4func(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  ret void\n"
						  "}\n");
    }

    /**
     * func(int a, int b, int c) {
     *  return 1 + a * b / (c - 2)
     * }
     */
    TEST_F(CodeGenTest, CGBinaryExpr) {
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
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // Create this expression: 1 + a * b / (c - 2)
        // E1 + (E2 * E3) / (E4 - E5)
        // E1 + (G2 / G3)
        // E1 + G1
        ASTNumberValue *E1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *E2 = ASTBuilder::CreateIdentifier(aParam);
        ASTIdentifier *E3 = ASTBuilder::CreateIdentifier(bParam);
        ASTIdentifier *E4 = ASTBuilder::CreateIdentifier(cParam);
        ASTNumberValue *E5 = ASTBuilder::CreateNumberValue(SourceLoc, "2");

        ASTBinary *G2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_MUL, E2, E3);
        ASTBinary *G3 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_SUB, E4, E5);
        ASTBinary *G1 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_DIV, G2, G3);
        ASTBinary *Group = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_ADD, E1, G1);


    	ASTLocalVar *LocalVar_r = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "r", EmptyModifiers);
    	ASTDeclStmt *DeclStmt_r = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_r);
    	ASTBinary *AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_r), Group);
    	DeclStmt_r->setExpr(AssignExpr);

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i_i_i(%error* %0, i32* %1, i32* %2, i32* %3) {\n"
                          "entry:\n"
                          "  %4 = alloca %error*, align 8\n"
                          "  %5 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %4, align 8\n"
                          "  %6 = load i32, i32* %1, align 4\n"
                          "  %7 = load i32, i32* %2, align 4\n"
                          "  %8 = mul i32 %6, %7\n"
                          "  %9 = load i32, i32* %3, align 4\n"
                          "  %10 = sub i32 %9, 2\n"
                          "  %11 = sdiv i32 %8, %10\n"
                          "  %12 = add i32 1, %11\n"
                          "  store i32 %12, i32* %5, align 4\n"
                          "  ret void\n"
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
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // a = 0
        ASTExprStmt * aVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *Assign0Expr = ASTBuilder::CreateNumberValue(SourceLoc, "0");
        ASTBinary *aAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(aParam), Assign0Expr);
        aVarStmt->setExpr(aAssignExpr);
        // b = 0
        ASTExprStmt *bVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *Assign0Expr2 = ASTBuilder::CreateNumberValue(SourceLoc, "0");
        ASTBinary *bAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(bParam), Assign0Expr2);
        bVarStmt->setExpr(bAssignExpr);

        // c = a + b
         ASTExprStmt * cAddVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *AddExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_ADD,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinary *cAddAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), AddExpr);
        cAddVarStmt->setExpr(cAddAssignExpr);

        // c = a - b
         ASTExprStmt * cSubVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *SubExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_SUB,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinary *cSubAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), SubExpr);
        cSubVarStmt->setExpr(cSubAssignExpr);

        // c = a * b
         ASTExprStmt * cMulVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *MulExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_MUL,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinary *cMulAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), MulExpr);
        cMulVarStmt->setExpr(cMulAssignExpr);

        // c = a / b
         ASTExprStmt * cDivVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *DivExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_DIV,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinary *cDivAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), DivExpr);
        cDivVarStmt->setExpr(cDivAssignExpr);

        // c = a % b
         ASTExprStmt * cModVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *ModExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_MOD,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinary *cModAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), ModExpr);
        cModVarStmt->setExpr(cModAssignExpr);

        // c = a & b
         ASTExprStmt * cAndVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *AndExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_AND,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinary *cAndAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), AndExpr);
        cAndVarStmt->setExpr(cAndAssignExpr);

        // c = a | b
         ASTExprStmt * cOrVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *OrExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_OR,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinary *cOrAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), OrExpr);
        cOrVarStmt->setExpr(cOrAssignExpr);

        // c = a xor b
         ASTExprStmt * cXorVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *XorExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_XOR,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinary *cXorAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), XorExpr);
        cXorVarStmt->setExpr(cXorAssignExpr);

        // c = a << b
         ASTExprStmt * cShlVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *ShlExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_SHIFT_L,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinary *cShlAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), ShlExpr);
        cShlVarStmt->setExpr(cShlAssignExpr);

        // c = a >> b
         ASTExprStmt * cShrVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *ShrExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ARITH_SHIFT_R,
                ASTBuilder::CreateIdentifier(aParam),
                ASTBuilder::CreateIdentifier(bParam));
        ASTBinary *cShrAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(cParam), ShrExpr);
        cShrVarStmt->setExpr(cShrAssignExpr);

        // ++c
        ASTExprStmt * cPreIncVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTUnary *PreIncExpr = ASTBuilder::CreateUnary(SourceLoc, ASTUnaryKind::OP_UNARY_PRE_INCR,
                ASTBuilder::CreateIdentifier(cParam));
        cPreIncVarStmt->setExpr(PreIncExpr);

        // c++
        ASTExprStmt * cPostIncVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTUnary *PostIncExpr = ASTBuilder::CreateUnary(SourceLoc, ASTUnaryKind::OP_UNARY_POST_INCR,
                ASTBuilder::CreateIdentifier(cParam));
        cPostIncVarStmt->setExpr(PostIncExpr);

        // --c
        ASTExprStmt * cPreDecVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTUnary *PreDecExpr = ASTBuilder::CreateUnary(SourceLoc, ASTUnaryKind::OP_UNARY_PRE_DECR,
                ASTBuilder::CreateIdentifier(cParam));
        cPreDecVarStmt->setExpr(PreDecExpr);

        // c--
        ASTExprStmt * cPostDecVarStmt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTUnary *PostDecExpr = ASTBuilder::CreateUnary(SourceLoc, ASTUnaryKind::OP_UNARY_POST_DECR,
                ASTBuilder::CreateIdentifier(cParam));
        cPostDecVarStmt->setExpr(PostDecExpr);

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i_i_i(%error* %0, i32* %1, i32* %2, i32* %3) {\n"
                          "entry:\n"
                          "  %4 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %4, align 8\n"
                          "  store i32 0, i32* %1, align 4\n"
                          "  store i32 0, i32* %2, align 4\n"
                          "  %5 = load i32, i32* %1, align 4\n"
                          "  %6 = load i32, i32* %2, align 4\n"
                          "  %7 = add i32 %5, %6\n"
                          "  store i32 %7, i32* %3, align 4\n"
                          "  %8 = sub i32 %5, %6\n"
                          "  store i32 %8, i32* %3, align 4\n"
                          "  %9 = mul i32 %5, %6\n"
                          "  store i32 %9, i32* %3, align 4\n"
                          "  %10 = sdiv i32 %5, %6\n"
                          "  store i32 %10, i32* %3, align 4\n"
                          "  %11 = srem i32 %5, %6\n"
                          "  store i32 %11, i32* %3, align 4\n"
                          "  %12 = and i32 %5, %6\n"
                          "  store i32 %12, i32* %3, align 4\n"
                          "  %13 = or i32 %5, %6\n"
                          "  store i32 %13, i32* %3, align 4\n"
                          "  %14 = xor i32 %5, %6\n"
                          "  store i32 %14, i32* %3, align 4\n"
                          "  %15 = shl i32 %5, %6\n"
                          "  store i32 %15, i32* %3, align 4\n"
                          "  %16 = ashr i32 %5, %6\n"
                          "  store i32 %16, i32* %3, align 4\n"
                          // Unary Operations
                          "  %17 = load i32, i32* %3, align 4\n"
                          "  %18 = add nsw i32 %17, 1\n"
                          "  store i32 %18, i32* %3, align 4\n"
                          "  %19 = load i32, i32* %3, align 4\n"
                          "  %20 = add nsw i32 %19, 1\n"
                          "  store i32 %20, i32* %3, align 4\n"
                          "  %21 = load i32, i32* %3, align 4\n"
                          "  %22 = add nsw i32 %21, -1\n"
                          "  store i32 %22, i32* %3, align 4\n"
                          "  %23 = load i32, i32* %3, align 4\n"
                          "  %24 = add nsw i32 %23, -1\n"
                          "  store i32 %24, i32* %3, align 4\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGComparatorOp) {
        /**
         * Fly code:
         * void func() {
    	 *   int a
		 *   int b
		 *   int c
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
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        ASTLocalVar *LocalVar_a = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        ASTDeclStmt *DeclStmt_a = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_a);
        ASTLocalVar *LocalVar_b = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "b", EmptyModifiers);
        ASTDeclStmt *DeclStmt_b = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_b);
        ASTLocalVar *LocalVar_c = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "c", EmptyModifiers);
        ASTDeclStmt *DeclStmt_c = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_c);

        // a = 0
        ASTExprStmt *ExprStmt_a = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *ValueExpr_a0 = ASTBuilder::CreateNumberValue(SourceLoc, "0");
        ASTBinary *AssignExpr_a = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_a), ValueExpr_a0);
        ExprStmt_a->setExpr(AssignExpr_a);

        // b = 0
        ASTExprStmt *ExprStmt_b = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *ValueExpr_b0 = ASTBuilder::CreateNumberValue(SourceLoc, "0");
        ASTBinary *AssignExpr_b = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_b), ValueExpr_b0);
        ExprStmt_b->setExpr(AssignExpr_b);

        // c = a == b
        ASTExprStmt *ExprStmt_cEq = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *BinaryExpr_cEq = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ,
                ASTBuilder::CreateIdentifier(LocalVar_a),
                ASTBuilder::CreateIdentifier(LocalVar_b));
        ASTBinary *AssignExpr_cEq = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_c), BinaryExpr_cEq);
        ExprStmt_cEq->setExpr(AssignExpr_cEq);

        // c = a != b
    	ASTExprStmt *ExprStmt_cNeq = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *BinaryExpr_cNeq = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_NE,
                ASTBuilder::CreateIdentifier(LocalVar_a),
                ASTBuilder::CreateIdentifier(LocalVar_b));
        ASTBinary *AssignExpr_cNeq = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_c), BinaryExpr_cNeq);
        ExprStmt_cNeq->setExpr(AssignExpr_cNeq);

        // c = a > b
        ASTExprStmt *ExprStmt_cGt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *BinaryExpr_cGt = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_GT,
                ASTBuilder::CreateIdentifier(LocalVar_a),
                ASTBuilder::CreateIdentifier(LocalVar_b));
        ASTBinary *AssignExpr_cGt = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_c), BinaryExpr_cGt);
        ExprStmt_cGt->setExpr(AssignExpr_cGt);

        // c = a >= b
        ASTExprStmt *ExprStmt_cGte = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *BinaryExpr_cGte = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_GTE,
                ASTBuilder::CreateIdentifier(LocalVar_a),
                ASTBuilder::CreateIdentifier(LocalVar_b));
        ASTBinary *AssignExpr_cGte = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_c), BinaryExpr_cGte);
        ExprStmt_cGte->setExpr(AssignExpr_cGte);

        // c = a < b
        ASTExprStmt *ExprStmt_cLt = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *BinaryExpr_cLt = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_LT,
                ASTBuilder::CreateIdentifier(LocalVar_a),
                ASTBuilder::CreateIdentifier(LocalVar_b));
        ASTBinary *AssignExpr_cLt = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_c), BinaryExpr_cLt);
        ExprStmt_cLt->setExpr(AssignExpr_cLt);

        // c = a <= b
        ASTExprStmt *ExprStmt_cLte = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *BinaryExpr_cLte = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_LTE,
                ASTBuilder::CreateIdentifier(LocalVar_a),
                ASTBuilder::CreateIdentifier(LocalVar_b));
        ASTBinary *AssignExpr_cLte = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_c), BinaryExpr_cLte);
        ExprStmt_cLte->setExpr(AssignExpr_cLte);

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca i32, align 4\n"
                          "  %3 = alloca i32, align 4\n"
                          "  %4 = alloca i8, align 1\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  store i32 0, i32* %2, align 4\n" // will be optimized by LLVM in later passes
                          "  store i32 0, i32* %3, align 4\n"  // will be optimized by LLVM in later passes
                          "  store i8 0, i8* %4, align 1\n"
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
         * void func() {
         *   bool a
         *   bool b
         *   bool c
         *   // a = false
         *   // b = false
         *   c = a && b
         *   c = a || b
         * }
         */
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        ASTLocalVar *LocalVar_a = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "a", EmptyModifiers);
        ASTDeclStmt *DeclStmt_a = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_a);
        ASTLocalVar *LocalVar_b = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "b", EmptyModifiers);
        ASTDeclStmt *DeclStmt_b = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_b);
        ASTLocalVar *LocalVar_c = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "c", EmptyModifiers);
        ASTDeclStmt *DeclStmt_c = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_c);

        // // a = false
        // ASTExprStmt *ExprStmt_a = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        // ASTBoolValue *BoolValue_a = ASTBuilder::CreateBoolValue(SourceLoc, false);
        // ASTBinary *AssignExpr_a = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_a), BoolValue_a);
        // ExprStmt_a->setExpr(AssignExpr_a);
        //
        // // b = false
        // ASTExprStmt *ExprStmt_b = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        // ASTBoolValue *BoolValue_b = ASTBuilder::CreateBoolValue(SourceLoc, false);
        // ASTBinary *AssignExpr_b = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_b), BoolValue_b);
        // ExprStmt_b->setExpr(AssignExpr_b);

        // c = a and b
        ASTExprStmt *ExprStmt_cAnd = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *BinaryExpr_cAnd = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_LOGIC_AND,
                ASTBuilder::CreateIdentifier(LocalVar_a),
                ASTBuilder::CreateIdentifier(LocalVar_b));
        ASTBinary *AssignExpr_cAnd = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_c), BinaryExpr_cAnd);
        ExprStmt_cAnd->setExpr(AssignExpr_cAnd);

        // c = a or b
        ASTExprStmt *ExprStmt_cOr = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTExpr *BinaryExpr_cOr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_LOGIC_OR,
                ASTBuilder::CreateIdentifier(LocalVar_a),
                ASTBuilder::CreateIdentifier(LocalVar_b));
        ASTBinary *AssignExpr_cOr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_c), BinaryExpr_cOr);
        ExprStmt_cOr->setExpr(AssignExpr_cOr);

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
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
                          "  store i8 0, i8* %4, align 1\n"
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
         * void func() {
         *   bool a
         *   bool b
         *   bool c
         *   c = a == b ? a : b
         * }
         */
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        ASTLocalVar *LocalVar_a = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "a", EmptyModifiers);
        ASTDeclStmt *DeclStmt_a = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_a);
        ASTLocalVar *LocalVar_b = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "b", EmptyModifiers);
        ASTDeclStmt *DeclStmt_b = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_b);
        ASTLocalVar *LocalVar_c = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "c", EmptyModifiers);
        ASTDeclStmt *DeclStmt_c = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_c);

        // c = a == b ? a : b
        ASTExprStmt *ExprStmt_c = ASTBuilder::CreateExprStmt(Body, SourceLoc);
        ASTBinary *CondExpr_c = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ,
                ASTBuilder::CreateIdentifier(LocalVar_a),
                ASTBuilder::CreateIdentifier(LocalVar_b));

        ASTTernary *TernaryExpr_c = ASTBuilder::CreateTernary(CondExpr_c, SourceLoc,
                                                                    ASTBuilder::CreateIdentifier(LocalVar_a),
                                                                    SourceLoc,
                                                                    ASTBuilder::CreateIdentifier(LocalVar_b));
        ASTBinary *AssignExpr_c = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_c), TernaryExpr_c);
        ExprStmt_c->setExpr(AssignExpr_c);

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
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
                          "  store i8 0, i8* %4, align 1\n"
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
         * void func() {
         *   int a = 0
         *   if (a == 1) {
         *     a = 2
         *   }
         * }
         */
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // int a = 0
        ASTLocalVar *LocalVar_a = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        ASTDeclStmt *DeclStmt_a = ASTBuilder::CreateDeclStmt(Body, SourceLoc, LocalVar_a);
        ASTNumberValue *NumberValue_a0 = ASTBuilder::CreateNumberValue(SourceLoc, "0");
        ASTBinary *AssignExpr_a = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_a), NumberValue_a0);
        DeclStmt_a->setExpr(AssignExpr_a);

        // if (a == 1)
        ASTIdentifier *Identifier_a = ASTBuilder::CreateIdentifier(LocalVar_a);
        ASTNumberValue *NumberValue_1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTBinary *IfCond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ, Identifier_a, NumberValue_1);

        // Create/Add if block
        ASTBuilderIfStmt *IfBuilder = ASTBuilderIfStmt::Create(Body);
        ASTBlockStmt *IfBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);

        // { a = 2 }
        ASTExprStmt *ExprStmt_a2 = ASTBuilder::CreateExprStmt(IfBlock, SourceLoc);
        ASTNumberValue *NumberValue_a2 = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        ASTBinary *AssignExpr_a2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(LocalVar_a), NumberValue_a2);
        ExprStmt_a2->setExpr(AssignExpr_a2);

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
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
        ASTParam *Param_a = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(Param_a);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // if (a == 1)
        ASTNumberValue *NumberValue_1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *Identifier_a = ASTBuilder::CreateIdentifier(Param_a);
        ASTBinary *IfCond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ,
                Identifier_a, NumberValue_1);

        // Create/Add if block
        ASTBuilderIfStmt *IfBuilder = ASTBuilderIfStmt::Create(Body);
        ASTBlockStmt *IfBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);

        // { a = 1 }
        ASTExprStmt *ExprStmt_a1 = ASTBuilder::CreateExprStmt(IfBlock, SourceLoc);
        ASTNumberValue *NumberValue_a1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTBinary *AssignExpr_a1 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(Param_a), NumberValue_a1);
        ExprStmt_a1->setExpr(AssignExpr_a1);

        // else {a = 2}
        ASTBlockStmt *ElseBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        IfBuilder->Else(SourceLoc, ElseBlock);
        ASTExprStmt *ExprStmt_a2 = ASTBuilder::CreateExprStmt(ElseBlock, SourceLoc);
        ASTNumberValue *NumberValue_a2 = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        ASTBinary *AssignExpr_a2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(Param_a), NumberValue_a2);
        ExprStmt_a2->setExpr(AssignExpr_a2);

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i(%error* %0, i32* %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  %3 = load i32, i32* %1, align 4\n"
                          "  %4 = icmp eq i32 %3, 1\n"
                          "  br i1 %4, label %ifthen, label %else\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "else:                                             ; preds = %entry\n"
                          "  store i32 2, i32* %1, align 4\n"
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
         *     a = 11
         *   } elsif (a == 2) {
         *     a = 22
         *   } elsif (a == 3) {
         *     a = 33
         *   } else {
         *     a = 44
         *   }
         * }
         */
        ASTModule *Module = CreateModule();

        // func()

        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *Param_a = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(Param_a);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // if (a == 1)
        ASTBuilderIfStmt *IfBuilder = ASTBuilderIfStmt::Create(Body);
        ASTNumberValue *NumberValue_1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *Identifier_a1 = ASTBuilder::CreateIdentifier(Param_a);
        ASTBinary *IfCond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ,
                Identifier_a1, NumberValue_1);
        ASTBlockStmt *IfBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);


        // { a = 11 }
        ASTExprStmt * aVarStmt = ASTBuilder::CreateExprStmt(IfBlock, SourceLoc);
        ASTNumberValue *Expr1 = ASTBuilder::CreateNumberValue(SourceLoc, "11");
        ASTBinary *aAssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(Param_a), Expr1);
        aVarStmt->setExpr(aAssignExpr);

        // elsif (a == 2)
        ASTBlockStmt *ElsifBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTNumberValue *NumberValue_2 = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        ASTIdentifier *Identifier_a2 = ASTBuilder::CreateIdentifier(Param_a);
        ASTBinary *ElsifCond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ,
                Identifier_a2, NumberValue_2);
        IfBuilder->ElseIf(SourceLoc, ElsifCond, ElsifBlock);
        // { a = 22 }
        ASTExprStmt *ExprStmt_a22 = ASTBuilder::CreateExprStmt(ElsifBlock, SourceLoc);
        ASTNumberValue *NumberValue_a22 = ASTBuilder::CreateNumberValue(SourceLoc, "22");
        ASTBinary *AssignExpr_a22 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(Param_a), NumberValue_a22);
        ExprStmt_a22->setExpr(AssignExpr_a22);

        // elsif (a == 3)
        ASTBlockStmt *ElsifBlock2 = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTNumberValue *NumberValue_3 = ASTBuilder::CreateNumberValue(SourceLoc, "3");
        ASTIdentifier *Identifier_a3 = ASTBuilder::CreateIdentifier(Param_a);
        ASTBinary *ElsifCond2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ,
                Identifier_a3, NumberValue_3);
        IfBuilder->ElseIf(SourceLoc, ElsifCond2, ElsifBlock2);
    	// { a = 33 }
        ASTExprStmt *ExprStmt_a33 = ASTBuilder::CreateExprStmt(ElsifBlock2, SourceLoc);
        ASTNumberValue *NumberValue_a33 = ASTBuilder::CreateNumberValue(SourceLoc, "33");
        ASTBinary *AssignExpr_a33 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(Param_a), NumberValue_a33);
        ExprStmt_a33->setExpr(AssignExpr_a33);

        // else {a == 44}
        ASTBlockStmt *ElseBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        IfBuilder->Else(SourceLoc, ElseBlock);
        ASTExprStmt *aVarStmt4 = ASTBuilder::CreateExprStmt(ElseBlock, SourceLoc);
        ASTNumberValue *Expr4 = ASTBuilder::CreateNumberValue(SourceLoc, "44");
        ASTBinary *a4AssignExpr = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(Param_a), Expr4);
        aVarStmt4->setExpr(a4AssignExpr);

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i(%error* %0, i32* %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  %3 = load i32, i32* %1, align 4\n"
                          "  %4 = icmp eq i32 %3, 1\n"
                          "  br i1 %4, label %ifthen, label %elsif\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 11, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif:                                            ; preds = %entry\n"
                          "  %5 = load i32, i32* %1, align 4\n"
                          "  %6 = icmp eq i32 %5, 2\n"
                          "  br i1 %6, label %elsifthen, label %elsif1\n"
                          "\n"
                          "elsifthen:                                        ; preds = %elsif\n"
                          "  store i32 22, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif1:                                           ; preds = %elsif\n"
                          "  %7 = load i32, i32* %1, align 4\n"
                          "  %8 = icmp eq i32 %7, 3\n"
                          "  br i1 %8, label %elsifthen2, label %else\n"
                          "\n"
                          "elsifthen2:                                       ; preds = %elsif1\n"
                          "  store i32 33, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "else:                                             ; preds = %elsif1\n"
                          "  store i32 44, i32* %1, align 4\n"
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
        ASTParam *Param_a = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(Param_a);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // if a == 1
        ASTBuilderIfStmt *IfBuilder = ASTBuilderIfStmt::Create(Body);
        ASTBlockStmt *IfBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTNumberValue *NumberValue_1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *Identifier_a = ASTBuilder::CreateIdentifier(Param_a);
        ASTBinary *IfCond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ, Identifier_a, NumberValue_1);

        // { a = 11 }
        IfBuilder->If(SourceLoc, IfCond, IfBlock);
        ASTExprStmt *ExprStmt_a11 = ASTBuilder::CreateExprStmt(IfBlock, SourceLoc);
        ASTNumberValue *NumberValue_a11 = ASTBuilder::CreateNumberValue(SourceLoc, "11");
        ASTBinary *AssignExpr_a11 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(Param_a), NumberValue_a11);
        ExprStmt_a11->setExpr(AssignExpr_a11);

        // elsif (a == 2)
        ASTBlockStmt *ElsifBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTNumberValue *NumberValue_2 = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        ASTIdentifier *Identifier_a2 = ASTBuilder::CreateIdentifier(Param_a);
        ASTBinary *ElsifCond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ,
                Identifier_a2, NumberValue_2);
        IfBuilder->ElseIf(SourceLoc, ElsifCond, ElsifBlock);
        // { a = 22 }
        ASTExprStmt *ExprStmt_a22 = ASTBuilder::CreateExprStmt(ElsifBlock, SourceLoc);
        ASTNumberValue *NumberValue_a22 = ASTBuilder::CreateNumberValue(SourceLoc, "22");
        ASTBinary *AssignExpr_a22 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(Param_a), NumberValue_a22);
        ExprStmt_a22->setExpr(AssignExpr_a22);

        // elsif (a == 3) { a = 33 }
        ASTBlockStmt *ElsifBlock2 = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTNumberValue *NumberValue_3 = ASTBuilder::CreateNumberValue(SourceLoc, "3");
        ASTIdentifier *Identifier_a3 = ASTBuilder::CreateIdentifier(Param_a);
        ASTBinary *ElsifCond2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ,
                Identifier_a3, NumberValue_3);
        IfBuilder->ElseIf(SourceLoc, ElsifCond2, ElsifBlock2);
        ASTExprStmt *ExprStmt_a33 = ASTBuilder::CreateExprStmt(ElsifBlock2, SourceLoc);
        ASTNumberValue *NumberValue_a33 = ASTBuilder::CreateNumberValue(SourceLoc, "33");
        ASTBinary *AssignExpr_a33 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(Param_a), NumberValue_a33);
        ExprStmt_a33->setExpr(AssignExpr_a33);

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i(%error* %0, i32* %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  %3 = load i32, i32* %1, align 4\n"
                          "  %4 = icmp eq i32 %3, 1\n"
                          "  br i1 %4, label %ifthen, label %elsif\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 11, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif:                                            ; preds = %entry\n"
                          "  %5 = load i32, i32* %1, align 4\n"
                          "  %6 = icmp eq i32 %5, 2\n"
                          "  br i1 %6, label %elsifthen, label %elsif1\n"
                          "\n"
                          "elsifthen:                                        ; preds = %elsif\n"
                          "  store i32 22, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif1:                                           ; preds = %elsif\n"
                          "  %7 = load i32, i32* %1, align 4\n"
                          "  %8 = icmp eq i32 %7, 3\n"
                          "  br i1 %8, label %elsifthen2, label %endif\n"
                          "\n"
                          "elsifthen2:                                       ; preds = %elsif1\n"
                          "  store i32 33, i32* %1, align 4\n"
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
         *       a = 1
         *     case 2:
         *       a = 2
         *     default:
         *       a = 3
         *   }
         * }
         */
        ASTModule *Module = CreateModule();

        // main()
        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *Param_a = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(Param_a);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // switch a
    	ASTIdentifier *Identifier_aExpr = ASTBuilder::CreateIdentifier(Param_a);
        ASTBuilderSwitchStmt *SwitchBuilder = ASTBuilderSwitchStmt::Create(Body, SourceLoc, Identifier_aExpr);

        // case 1: a = 1 break
        ASTBlockStmt *Case1Block = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTNumberValue *NumberValue_Case1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        SwitchBuilder->addCase(SourceLoc, NumberValue_Case1, Case1Block);
        ASTExprStmt *ExprStmt_a1 = ASTBuilder::CreateExprStmt(Case1Block, SourceLoc);
        ASTNumberValue *NumberValue_a1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTBinary *AssignExpr_a1 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(Param_a), NumberValue_a1);
        ExprStmt_a1->setExpr(AssignExpr_a1);

        // case 2: a = 2 break
        ASTBlockStmt *Case2Block = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTNumberValue *Case2Value = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        SwitchBuilder->addCase(SourceLoc, Case2Value, Case2Block);
        ASTExprStmt *aVarStmt2 = ASTBuilder::CreateExprStmt(Case2Block, SourceLoc);
        ASTNumberValue *Expr2 = ASTBuilder::CreateNumberValue(SourceLoc, "2");
        ASTBinary *Assign2 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(Param_a), Expr2);
        aVarStmt2->setExpr(Assign2);

        // default: a = 3
        ASTBlockStmt *DefaultBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        SwitchBuilder->setDefault(SourceLoc, DefaultBlock);
        ASTExprStmt *aVarStmt3 = ASTBuilder::CreateExprStmt(DefaultBlock, SourceLoc);
        ASTNumberValue *Expr3 = ASTBuilder::CreateNumberValue(SourceLoc, "3");
        ASTBinary *Assign3 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(Param_a), Expr3);
        aVarStmt3->setExpr(Assign3);

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i(%error* %0, i32* %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  %3 = load i32, i32* %1, align 4\n"
                          "  switch i32 %3, label %default [\n"
                          "    i32 1, label %case\n"
                          "    i32 2, label %case1\n"
                          "  ]\n"
                          "\n"
                          "case:                                             ; preds = %entry\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  br label %case1\n"
                          "\n"
                          "case1:                                            ; preds = %entry, %case\n"
                          "  store i32 2, i32* %1, align 4\n"
                          "  br label %endswitch\n"
                          "\n"
                          "default:                                          ; preds = %entry\n"
                          "  store i32 3, i32* %1, align 4\n"
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

        ASTParam *Param_a = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(Param_a);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // while a == 1
        ASTBuilderLoopStmt *LoopBuilder = ASTBuilderLoopStmt::Create(Body, SourceLoc);
        ASTNumberValue *NumberValue_1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *Identifier_a = ASTBuilder::CreateIdentifier(Param_a);
        ASTBinary *Cond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ, Identifier_a, NumberValue_1);
        ASTBlockStmt *BlockStmt = ASTBuilder::CreateBlockStmt(SourceLoc);
        LoopBuilder->setCycle(Cond, BlockStmt);

        // { a = 1 }
        ASTExprStmt *ExprStmt_a1 = ASTBuilder::CreateExprStmt(BlockStmt, SourceLoc);
        ASTNumberValue *NumberValue_a1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTBinary *AssignExpr_a1 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(Param_a), NumberValue_a1);
        ExprStmt_a1->setExpr(AssignExpr_a1);

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i(%error* %0, i32* %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  br label %loopcond\n"
                          "\n"
                          "loopcond:                                         ; preds = %loop, %entry\n"
                          "  %3 = load i32, i32* %1, align 4\n"
                          "  %4 = icmp eq i32 %3, 1\n"
                          "  br i1 %4, label %loop, label %loopend\n"
                          "\n"
                          "loop:                                             ; preds = %loopcond\n"
                          "  store i32 1, i32* %1, align 4\n"
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
        ASTParam *Param_a = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(Param_a);
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // for int i = 1; i < 1; ++i
        ASTBuilderLoopStmt *LoopBuilder = ASTBuilderLoopStmt::Create(Body, SourceLoc);

        // Init
        // int i = 1
        ASTBlockStmt *InitBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTLocalVar *LocalVar_i = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "i", EmptyModifiers);
        ASTDeclStmt *DeclStmt_i = ASTBuilder::CreateDeclStmt(InitBlock, SourceLoc, LocalVar_i);
    	LoopBuilder->setInit(InitBlock);

        // Create initialization: i = 1
        ASTIdentifier *Identifier_iInit = ASTBuilder::CreateIdentifier(SourceLoc, "i", nullptr);
        ASTNumberValue *NumberValue_i1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTBinary *AssignExpr_i = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, Identifier_iInit, NumberValue_i1);
        DeclStmt_i->setExpr(AssignExpr_i);

        // Condition
        // i < 1
        ASTIdentifier *Identifier_iCond = ASTBuilder::CreateIdentifier(SourceLoc, "i", nullptr);
        ASTNumberValue *NumberValue_1Cond = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTBinary *Cond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_LTE,
                Identifier_iCond, NumberValue_1Cond);

    	// Create Loop
        ASTBlockStmt *LoopBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        LoopBuilder->setCycle(Cond, LoopBlock);

        // Post
        // ++i
        ASTBlockStmt *PostBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTIdentifier *Identifier_iPost = ASTBuilder::CreateIdentifier(SourceLoc, "i", nullptr);
        ASTUnary *UnaryExpr_iIncr = ASTBuilder::CreateUnary(SourceLoc, ASTUnaryKind::OP_UNARY_PRE_INCR, Identifier_iPost);
        ASTExprStmt *ExprStmt_iIncr = ASTBuilder::CreateExprStmt(PostBlock, SourceLoc);
        ExprStmt_iIncr->setExpr(UnaryExpr_iIncr);
    	LoopBuilder->setPost(PostBlock);

        // Loop Block
        // { a = 1 }
        ASTExprStmt *ExprStmt_a1 = ASTBuilder::CreateExprStmt(LoopBlock, SourceLoc);
        ASTNumberValue *NumberValue_a1 = ASTBuilder::CreateNumberValue(SourceLoc, "1");
        ASTBinary *AssignExpr_a1 = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, ASTBuilder::CreateIdentifier(Param_a), NumberValue_a1);
        ExprStmt_a1->setExpr(AssignExpr_a1);

    	// CreateVTable Code
    	Generate();
    	llvm::Module * M = getModules()[0];
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func_i(%error* %0, i32* %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store i32 1, i32* %3, align 4\n"
                          "  br label %loopcond\n"
                          "\n"
                          "loopcond:                                         ; preds = %looppost, %entry\n"
                          "  %4 = load i32, i32* %3, align 4\n"
                          "  %5 = icmp sle i32 %4, 1\n"
                          "  br i1 %5, label %loop, label %loopend\n"
                          "\n"
                          "loop:                                             ; preds = %loopcond\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  br label %looppost\n"
                          "\n"
                          "looppost:                                         ; preds = %loop\n"
                          "  %6 = load i32, i32* %3, align 4\n"
                          "  %7 = add nsw i32 %6, 1\n"
                          "  store i32 %7, i32* %3, align 4\n"
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
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "main", TopModifiers, Params, Body);

		// CreateVTable Code
		Generate();
    	llvm::Module * M = getModules()[0];
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
