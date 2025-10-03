//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

// fly
#include "CodeGenTest.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGenClass.h"
#include "Sema/SemaBuilderModifiers.h"
#include "Sema/SemaBuilderStmt.h"
#include "Sema/SemaBuilderIfStmt.h"
#include "Sema/SemaBuilderSwitchStmt.h"
#include "Sema/SemaBuilderLoopStmt.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTRef.h"
#include "AST/ASTVar.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTOpExpr.h"


namespace {

    using namespace fly;

    TEST_F(CodeGenTest, CGDefaultValueLocalVar) {
        ASTModule *Module = CreateModule();

    	ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // default bool a = false
    	ASTVar *LocalVar_a = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "a", EmptyModifiers);
    	SemaBuilderStmt *VarStmt_a = getASTBuilder().CreateAssignmentStmt(Body, LocalVar_a);
    	VarStmt_a->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(BoolTypeRef->getSema())));

        // default byte b = 0
    	ASTVar *LocalVar_b = getASTBuilder().CreateLocalVar(Body, SourceLoc, ByteTypeRef, "b", EmptyModifiers);
    	SemaBuilderStmt *VarStmt_b = getASTBuilder().CreateAssignmentStmt(Body, LocalVar_b);
    	VarStmt_b->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(ByteTypeRef->getSema())));

        // default short c = 0
    	ASTVar *LocalVar_c = getASTBuilder().CreateLocalVar(Body, SourceLoc, ShortTypeRef, "c", EmptyModifiers);
    	SemaBuilderStmt *VarStmt_c = getASTBuilder().CreateAssignmentStmt(Body, LocalVar_c);
    	VarStmt_c->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(ShortTypeRef->getSema())));

        // default ushort d = 0
    	ASTVar *LocalVar_d = getASTBuilder().CreateLocalVar(Body, SourceLoc, UShortTypeRef, "d", EmptyModifiers);
    	SemaBuilderStmt *VarStmt_d = getASTBuilder().CreateAssignmentStmt(Body, LocalVar_d);
    	VarStmt_d->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(UShortTypeRef->getSema())));

        // default int e = 0
    	ASTVar *LocalVar_e = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntTypeRef, "e", EmptyModifiers);
    	SemaBuilderStmt *VarStmt_e = getASTBuilder().CreateAssignmentStmt(Body, LocalVar_e);
    	VarStmt_e->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(IntTypeRef->getSema())));

        // default uint f = 0
    	ASTVar *LocalVar_f = getASTBuilder().CreateLocalVar(Body, SourceLoc, UIntTypeRef, "f", EmptyModifiers);
    	SemaBuilderStmt *VarStmt_f = getASTBuilder().CreateAssignmentStmt(Body, LocalVar_d);
    	VarStmt_f->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(UIntTypeRef->getSema())));

        // default long g = 0
    	ASTVar *LocalVar_g = getASTBuilder().CreateLocalVar(Body, SourceLoc, LongTypeRef, "g", EmptyModifiers);
    	SemaBuilderStmt *VarStmt_g = getASTBuilder().CreateAssignmentStmt(Body, LocalVar_g);
    	VarStmt_g->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(LongTypeRef->getSema())));

        // default ulong h = 0
    	ASTVar *LocalVar_h = getASTBuilder().CreateLocalVar(Body, SourceLoc, ULongTypeRef, "h", EmptyModifiers);
    	SemaBuilderStmt *VarStmt_h = getASTBuilder().CreateAssignmentStmt(Body, LocalVar_h);
    	VarStmt_h->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(ULongTypeRef->getSema())));

        // default float i = 0.0
    	ASTVar *LocalVar_i = getASTBuilder().CreateLocalVar(Body, SourceLoc, FloatTypeRef, "i", EmptyModifiers);
    	SemaBuilderStmt *VarStmt_i = getASTBuilder().CreateAssignmentStmt(Body, LocalVar_i);
    	VarStmt_i->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(FloatTypeRef->getSema())));

        // default double j = 0.0
    	ASTVar *LocalVar_j = getASTBuilder().CreateLocalVar(Body, SourceLoc, DoubleTypeRef, "j", EmptyModifiers);
    	SemaBuilderStmt *VarStmt_j = getASTBuilder().CreateAssignmentStmt(Body, LocalVar_j);
    	VarStmt_j->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(DoubleTypeRef->getSema())));

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        llvm::SmallVector<ASTVar *, 8> Params;
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

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, FloatTypeRef, "func", TopModifiers, Params, Body);

    	// float g
    	ASTVar *LocalVar_g = getASTBuilder().CreateLocalVar(Body, SourceLoc, FloatTypeRef, "g", EmptyModifiers);

        // g = 1.0
        ASTRef *VarRef_g = getASTBuilder().CreateVarRef(LocalVar_g);
        SemaBuilderStmt * GVarStmt = getASTBuilder().CreateAssignmentStmt(Body, VarRef_g);
        ASTExpr *ExprG = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "1.0"));
        GVarStmt->setExpr(ExprG);

        // return g
        SemaBuilderStmt *Return = getASTBuilder().CreateReturnStmt(Body, SourceLoc);
        Return->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(LocalVar_g)));

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // int a = 1
        ASTVar *LocalVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntTypeRef, "a", EmptyModifiers);
        SemaBuilderStmt *VarStmt = getASTBuilder().CreateAssignmentStmt(Body, LocalVar);
        ASTValueExpr *ValueExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "1"));
        VarStmt->setExpr(ValueExpr);

        // validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // test()
        ASTBlockStmt *BodyTest = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Test = getASTBuilder().CreateFunction(Module, SourceLoc, IntTypeRef, "test", TopModifiers, Params, BodyTest);

        // func()
        ASTBlockStmt *BodyFunc = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, IntTypeRef, "func", TopModifiers, Params, BodyFunc);

        // call test()
        SemaBuilderStmt *ExprStmt = getASTBuilder().CreateExprStmt(BodyFunc, SourceLoc);
        ASTCall *TestCall = CreateCall(Test, Args, ASTCallKind::CALL_DIRECT);
        ASTCallExpr *Expr = getASTBuilder().CreateExpr(TestCall);
        ExprStmt->setExpr(Expr);

        //return test()
        SemaBuilderStmt *Return = getASTBuilder().CreateReturnStmt(BodyFunc, SourceLoc);
        ASTCallExpr *ReturnExpr = getASTBuilder().CreateExpr(TestCall);
        Return->setExpr(ReturnExpr);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        llvm::SmallVector<ASTVar *, 8> Params;
        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTVar *bParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "b", EmptyModifiers);
        Params.push_back(bParam);
        ASTVar *cParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "c", EmptyModifiers);
        Params.push_back(cParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, IntTypeRef, "func", TopModifiers, Params, Body);

        SemaBuilderStmt *Return = getASTBuilder().CreateReturnStmt(Body, SourceLoc);
        // Create this expression: 1 + a * b / (c - 2)
        // E1 + (E2 * E3) / (E4 - E5)
        // E1 + (G2 / G3)
        // E1 + G1
        ASTValueExpr *E1 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "1"));
        ASTVarRefExpr *E2 = getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam));
        ASTVarRefExpr *E3 = getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bParam));
        ASTVarRefExpr *E4 = getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(cParam));
        ASTValueExpr *E5 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "2"));

        ASTBinaryOpExpr *G2 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_MUL, E2, E3);
        ASTBinaryOpExpr *G3 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_SUB, E4, E5);
        ASTBinaryOpExpr *G1 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_DIV, G2, G3);
        ASTBinaryOpExpr *Group = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_ADD, E1, G1);

        Return->setExpr(Group);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // func()
        llvm::SmallVector<ASTVar *, 8> Params;
        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTVar *bParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "b", EmptyModifiers);
        Params.push_back(bParam);
        ASTVar *cParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "c", EmptyModifiers);
        Params.push_back(cParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // a = 0
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(aParam));
        ASTExpr *AssignExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "0"));
        aVarStmt->setExpr(AssignExpr);

        // b = 0
        SemaBuilderStmt *bVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(bParam));
        AssignExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "0"));
        bVarStmt->setExpr(AssignExpr);

        // c = a + b
        SemaBuilderStmt * cAddVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_ADD,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bParam)));
        cAddVarStmt->setExpr(AssignExpr);

        // c = a - b
        SemaBuilderStmt * cSubVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_SUB,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bParam)));
        cSubVarStmt->setExpr(AssignExpr);

        // c = a * b
        SemaBuilderStmt * cMulVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_MUL,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bParam)));
        cMulVarStmt->setExpr(AssignExpr);

        // c = a / b
        SemaBuilderStmt * cDivVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_DIV,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bParam)));
        cDivVarStmt->setExpr(AssignExpr);

        // c = a % b
        SemaBuilderStmt * cModVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_MOD,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bParam)));
        cModVarStmt->setExpr(AssignExpr);

        // c = a & b
        SemaBuilderStmt * cAndVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_AND,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bParam)));
        cAndVarStmt->setExpr(AssignExpr);

        // c = a | b
        SemaBuilderStmt * cOrVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_OR,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bParam)));
        cOrVarStmt->setExpr(AssignExpr);

        // c = a xor b
        SemaBuilderStmt * cXorVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_XOR,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bParam)));
        cXorVarStmt->setExpr(AssignExpr);

        // c = a << b
        SemaBuilderStmt * cShlVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_SHIFT_L,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bParam)));
        cShlVarStmt->setExpr(AssignExpr);

        // c = a >> b
        SemaBuilderStmt * cShrVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_SHIFT_R,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bParam)));
        cShrVarStmt->setExpr(AssignExpr);

        // ++c
        SemaBuilderStmt *cPreIncVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        AssignExpr = getASTBuilder().CreateUnaryOpExpr(SourceLoc, ASTUnaryOpExprKind::OP_UNARY_PRE_INCR,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(cParam)));
        cPreIncVarStmt->setExpr(AssignExpr);

        // c++
        SemaBuilderStmt *cPostIncVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        AssignExpr = getASTBuilder().CreateUnaryOpExpr(SourceLoc, ASTUnaryOpExprKind::OP_UNARY_POST_INCR,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(cParam)));
        cPostIncVarStmt->setExpr(AssignExpr);

        // --c
        SemaBuilderStmt *cPreDecVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        AssignExpr = getASTBuilder().CreateUnaryOpExpr(SourceLoc, ASTUnaryOpExprKind::OP_UNARY_PRE_DECR,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(cParam)));
        cPreDecVarStmt->setExpr(AssignExpr);

        // c--
        SemaBuilderStmt *cPostDecVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        AssignExpr = getASTBuilder().CreateUnaryOpExpr(SourceLoc, ASTUnaryOpExprKind::OP_UNARY_POST_DECR,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(cParam)));
        cPostDecVarStmt->setExpr(AssignExpr);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntTypeRef, "a", EmptyModifiers);
        ASTVar *bVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntTypeRef, "b", EmptyModifiers);
        ASTVar *cVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "c", EmptyModifiers);

        // a = 0
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(aVar));
        ASTExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "0"));
        aVarStmt->setExpr(Expr1);

        // b = 0
        SemaBuilderStmt *bVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(bVar));
        ASTExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "0"));
        bVarStmt->setExpr(Expr2);

        // c = a == b
        SemaBuilderStmt * cEqVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cVar));
        ASTExpr *Expr3 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bVar)));
        cEqVarStmt->setExpr(Expr3);

        // c = a != b
        SemaBuilderStmt * cNeqVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cVar));
        ASTExpr *Expr4 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_NE,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bVar)));
        cNeqVarStmt->setExpr(Expr4);

        // c = a > b
        SemaBuilderStmt * cGtVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cVar));
        ASTExpr *Expr5 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_GT,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bVar)));
        cGtVarStmt->setExpr(Expr5);

        // c = a >= b
        SemaBuilderStmt * cGteVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cVar));
        ASTExpr *Expr6 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_GTE,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bVar)));
        cGteVarStmt->setExpr(Expr6);

        // c = a < b
        SemaBuilderStmt * cLtVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cVar));
        ASTExpr *Expr7 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_LT,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bVar)));
        cLtVarStmt->setExpr(Expr7);

        // c = a <= b
        SemaBuilderStmt * cLteVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cVar));
        ASTExpr *Expr8 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_LTE,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bVar)));
        cLteVarStmt->setExpr(Expr8);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "a", EmptyModifiers);
        ASTVar *bVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "b", EmptyModifiers);
        ASTVar *cVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "c", EmptyModifiers);

        // a = false
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(aVar));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateBoolValue(SourceLoc, false));
        aVarStmt->setExpr(Expr1);

        // b = false
        SemaBuilderStmt *bVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(bVar));
        ASTValueExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateBoolValue(SourceLoc, false));
        bVarStmt->setExpr(Expr2);

        // c = a and b
        SemaBuilderStmt * cAndVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cVar));
        ASTExpr *Expr3 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_LOGIC_AND,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bVar)));
        cAndVarStmt->setExpr(Expr3);

        // c = a or b
        SemaBuilderStmt * cOrVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cVar));
        ASTExpr *Expr4 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_LOGIC_OR,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bVar)));
        cOrVarStmt->setExpr(Expr4);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "a", EmptyModifiers);
        ASTVar *bVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "b", EmptyModifiers);
        ASTVar *cVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolTypeRef, "c", EmptyModifiers);

        // a = false
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(aVar));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateBoolValue(SourceLoc, false));
        aVarStmt->setExpr(Expr1);

        // b = false
        SemaBuilderStmt *bVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(bVar));
        ASTValueExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateBoolValue(SourceLoc, false));
        bVarStmt->setExpr(Expr2);

        // c = a == b ? a : b
        SemaBuilderStmt * cVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(cVar));
        ASTBinaryOpExpr *Cond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bVar)));

        ASTTernaryOpExpr *TernaryExpr = getASTBuilder().CreateTernaryOpExpr(Cond, SourceLoc,
                                                                    getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aVar)),
                                                                    SourceLoc,
                                                                    getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(bVar)));
        cVarStmt->setExpr(TernaryExpr);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // int a = 0
        ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntTypeRef, "a", EmptyModifiers);
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(Body, getASTBuilder().CreateVarRef(aVar));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "0"));
        aVarStmt->setExpr(Expr1);

        // if (a == 1)
        ASTVarRefExpr *aVarRef = getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aVar));
        ASTValueExpr *Value1 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "1"));
        ASTBinaryOpExpr *IfCond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ, aVarRef, Value1);

        // Create/Add if block
        SemaBuilderIfStmt *IfBuilder = getASTBuilder().CreateIfBuilder(Body);
        ASTBlockStmt *IfBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);

        // { a = 2 }
        SemaBuilderStmt *a2VarStmt = getASTBuilder().CreateAssignmentStmt(IfBlock, getASTBuilder().CreateVarRef(aVar));
        ASTValueExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "2"));
        a2VarStmt->setExpr(Expr2);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // func()
        llvm::SmallVector<ASTVar *, 8> Params;
        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // if (a == 1)
        ASTValueExpr *Value1 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "1"));
        ASTVarRefExpr *aVarRef = getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam));
        ASTBinaryOpExpr *IfCond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                aVarRef, Value1);

        // Create/Add if block
        SemaBuilderIfStmt *IfBuilder = getASTBuilder().CreateIfBuilder(Body);
        ASTBlockStmt *IfBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);

        // { a = 1 }
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(IfBlock, getASTBuilder().CreateVarRef(aParam));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "1"));
        aVarStmt->setExpr(Expr1);

        // else {a = 2}
        ASTBlockStmt *ElseBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        IfBuilder->Else(SourceLoc, ElseBlock);
        SemaBuilderStmt *aVarStmt2 = getASTBuilder().CreateAssignmentStmt(ElseBlock, getASTBuilder().CreateVarRef(aParam));
        ASTValueExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "2"));
        aVarStmt2->setExpr(Expr2);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // func()

        llvm::SmallVector<ASTVar *, 8> Params;
        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // if (a == 1)
        SemaBuilderIfStmt *IfBuilder = getASTBuilder().CreateIfBuilder(Body);
        ASTValueExpr *Value1 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "1"));
        ASTVarRefExpr *aVarRef = getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam));
        ASTBinaryOpExpr *IfCond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                aVarRef, Value1);
        ASTBlockStmt *IfBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);


        // { a = 11 }
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(IfBlock, getASTBuilder().CreateVarRef(aParam));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "11"));
        aVarStmt->setExpr(Expr1);

        // elsif (a == 2)
        ASTBlockStmt *ElsifBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value2 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "2"));
        ASTBinaryOpExpr *ElsifCond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                aVarRef, Value2);
        IfBuilder->ElseIf(SourceLoc, ElsifCond, ElsifBlock);
        // { a = 22 }
        SemaBuilderStmt *aVarStmt2 = getASTBuilder().CreateAssignmentStmt(ElsifBlock, getASTBuilder().CreateVarRef(aParam));
        ASTValueExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "22"));
        aVarStmt2->setExpr(Expr2);

        // elsif (a == 3) { a = 33 }
        ASTBlockStmt *ElsifBlock2 = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value3 = getASTBuilder().CreateExpr( getASTBuilder().CreateNumberValue(SourceLoc, "3"));
        ASTBinaryOpExpr *ElsifCond2 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                aVarRef, Value3);
        IfBuilder->ElseIf(SourceLoc, ElsifCond2, ElsifBlock2);
        SemaBuilderStmt *aVarStmt3 = getASTBuilder().CreateAssignmentStmt(ElsifBlock2, getASTBuilder().CreateVarRef(aParam));
        ASTValueExpr *Expr3 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "33"));
        aVarStmt3->setExpr(Expr3);

        // else {a == 44}
        ASTBlockStmt *ElseBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        IfBuilder->Else(SourceLoc, ElseBlock);
        SemaBuilderStmt *aVarStmt4 = getASTBuilder().CreateAssignmentStmt(ElseBlock, getASTBuilder().CreateVarRef(aParam));
        ASTValueExpr *Expr4 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "44"));
        aVarStmt4->setExpr(Expr4);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // main()
        llvm::SmallVector<ASTVar *, 8> Params;
        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // if a == 1
        SemaBuilderIfStmt *IfBuilder = getASTBuilder().CreateIfBuilder(Body);
        ASTBlockStmt *IfBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value1 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "1"));
        ASTVarRefExpr *aVarRef = getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam));
        ASTBinaryOpExpr *IfCond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ, aVarRef, Value1);

        // { a = 11 }
        IfBuilder->If(SourceLoc, IfCond, IfBlock);
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(IfBlock, getASTBuilder().CreateVarRef(aParam));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "11"));
        aVarStmt->setExpr(Expr1);

        // elsif (a == 2)
        ASTBlockStmt *ElsifBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value2 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "2"));
        ASTBinaryOpExpr *ElsifCond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                aVarRef, Value2);
        IfBuilder->ElseIf(SourceLoc, ElsifCond, ElsifBlock);
        // { a = 22 }
        SemaBuilderStmt *aVarStmt2 = getASTBuilder().CreateAssignmentStmt(ElsifBlock, getASTBuilder().CreateVarRef(aParam));
        ASTValueExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "22"));
        aVarStmt2->setExpr(Expr2);

        // elsif (a == 3) { a = 33 }
        ASTBlockStmt *ElsifBlock2 = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value3 = getASTBuilder().CreateExpr( getASTBuilder().CreateNumberValue(SourceLoc, "3"));
        ASTBinaryOpExpr *ElsifCond2 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                aVarRef, Value3);
        IfBuilder->ElseIf(SourceLoc, ElsifCond2, ElsifBlock2);
        SemaBuilderStmt *aVarStmt3 = getASTBuilder().CreateAssignmentStmt(ElsifBlock2, getASTBuilder().CreateVarRef(aParam));
        ASTValueExpr *Expr3 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "33"));
        aVarStmt3->setExpr(Expr3);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // main()
        llvm::SmallVector<ASTVar *, 8> Params;
        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // switch a
        SemaBuilderSwitchStmt *SwitchBuilder = getASTBuilder().CreateSwitchBuilder(Body);
        ASTVarRefExpr *aVarRefExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam));
        SwitchBuilder->Switch(SourceLoc, aVarRefExpr);

        // case 1: a = 1 break
        ASTBlockStmt *Case1Block = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTValueExpr *Case1Value = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "1"));
        SwitchBuilder->Case(SourceLoc, Case1Value, Case1Block);
        SemaBuilderStmt *aVarStmt1 = getASTBuilder().CreateAssignmentStmt(Case1Block, getASTBuilder().CreateVarRef(aParam));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "1"));
        aVarStmt1->setExpr(Expr1);

        // case 2: a = 2 break
        ASTBlockStmt *Case2Block = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTValueExpr *Case2Value = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "2"));
        SwitchBuilder->Case(SourceLoc, Case2Value, Case2Block);
        SemaBuilderStmt *aVarStmt2 = getASTBuilder().CreateAssignmentStmt(Case2Block, getASTBuilder().CreateVarRef(aParam));
        ASTValueExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "2"));
        aVarStmt2->setExpr(Expr2);

        // default: a = 3
        ASTBlockStmt *DefaultBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        SwitchBuilder->Default(SourceLoc, DefaultBlock);
        SemaBuilderStmt *aVarStmt3 = getASTBuilder().CreateAssignmentStmt(DefaultBlock, getASTBuilder().CreateVarRef(aParam));
        ASTValueExpr *Expr3 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "3"));
        aVarStmt3->setExpr(Expr3);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // main()
        llvm::SmallVector<ASTVar *, 8> Params;

        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // while a == 1
        SemaBuilderLoopStmt *LoopBuilder = getASTBuilder().CreateLoopBuilder(Body, SourceLoc);
        ASTValueExpr *Value1 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "1"));
        ASTVarRefExpr *aVarRef = getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(aParam));
        ASTBinaryOpExpr *Cond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ, aVarRef, Value1);
        ASTBlockStmt *BlockStmt = getASTBuilder().CreateBlockStmt(SourceLoc);
        LoopBuilder->Loop(Cond, BlockStmt);

        // { a = 1 }
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(BlockStmt, getASTBuilder().CreateVarRef(aParam));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "1"));
        aVarStmt->setExpr(Expr1);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // main()
        llvm::SmallVector<ASTVar *, 8> Params;
        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // for int i = 1; i < 1; ++i
        SemaBuilderLoopStmt *LoopBuilder = getASTBuilder().CreateLoopBuilder(Body, SourceLoc);

        // Init
        // int i = 1
        ASTBlockStmt *InitBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        LoopBuilder->Init(InitBlock);
        ASTVar *iVar = getASTBuilder().CreateLocalVar(InitBlock, SourceLoc, IntTypeRef, "i", EmptyModifiers);
        ASTRef *iVarRef = getASTBuilder().CreateVarRef(iVar);
        SemaBuilderStmt *iVarStmt = getASTBuilder().CreateAssignmentStmt(InitBlock, iVarRef);
        ASTValueExpr *Value1Expr = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "1"));
        iVarStmt->setExpr(Value1Expr);

        // Condition
        // i < 1
        ASTBinaryOpExpr *Cond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_LTE,
                getASTBuilder().CreateExpr(iVarRef), Value1Expr);
        ASTBlockStmt *LoopBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        LoopBuilder->Loop(Cond, LoopBlock);

        // Post
        // ++i
        ASTBlockStmt *PostBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        LoopBuilder->Post(PostBlock);
        ASTUnaryOpExpr *IncExpr = getASTBuilder().CreateUnaryOpExpr(SourceLoc, ASTUnaryOpExprKind::OP_UNARY_PRE_INCR,
                getASTBuilder().CreateExpr(getASTBuilder().CreateVarRef(iVar)));
        SemaBuilderStmt *iVarIncStmt = getASTBuilder().CreateExprStmt(PostBlock, SourceLoc);
        iVarIncStmt->setExpr(IncExpr);

        // Loop Block
        // { a = 1 }
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(LoopBlock, getASTBuilder().CreateVarRef(aParam));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateNumberValue(SourceLoc, "1"));
        aVarStmt->setExpr(Expr1);

    	// validate and resolve
    	EXPECT_TRUE(S->Resolve());

    	// CreateVTable Code
    	llvm::Module * M = Generate();
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
        ASTModule *Module = CreateModule();

        // main() {
        //
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "main", TopModifiers, Params, Body);

		// validate and resolve
		EXPECT_TRUE(S->Resolve());

		// CreateVTable Code
		llvm::Module * M = Generate();
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
