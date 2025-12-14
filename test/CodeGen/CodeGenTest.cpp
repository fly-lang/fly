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
    	ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // default bool a = false
    	ASTLocalVar *LocalVar_a = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "a", EmptyModifiers);
    	ASTIdentifier * Identifier_a = getASTBuilder().CreateIdentifier(LocalVar_a);
    	ASTDeclStmt * DeclStmt_a = getASTBuilder().CreateDeclStmt(Body, SourceLoc, LocalVar_a);
    	ASTBinaryOp *AssignExpr_a = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, Identifier_a, getASTBuilder().CreateDefaultValue());
    	DeclStmt_a->setExpr(AssignExpr_a);

        // default byte b = 0
    	ASTLocalVar *LocalVar_b = getASTBuilder().CreateLocalVar(Body, SourceLoc, ByteTypeRef, "b", EmptyModifiers);
    	ASTIdentifier * Identifier_b = getASTBuilder().CreateIdentifier(LocalVar_b);
    	ASTDeclStmt * DeclStmt_b = getASTBuilder().CreateDeclStmt(Body, SourceLoc, LocalVar_b);
    	ASTBinaryOp *AssignExpr_b = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, Identifier_b, getASTBuilder().CreateDefaultValue());
    	DeclStmt_b->setExpr(AssignExpr_b);

        // default short c = 0
    	ASTLocalVar *LocalVar_c = getASTBuilder().CreateLocalVar(Body, SourceLoc, ShortTypeRef, "c", EmptyModifiers);
    	ASTIdentifier * Identifier_c = getASTBuilder().CreateIdentifier(LocalVar_c);
    	ASTDeclStmt * DeclStmt_c = getASTBuilder().CreateDeclStmt(Body, SourceLoc, LocalVar_c);
    	ASTBinaryOp *AssignExpr_c = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, Identifier_c, getASTBuilder().CreateDefaultValue());
    	DeclStmt_c->setExpr(AssignExpr_c);

        // default ushort d = 0
    	ASTLocalVar *LocalVar_d = getASTBuilder().CreateLocalVar(Body, SourceLoc, UShortTypeRef, "d", EmptyModifiers);
    	ASTIdentifier * Identifier_d = getASTBuilder().CreateIdentifier(LocalVar_d);
    	ASTDeclStmt * DeclStmt_d = getASTBuilder().CreateDeclStmt(Body, SourceLoc, LocalVar_d);
    	ASTBinaryOp *AssignExpr_d = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, Identifier_d, getASTBuilder().CreateDefaultValue());
    	DeclStmt_d->setExpr(AssignExpr_d);

        // default int e = 0
    	ASTLocalVar *LocalVar_e = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntTypeRef, "e", EmptyModifiers);
    	ASTIdentifier * Identifier_e = getASTBuilder().CreateIdentifier(LocalVar_e);
    	ASTDeclStmt * DeclStmt_e = getASTBuilder().CreateDeclStmt(Body, SourceLoc, LocalVar_e);
    	ASTBinaryOp *AssignExpr_e = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, Identifier_e, getASTBuilder().CreateDefaultValue());
    	DeclStmt_e->setExpr(AssignExpr_e);

        // default uint f = 0
    	ASTLocalVar *LocalVar_f = getASTBuilder().CreateLocalVar(Body, SourceLoc, UIntTypeRef, "f", EmptyModifiers);
    	ASTIdentifier * Identifier_f = getASTBuilder().CreateIdentifier(LocalVar_f);
    	ASTDeclStmt * DeclStmt_f = getASTBuilder().CreateDeclStmt(Body, SourceLoc, LocalVar_f);
    	ASTBinaryOp *AssignExpr_f = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, Identifier_f, getASTBuilder().CreateDefaultValue());
    	DeclStmt_f->setExpr(AssignExpr_f);

        // default long g = 0
    	ASTLocalVar *LocalVar_g = getASTBuilder().CreateLocalVar(Body, SourceLoc, LongTypeRef, "g", EmptyModifiers);
    	ASTIdentifier * Identifier_g = getASTBuilder().CreateIdentifier(LocalVar_g);
    	ASTDeclStmt * DeclStmt_g = getASTBuilder().CreateDeclStmt(Body, SourceLoc, LocalVar_g);
    	ASTBinaryOp *AssignExpr_g = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, Identifier_g, getASTBuilder().CreateDefaultValue());
    	DeclStmt_g->setExpr(AssignExpr_g);

        // default ulong h = 0
    	ASTLocalVar *LocalVar_h = getASTBuilder().CreateLocalVar(Body, SourceLoc, ULongTypeRef, "h", EmptyModifiers);
    	ASTIdentifier * Identifier_h = getASTBuilder().CreateIdentifier(LocalVar_h);
    	ASTDeclStmt * DeclStmt_h = getASTBuilder().CreateDeclStmt(Body, SourceLoc, LocalVar_h);
    	ASTBinaryOp *AssignExpr_h = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, Identifier_h, getASTBuilder().CreateDefaultValue());
    	DeclStmt_h->setExpr(AssignExpr_h);

        // default float i = 0.0
    	ASTLocalVar *LocalVar_i = getASTBuilder().CreateLocalVar(Body, SourceLoc, FloatTypeRef, "i", EmptyModifiers);
    	ASTIdentifier * Identifier_i = getASTBuilder().CreateIdentifier(LocalVar_i);
    	ASTDeclStmt * DeclStmt_i = getASTBuilder().CreateDeclStmt(Body, SourceLoc, LocalVar_i);
    	ASTBinaryOp *AssignExpr_i = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, Identifier_i, getASTBuilder().CreateDefaultValue());
    	DeclStmt_i->setExpr(AssignExpr_i);

        // default double j = 0.0
    	ASTLocalVar *LocalVar_j = getASTBuilder().CreateLocalVar(Body, SourceLoc, DoubleTypeRef, "j", EmptyModifiers);
    	ASTIdentifier * Identifier_j = getASTBuilder().CreateIdentifier(LocalVar_j);
    	ASTDeclStmt * DeclStmt_j = getASTBuilder().CreateDeclStmt(Body, SourceLoc, LocalVar_j);
    	ASTBinaryOp *AssignExpr_j = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, Identifier_j, getASTBuilder().CreateDefaultValue());
    	DeclStmt_j->setExpr(AssignExpr_j);

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
						  "  store i8 0, i8* %2, align 1\n"
						  "  store i8 0, i8* %3, align 1\n"
						  "  store i16 0, i16* %4, align 2\n"
						  "  store i16 0, i16* %5, align 2\n"
						  "  store i32 0, i32* %6, align 4\n"
						  "  store i16 0, i16* %5, align 2\n"
						  "  store i64 0, i64* %8, align 8\n"
						  "  store i64 0, i64* %9, align 8\n"
						  "  store double 0.000000e+00, float* %10, align 8\n"
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
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, FloatTypeRef, "b", EmptyModifiers));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, BoolTypeRef, "c", EmptyModifiers));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, LongTypeRef, "d", EmptyModifiers));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, DoubleTypeRef, "e", EmptyModifiers));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, ByteTypeRef, "f", EmptyModifiers));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, ShortTypeRef, "g", EmptyModifiers));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, UShortTypeRef, "h", EmptyModifiers));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, UIntTypeRef, "i", EmptyModifiers));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, ULongTypeRef, "j", EmptyModifiers));
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);

        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);
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
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, FloatTypeRef, "func", TopModifiers, Params, Body);

    	// float g
    	ASTLocalVar *LocalVar_g = getASTBuilder().CreateLocalVar(Body, SourceLoc, FloatTypeRef, "g", EmptyModifiers);
    	ASTDeclStmt *DeclStmt_g = getASTBuilder().CreateDeclStmt(Body, SourceLoc, LocalVar_g);

        // g = 1.0
        ASTIdentifier *VarRef_g = getASTBuilder().CreateIdentifier(LocalVar_g);
         ASTExprStmt * GVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *ExprG = getASTBuilder().CreateNumberValue(SourceLoc, "1.0");
        ASTBinaryOp *AssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, VarRef_g, ExprG);
        GVarStmt->setExpr(AssignExpr);

        // return g
        ASTReturnStmt * Return = getASTBuilder().CreateReturnStmt(Body, SourceLoc);
        Return->setExpr(getASTBuilder().CreateIdentifier(LocalVar_g));

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
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // int a = 1
        ASTLocalVar *LocalVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntTypeRef, "a", EmptyModifiers);
		ASTIdentifier * Identifier = getASTBuilder().CreateIdentifier(LocalVar);
        ASTDeclStmt * DeclStmt = getASTBuilder().CreateDeclStmt(Body, SourceLoc, LocalVar);
        ASTExpr *ValueExpr = getASTBuilder().CreateNumberValue(SourceLoc, "1");
        ASTBinaryOp *AssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, Identifier, ValueExpr);
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
        ASTBlockStmt *BodyTest = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Test = getASTBuilder().CreateFunction(Module, SourceLoc, IntTypeRef, "test", TopModifiers, Params, BodyTest);

        // func()
        ASTBlockStmt *BodyFunc = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, IntTypeRef, "func", TopModifiers, Params, BodyFunc);

        // call test()
        ASTExprStmt * ExprStmt = getASTBuilder().CreateExprStmt(BodyFunc, SourceLoc);
        ASTCall *TestCall = getASTBuilder().CreateCall(SourceLoc, Test->getName(), Args, ASTCallKind::CALL_DIRECT);
        ExprStmt->setExpr(TestCall);

        //return test()
        ASTReturnStmt * Return = getASTBuilder().CreateReturnStmt(BodyFunc, SourceLoc);
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
        ASTParam *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTParam *bParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "b", EmptyModifiers);
        Params.push_back(bParam);
        ASTParam *cParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "c", EmptyModifiers);
        Params.push_back(cParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, IntTypeRef, "func", TopModifiers, Params, Body);

        ASTReturnStmt *Return = getASTBuilder().CreateReturnStmt(Body, SourceLoc);
        // Create this expression: 1 + a * b / (c - 2)
        // E1 + (E2 * E3) / (E4 - E5)
        // E1 + (G2 / G3)
        // E1 + G1
        ASTNumberValue *E1 = getASTBuilder().CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *E2 = getASTBuilder().CreateIdentifier(aParam);
        ASTIdentifier *E3 = getASTBuilder().CreateIdentifier(bParam);
        ASTIdentifier *E4 = getASTBuilder().CreateIdentifier(cParam);
        ASTNumberValue *E5 = getASTBuilder().CreateNumberValue(SourceLoc, "2");

        ASTBinaryOp *G2 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_MUL, E2, E3);
        ASTBinaryOp *G3 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_SUB, E4, E5);
        ASTBinaryOp *G1 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_DIV, G2, G3);
        ASTBinaryOp *Group = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ADD, E1, G1);

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
        ASTParam *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTParam *bParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "b", EmptyModifiers);
        Params.push_back(bParam);
        ASTParam *cParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "c", EmptyModifiers);
        Params.push_back(cParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // a = 0
        ASTExprStmt * aVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *Assign0Expr = getASTBuilder().CreateNumberValue(SourceLoc, "0");
        ASTBinaryOp *aAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aParam), Assign0Expr);
        aVarStmt->setExpr(aAssignExpr);
        // b = 0
        ASTExprStmt *bVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *Assign0Expr2 = getASTBuilder().CreateNumberValue(SourceLoc, "0");
        ASTBinaryOp *bAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(bParam), Assign0Expr2);
        bVarStmt->setExpr(bAssignExpr);

        // c = a + b
         ASTExprStmt * cAddVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *AddExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ADD,
                getASTBuilder().CreateIdentifier(aParam),
                getASTBuilder().CreateIdentifier(bParam));
        ASTBinaryOp *cAddAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cParam), AddExpr);
        cAddVarStmt->setExpr(cAddAssignExpr);

        // c = a - b
         ASTExprStmt * cSubVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *SubExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_SUB,
                getASTBuilder().CreateIdentifier(aParam),
                getASTBuilder().CreateIdentifier(bParam));
        ASTBinaryOp *cSubAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cParam), SubExpr);
        cSubVarStmt->setExpr(cSubAssignExpr);

        // c = a * b
         ASTExprStmt * cMulVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *MulExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_MUL,
                getASTBuilder().CreateIdentifier(aParam),
                getASTBuilder().CreateIdentifier(bParam));
        ASTBinaryOp *cMulAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cParam), MulExpr);
        cMulVarStmt->setExpr(cMulAssignExpr);

        // c = a / b
         ASTExprStmt * cDivVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *DivExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_DIV,
                getASTBuilder().CreateIdentifier(aParam),
                getASTBuilder().CreateIdentifier(bParam));
        ASTBinaryOp *cDivAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cParam), DivExpr);
        cDivVarStmt->setExpr(cDivAssignExpr);

        // c = a % b
         ASTExprStmt * cModVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *ModExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_MOD,
                getASTBuilder().CreateIdentifier(aParam),
                getASTBuilder().CreateIdentifier(bParam));
        ASTBinaryOp *cModAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cParam), ModExpr);
        cModVarStmt->setExpr(cModAssignExpr);

        // c = a & b
         ASTExprStmt * cAndVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *AndExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_AND,
                getASTBuilder().CreateIdentifier(aParam),
                getASTBuilder().CreateIdentifier(bParam));
        ASTBinaryOp *cAndAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cParam), AndExpr);
        cAndVarStmt->setExpr(cAndAssignExpr);

        // c = a | b
         ASTExprStmt * cOrVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *OrExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_OR,
                getASTBuilder().CreateIdentifier(aParam),
                getASTBuilder().CreateIdentifier(bParam));
        ASTBinaryOp *cOrAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cParam), OrExpr);
        cOrVarStmt->setExpr(cOrAssignExpr);

        // c = a xor b
         ASTExprStmt * cXorVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *XorExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_XOR,
                getASTBuilder().CreateIdentifier(aParam),
                getASTBuilder().CreateIdentifier(bParam));
        ASTBinaryOp *cXorAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cParam), XorExpr);
        cXorVarStmt->setExpr(cXorAssignExpr);

        // c = a << b
         ASTExprStmt * cShlVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *ShlExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_SHIFT_L,
                getASTBuilder().CreateIdentifier(aParam),
                getASTBuilder().CreateIdentifier(bParam));
        ASTBinaryOp *cShlAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cParam), ShlExpr);
        cShlVarStmt->setExpr(cShlAssignExpr);

        // c = a >> b
         ASTExprStmt * cShrVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *ShrExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_SHIFT_R,
                getASTBuilder().CreateIdentifier(aParam),
                getASTBuilder().CreateIdentifier(bParam));
        ASTBinaryOp *cShrAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cParam), ShrExpr);
        cShrVarStmt->setExpr(cShrAssignExpr);

        // ++c
        ASTExprStmt * cPreIncVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTUnaryOp *PreIncExpr = getASTBuilder().CreateUnary(SourceLoc, ASTUnaryOpKind::OP_UNARY_PRE_INCR,
                getASTBuilder().CreateIdentifier(cParam));
        cPreIncVarStmt->setExpr(PreIncExpr);

        // c++
        ASTExprStmt * cPostIncVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTUnaryOp *PostIncExpr = getASTBuilder().CreateUnary(SourceLoc, ASTUnaryOpKind::OP_UNARY_POST_INCR,
                getASTBuilder().CreateIdentifier(cParam));
        cPostIncVarStmt->setExpr(PostIncExpr);

        // --c
        ASTExprStmt * cPreDecVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTUnaryOp *PreDecExpr = getASTBuilder().CreateUnary(SourceLoc, ASTUnaryOpKind::OP_UNARY_PRE_DECR,
                getASTBuilder().CreateIdentifier(cParam));
        cPreDecVarStmt->setExpr(PreDecExpr);

        // c--
        ASTExprStmt * cPostDecVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTUnaryOp *PostDecExpr = getASTBuilder().CreateUnary(SourceLoc, ASTUnaryOpKind::OP_UNARY_POST_DECR,
                getASTBuilder().CreateIdentifier(cParam));
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
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        ASTLocalVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntTypeRef, "a", EmptyModifiers);
        ASTDeclStmt *aDeclStmt = getASTBuilder().CreateDeclStmt(Body, SourceLoc, aVar);
        ASTLocalVar *bVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntTypeRef, "b", EmptyModifiers);
        ASTDeclStmt *bDeclStmt = getASTBuilder().CreateDeclStmt(Body, SourceLoc, bVar);
        ASTLocalVar *cVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "c", EmptyModifiers);
        ASTDeclStmt *cDeclStmt = getASTBuilder().CreateDeclStmt(Body, SourceLoc, cVar);

        // a = 0
        ASTExprStmt * aVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr1 = getASTBuilder().CreateNumberValue(SourceLoc, "0");
        ASTBinaryOp *aAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aVar), Expr1);
        aVarStmt->setExpr(aAssignExpr);

        // b = 0
        ASTExprStmt * bVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr2 = getASTBuilder().CreateNumberValue(SourceLoc, "0");
        ASTBinaryOp *bAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(bVar), Expr2);
        bVarStmt->setExpr(bAssignExpr);

        // c = a == b
         ASTExprStmt * cEqVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr3 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                getASTBuilder().CreateIdentifier(aVar),
                getASTBuilder().CreateIdentifier(bVar));
        ASTBinaryOp *cEqAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cVar), Expr3);
        cEqVarStmt->setExpr(cEqAssignExpr);

        // c = a != b
    	ASTExprStmt * cNeqVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr4 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_NE,
                getASTBuilder().CreateIdentifier(aVar),
                getASTBuilder().CreateIdentifier(bVar));
        ASTBinaryOp *cNeqAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cVar), Expr4);
        cNeqVarStmt->setExpr(cNeqAssignExpr);

        // c = a > b
         ASTExprStmt * cGtVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr5 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_GT,
                getASTBuilder().CreateIdentifier(aVar),
                getASTBuilder().CreateIdentifier(bVar));
        ASTBinaryOp *cGtAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cVar), Expr5);
        cGtVarStmt->setExpr(cGtAssignExpr);

        // c = a >= b
         ASTExprStmt * cGteVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr6 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_GTE,
                getASTBuilder().CreateIdentifier(aVar),
                getASTBuilder().CreateIdentifier(bVar));
        ASTBinaryOp *cGteAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cVar), Expr6);
        cGteVarStmt->setExpr(cGteAssignExpr);

        // c = a < b
         ASTExprStmt * cLtVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr7 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_LT,
                getASTBuilder().CreateIdentifier(aVar),
                getASTBuilder().CreateIdentifier(bVar));
        ASTBinaryOp *cLtAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cVar), Expr7);
        cLtVarStmt->setExpr(cLtAssignExpr);

        // c = a <= b
         ASTExprStmt * cLteVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr8 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_LTE,
                getASTBuilder().CreateIdentifier(aVar),
                getASTBuilder().CreateIdentifier(bVar));
        ASTBinaryOp *cLteAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cVar), Expr8);
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
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        ASTLocalVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "a", EmptyModifiers);
        ASTDeclStmt *aDeclStmt = getASTBuilder().CreateDeclStmt(Body, SourceLoc, aVar);
        ASTLocalVar *bVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "b", EmptyModifiers);
        ASTDeclStmt *bDeclStmt = getASTBuilder().CreateDeclStmt(Body, SourceLoc, bVar);
        ASTLocalVar *cVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "c", EmptyModifiers);
        ASTDeclStmt *cDeclStmt = getASTBuilder().CreateDeclStmt(Body, SourceLoc, cVar);

        // a = false
        ASTExprStmt * aVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTBoolValue *Expr1 = getASTBuilder().CreateBoolValue(SourceLoc, false);
        ASTBinaryOp *aAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aVar), Expr1);
        aVarStmt->setExpr(aAssignExpr);

        // b = false
        ASTExprStmt * bVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTBoolValue *Expr2 = getASTBuilder().CreateBoolValue(SourceLoc, false);
        ASTBinaryOp *bAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(bVar), Expr2);
        bVarStmt->setExpr(bAssignExpr);

        // c = a and b
         ASTExprStmt * cAndVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr3 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_LOGIC_AND,
                getASTBuilder().CreateIdentifier(aVar),
                getASTBuilder().CreateIdentifier(bVar));
        ASTBinaryOp *cAndAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cVar), Expr3);
        cAndVarStmt->setExpr(cAndAssignExpr);

        // c = a or b
         ASTExprStmt * cOrVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTExpr *Expr4 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_LOGIC_OR,
                getASTBuilder().CreateIdentifier(aVar),
                getASTBuilder().CreateIdentifier(bVar));
        ASTBinaryOp *cOrAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cVar), Expr4);
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
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        ASTLocalVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "a", EmptyModifiers);
        ASTDeclStmt *aDeclStmt = getASTBuilder().CreateDeclStmt(Body, SourceLoc, aVar);
        ASTLocalVar *bVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "b", EmptyModifiers);
        ASTDeclStmt *bDeclStmt = getASTBuilder().CreateDeclStmt(Body, SourceLoc, bVar);
        ASTLocalVar *cVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "c", EmptyModifiers);
        ASTDeclStmt *cDeclStmt = getASTBuilder().CreateDeclStmt(Body, SourceLoc, cVar);

        // a = false
        ASTExprStmt * aVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTBoolValue *Expr1 = getASTBuilder().CreateBoolValue(SourceLoc, false);
        ASTBinaryOp *aAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aVar), Expr1);
        aVarStmt->setExpr(aAssignExpr);

        // b = false
        ASTExprStmt * bVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTBoolValue *Expr2 = getASTBuilder().CreateBoolValue(SourceLoc, false);
        ASTBinaryOp *bAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(bVar), Expr2);
        bVarStmt->setExpr(bAssignExpr);

        // c = a == b ? a : b
         ASTExprStmt * cVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTBinaryOp *Cond = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                getASTBuilder().CreateIdentifier(aVar),
                getASTBuilder().CreateIdentifier(bVar));

        ASTTernaryOp *TernaryExpr = getASTBuilder().CreateTernary(Cond, SourceLoc,
                                                                    getASTBuilder().CreateIdentifier(aVar),
                                                                    SourceLoc,
                                                                    getASTBuilder().CreateIdentifier(bVar));
        ASTBinaryOp *cAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(cVar), TernaryExpr);
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
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // int a = 0
        ASTLocalVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntTypeRef, "a", EmptyModifiers);
        ASTExprStmt * aVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTNumberValue *Expr1 = getASTBuilder().CreateNumberValue(SourceLoc, "0");
        ASTBinaryOp *aAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aVar), Expr1);
        aVarStmt->setExpr(aAssignExpr);

        // if (a == 1)
        ASTIdentifier *aVarRef = getASTBuilder().CreateIdentifier(aVar);
        ASTNumberValue *Value1 = getASTBuilder().CreateNumberValue(SourceLoc, "1");
        ASTBinaryOp *IfCond = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ, aVarRef, Value1);

        // Create/Add if block
        ASTBuilderIfStmt *IfBuilder = ASTBuilderIfStmt::Create(Body);
        ASTBlockStmt *IfBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);

        // { a = 2 }
        ASTExprStmt * a2VarStmt = getASTBuilder().CreateExprStmt(IfBlock, SourceLoc);
        ASTNumberValue *Expr2 = getASTBuilder().CreateNumberValue(SourceLoc, "2");
        ASTBinaryOp *a2AssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aVar), Expr2);
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
        ASTParam *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // if (a == 1)
        ASTNumberValue *Value1 = getASTBuilder().CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *aVarRef = getASTBuilder().CreateIdentifier(aParam);
        ASTBinaryOp *IfCond = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                aVarRef, Value1);

        // Create/Add if block
        ASTBuilderIfStmt *IfBuilder = ASTBuilderIfStmt::Create(Body);
        ASTBlockStmt *IfBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);

        // { a = 1 }
        ASTExprStmt * aVarStmt = getASTBuilder().CreateExprStmt(IfBlock, SourceLoc);
        ASTNumberValue *Expr1 = getASTBuilder().CreateNumberValue(SourceLoc, "1");
        ASTBinaryOp *aAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aParam), Expr1);
        aVarStmt->setExpr(aAssignExpr);

        // else {a = 2}
        ASTBlockStmt *ElseBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        IfBuilder->Else(SourceLoc, ElseBlock);
        ASTExprStmt *aVarStmt2 = getASTBuilder().CreateExprStmt(ElseBlock, SourceLoc);
        ASTNumberValue *Expr2 = getASTBuilder().CreateNumberValue(SourceLoc, "2");
        ASTBinaryOp *a2AssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aParam), Expr2);
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
        ASTParam *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // if (a == 1)
        ASTBuilderIfStmt *IfBuilder = ASTBuilderIfStmt::Create(Body);
        ASTNumberValue *Value1 = getASTBuilder().CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *aVarRef = getASTBuilder().CreateIdentifier(aParam);
        ASTBinaryOp *IfCond = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                aVarRef, Value1);
        ASTBlockStmt *IfBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);


        // { a = 11 }
        ASTExprStmt * aVarStmt = getASTBuilder().CreateExprStmt(IfBlock, SourceLoc);
        ASTNumberValue *Expr1 = getASTBuilder().CreateNumberValue(SourceLoc, "11");
        ASTBinaryOp *aAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aParam), Expr1);
        aVarStmt->setExpr(aAssignExpr);

        // elsif (a == 2)
        ASTBlockStmt *ElsifBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTNumberValue *Value2 = getASTBuilder().CreateNumberValue(SourceLoc, "2");
        ASTBinaryOp *ElsifCond = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                aVarRef, Value2);
        IfBuilder->ElseIf(SourceLoc, ElsifCond, ElsifBlock);
        // { a = 22 }
        ASTExprStmt *aVarStmt2 = getASTBuilder().CreateExprStmt(ElsifBlock, SourceLoc);
        ASTNumberValue *Expr2 = getASTBuilder().CreateNumberValue(SourceLoc, "22");
        ASTBinaryOp *a2AssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aParam), Expr2);
        aVarStmt2->setExpr(a2AssignExpr);

        // elsif (a == 3) { a = 33 }
        ASTBlockStmt *ElsifBlock2 = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTNumberValue *Value3 = getASTBuilder().CreateNumberValue(SourceLoc, "3");
        ASTBinaryOp *ElsifCond2 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                aVarRef, Value3);
        IfBuilder->ElseIf(SourceLoc, ElsifCond2, ElsifBlock2);
        ASTExprStmt *aVarStmt3 = getASTBuilder().CreateExprStmt(ElsifBlock2, SourceLoc);
        ASTNumberValue *Expr3 = getASTBuilder().CreateNumberValue(SourceLoc, "33");
        ASTBinaryOp *a3AssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aParam), Expr3);
        aVarStmt3->setExpr(a3AssignExpr);

        // else {a == 44}
        ASTBlockStmt *ElseBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        IfBuilder->Else(SourceLoc, ElseBlock);
        ASTExprStmt *aVarStmt4 = getASTBuilder().CreateExprStmt(ElseBlock, SourceLoc);
        ASTNumberValue *Expr4 = getASTBuilder().CreateNumberValue(SourceLoc, "44");
        ASTBinaryOp *a4AssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aParam), Expr4);
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
        ASTParam *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // if a == 1
        ASTBuilderIfStmt *IfBuilder = ASTBuilderIfStmt::Create(Body);
        ASTBlockStmt *IfBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTNumberValue *Value1 = getASTBuilder().CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *aVarRef = getASTBuilder().CreateIdentifier(aParam);
        ASTBinaryOp *IfCond = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ, aVarRef, Value1);

        // { a = 11 }
        IfBuilder->If(SourceLoc, IfCond, IfBlock);
        ASTExprStmt * aVarStmt = getASTBuilder().CreateExprStmt(IfBlock, SourceLoc);
        ASTNumberValue *Expr1 = getASTBuilder().CreateNumberValue(SourceLoc, "11");
        ASTBinaryOp *aAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aParam), Expr1);
        aVarStmt->setExpr(aAssignExpr);

        // elsif (a == 2)
        ASTBlockStmt *ElsifBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTNumberValue *Value2 = getASTBuilder().CreateNumberValue(SourceLoc, "2");
        ASTBinaryOp *ElsifCond = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                aVarRef, Value2);
        IfBuilder->ElseIf(SourceLoc, ElsifCond, ElsifBlock);
        // { a = 22 }
        ASTExprStmt * aVarStmt2 = getASTBuilder().CreateExprStmt(ElsifBlock, SourceLoc);
        ASTNumberValue *Expr2 = getASTBuilder().CreateNumberValue(SourceLoc, "22");
        ASTBinaryOp *a2AssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aParam), Expr2);
        aVarStmt2->setExpr(a2AssignExpr);

        // elsif (a == 3) { a = 33 }
        ASTBlockStmt *ElsifBlock2 = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTNumberValue *Value3 = getASTBuilder().CreateNumberValue(SourceLoc, "3");
        ASTBinaryOp *ElsifCond2 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ,
                aVarRef, Value3);
        IfBuilder->ElseIf(SourceLoc, ElsifCond2, ElsifBlock2);
        ASTExprStmt *aVarStmt3 = getASTBuilder().CreateExprStmt(ElsifBlock2, SourceLoc);
        ASTNumberValue *Expr3 = getASTBuilder().CreateNumberValue(SourceLoc, "33");
        ASTBinaryOp *a3AssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aParam), Expr3);
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
        ASTParam *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // switch a
        ASTBuilderSwitchStmt *SwitchBuilder = ASTBuilderSwitchStmt::Create(Body);
        ASTIdentifier *aVarRefExpr = getASTBuilder().CreateIdentifier(aParam);
        SwitchBuilder->Switch(SourceLoc, aVarRefExpr);

        // case 1: a = 1 break
        ASTBlockStmt *Case1Block = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTNumberValue *Case1Value = getASTBuilder().CreateNumberValue(SourceLoc, "1");
        SwitchBuilder->Case(SourceLoc, Case1Value, Case1Block);
        ASTExprStmt *aVarStmt1 = getASTBuilder().CreateExprStmt(Case1Block, SourceLoc);
        ASTNumberValue *Expr1 = getASTBuilder().CreateNumberValue(SourceLoc, "1");
        ASTBinaryOp *Assign1 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aParam), Expr1);
        aVarStmt1->setExpr(Assign1);

        // case 2: a = 2 break
        ASTBlockStmt *Case2Block = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTNumberValue *Case2Value = getASTBuilder().CreateNumberValue(SourceLoc, "2");
        SwitchBuilder->Case(SourceLoc, Case2Value, Case2Block);
        ASTExprStmt *aVarStmt2 = getASTBuilder().CreateExprStmt(Case2Block, SourceLoc);
        ASTNumberValue *Expr2 = getASTBuilder().CreateNumberValue(SourceLoc, "2");
        ASTBinaryOp *Assign2 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aParam), Expr2);
        aVarStmt2->setExpr(Assign2);

        // default: a = 3
        ASTBlockStmt *DefaultBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        SwitchBuilder->Default(SourceLoc, DefaultBlock);
        ASTExprStmt *aVarStmt3 = getASTBuilder().CreateExprStmt(DefaultBlock, SourceLoc);
        ASTNumberValue *Expr3 = getASTBuilder().CreateNumberValue(SourceLoc, "3");
        ASTBinaryOp *Assign3 = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aParam), Expr3);
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

        ASTParam *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // while a == 1
        ASTBuilderLoopStmt *LoopBuilder = ASTBuilderLoopStmt::CreateLoop(Body, SourceLoc);
        ASTNumberValue *Value1 = getASTBuilder().CreateNumberValue(SourceLoc, "1");
        ASTIdentifier *aVarRef = getASTBuilder().CreateIdentifier(aParam);
        ASTBinaryOp *Cond = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_EQ, aVarRef, Value1);
        ASTBlockStmt *BlockStmt = getASTBuilder().CreateBlockStmt(SourceLoc);
        LoopBuilder->Loop(Cond, BlockStmt);

        // { a = 1 }
        ASTExprStmt * aVarStmt = getASTBuilder().CreateExprStmt(BlockStmt, SourceLoc);
        ASTNumberValue *Expr1 = getASTBuilder().CreateNumberValue(SourceLoc, "1");
        ASTBinaryOp *AssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aParam), Expr1);
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
        ASTParam *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // for int i = 1; i < 1; ++i
        ASTBuilderLoopStmt *LoopBuilder = ASTBuilderLoopStmt::CreateLoop(Body, SourceLoc);

        // Init
        // int i = 1
        ASTBlockStmt *InitBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        LoopBuilder->Init(InitBlock);
        ASTLocalVar *iVar = getASTBuilder().CreateLocalVar(InitBlock, SourceLoc, IntTypeRef, "i", EmptyModifiers);
        ASTIdentifier *iVarRef = getASTBuilder().CreateIdentifier(iVar);
        ASTExprStmt *iVarStmt = getASTBuilder().CreateExprStmt(InitBlock, SourceLoc);
        ASTNumberValue *Value1Expr = getASTBuilder().CreateNumberValue(SourceLoc, "1");
        ASTBinaryOp *iAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, iVarRef, Value1Expr);
        iVarStmt->setExpr(iAssignExpr);

        // Condition
        // i < 1
        ASTBinaryOp *Cond = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_LTE,
                iVarRef, Value1Expr);
        ASTBlockStmt *LoopBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        LoopBuilder->Loop(Cond, LoopBlock);

        // Post
        // ++i
        ASTBlockStmt *PostBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        LoopBuilder->Post(PostBlock);
        ASTUnaryOp *IncExpr = getASTBuilder().CreateUnary(SourceLoc, ASTUnaryOpKind::OP_UNARY_PRE_INCR,
                getASTBuilder().CreateIdentifier(iVar));
        ASTExprStmt *iVarIncStmt = getASTBuilder().CreateExprStmt(PostBlock, SourceLoc);
        iVarIncStmt->setExpr(IncExpr);

        // Loop Block
        // { a = 1 }
        ASTExprStmt * aVarStmt = getASTBuilder().CreateExprStmt(LoopBlock, SourceLoc);
        ASTNumberValue *Expr1 = getASTBuilder().CreateNumberValue(SourceLoc, "1");
        ASTBinaryOp *aAssignExpr = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, getASTBuilder().CreateIdentifier(aParam), Expr1);
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
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "main", TopModifiers, Params, Body);

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
