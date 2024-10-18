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
#include "CodeGen/CodeGenGlobalVar.h"
#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGenClass.h"
#include "Sema/SemaBuilder.h"
#include "Sema/SemaBuilderScopes.h"
#include "Sema/SemaBuilderStmt.h"
#include "Sema/SemaBuilderIfStmt.h"
#include "Sema/SemaBuilderSwitchStmt.h"
#include "Sema/SemaBuilderLoopStmt.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTAssignmentStmt.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTParam.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTEnum.h"
#include "AST/ASTEnumType.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTOpExpr.h"
#include "AST/ASTOperatorExpr.h"


namespace {

    using namespace fly;

    TEST_F(CodeGenTest, CGDefaultValueGlobalVar) {
        ASTModule *Module = CreateModule();

        // default Bool value
        ASTValueExpr *DefaultBoolVal = Builder.CreateExpr(Builder.CreateDefaultValue(BoolType));
        ASTGlobalVar *aVar = Builder.CreateGlobalVar(Module, SourceLoc, BoolType, "a", TopScopes, DefaultBoolVal);

        // default Byte value
        ASTValueExpr *DefaultByteVal = Builder.CreateExpr(Builder.CreateDefaultValue(ByteType));
        ASTGlobalVar *bVar = Builder.CreateGlobalVar(Module, SourceLoc, ByteType, "b", TopScopes, DefaultByteVal);

        // default Short value
        ASTValueExpr *DefaultShortVal = Builder.CreateExpr(Builder.CreateDefaultValue(ShortType));
        ASTGlobalVar *cVar = Builder.CreateGlobalVar(Module, SourceLoc, ShortType, "c", TopScopes, DefaultShortVal);

        // default UShort value
        ASTValueExpr *DefaultUShortVal = Builder.CreateExpr(Builder.CreateDefaultValue(UShortType));
        ASTGlobalVar *dVar = Builder.CreateGlobalVar(Module, SourceLoc, UShortType, "d", TopScopes, DefaultUShortVal);

        // default Int value
        ASTValueExpr *DefaultIntVal = Builder.CreateExpr(Builder.CreateDefaultValue(IntType));
        ASTGlobalVar *eVar = Builder.CreateGlobalVar(Module, SourceLoc, IntType, "e", TopScopes, DefaultIntVal);

        // default UInt value
        ASTValueExpr *DefaultUintVal = Builder.CreateExpr(Builder.CreateDefaultValue(UIntType));
        ASTGlobalVar *fVar = Builder.CreateGlobalVar(Module, SourceLoc, UIntType, "f", TopScopes, DefaultUintVal);

        // default Long value
        ASTValueExpr *DefaultLongVal = Builder.CreateExpr(Builder.CreateDefaultValue(LongType));
        ASTGlobalVar *gVar = Builder.CreateGlobalVar(Module, SourceLoc, LongType, "g", TopScopes, DefaultLongVal);

        // default ULong value
        ASTValueExpr *DefaultULongVal = Builder.CreateExpr(Builder.CreateDefaultValue(ULongType));
        ASTGlobalVar *hVar = Builder.CreateGlobalVar(Module, SourceLoc, ULongType, "h", TopScopes, DefaultULongVal);

        // default Float value
        ASTValueExpr *DefaultFloatVal = Builder.CreateExpr(Builder.CreateDefaultValue(FloatType));
        ASTGlobalVar *iVar = Builder.CreateGlobalVar(Module, SourceLoc, FloatType, "i", TopScopes, DefaultFloatVal);

        // default Double value
        ASTValueExpr *DefaultDoubleVal = Builder.CreateExpr(Builder.CreateDefaultValue(DoubleType));
        ASTGlobalVar *jVar = Builder.CreateGlobalVar(Module, SourceLoc, DoubleType, "j", TopScopes, DefaultDoubleVal);

        // default Array value
        ASTValueExpr *DefaultArrayVal = Builder.CreateExpr(Builder.CreateDefaultValue(ArrayInt0Type));
        ASTGlobalVar *kVar = Builder.CreateGlobalVar(Module, SourceLoc, ArrayInt0Type, "k", TopScopes, DefaultArrayVal);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        EXPECT_FALSE(Diags.hasErrorOccurred());
        std::string output;

        // a
        GlobalVariable *aGVar = (GlobalVariable *) CGM->GenGlobalVar(aVar)->getPointer();
        testing::internal::CaptureStdout();
        aGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@a = global i1 false");

        // b
        GlobalVariable *bGVar = (GlobalVariable *) CGM->GenGlobalVar(bVar)->getPointer();
        testing::internal::CaptureStdout();
        bGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@b = global i8 0");

        // c
        GlobalVariable *cGVar = (GlobalVariable *) CGM->GenGlobalVar(cVar)->getPointer();
        testing::internal::CaptureStdout();
        cGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@c = global i16 0");

        // d
        GlobalVariable *dGVar = (GlobalVariable *) CGM->GenGlobalVar(dVar)->getPointer();
        testing::internal::CaptureStdout();
        dGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@d = global i16 0");

        // e
        GlobalVariable *eGVar = (GlobalVariable *) CGM->GenGlobalVar(eVar)->getPointer();
        testing::internal::CaptureStdout();
        eGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@e = global i32 0");

        // f
        GlobalVariable *fGVar = (GlobalVariable *) CGM->GenGlobalVar(fVar)->getPointer();
        testing::internal::CaptureStdout();
        fGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@f = global i32 0");

        // g
        GlobalVariable *gGVar = (GlobalVariable *) CGM->GenGlobalVar(gVar)->getPointer();
        testing::internal::CaptureStdout();
        gGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@g = global i64 0");

        // h
        GlobalVariable *hGVar = (GlobalVariable *) CGM->GenGlobalVar(hVar)->getPointer();
        testing::internal::CaptureStdout();
        hGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@h = global i64 0");

        // i
        GlobalVariable *iGVar = (GlobalVariable *) CGM->GenGlobalVar(iVar)->getPointer();
        testing::internal::CaptureStdout();
        iGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@i = global float 0.000000e+00");

        // l
        GlobalVariable *jGVar = (GlobalVariable *) CGM->GenGlobalVar(jVar)->getPointer();
        testing::internal::CaptureStdout();
        jGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@j = global double 0.000000e+00");

        GlobalVariable *kGVar = (GlobalVariable *) CGM->GenGlobalVar(kVar)->getPointer();
        testing::internal::CaptureStdout();
        kGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@k = global [0 x i32] zeroinitializer");
    }

    TEST_F(CodeGenTest, CGValuedGlobalVar) {
        ASTModule *Module = CreateModule();

        // a
        ASTBoolValue *BoolVal = Builder.CreateBoolValue(SourceLoc, true);
        ASTGlobalVar *aVar = Builder.CreateGlobalVar(Module, SourceLoc, BoolType, "a", TopScopes, Builder.CreateExpr(BoolVal));

        // b
        ASTIntegerValue *ByteVal = Builder.CreateIntegerValue(SourceLoc, 1);
        ASTGlobalVar *bVar = Builder.CreateGlobalVar(Module, SourceLoc, ByteType, "b", TopScopes, Builder.CreateExpr(ByteVal));

        // c
        ASTIntegerValue *ShortVal = Builder.CreateIntegerValue(SourceLoc, 2, true);
        ASTGlobalVar *cVar = Builder.CreateGlobalVar(Module, SourceLoc, ShortType, "c", TopScopes, Builder.CreateExpr(ShortVal));

        // d
        ASTIntegerValue *UShortVal = Builder.CreateIntegerValue(SourceLoc, 2);
        ASTGlobalVar *dVar = Builder.CreateGlobalVar(Module, SourceLoc, UShortType, "d", TopScopes, Builder.CreateExpr(UShortVal));

        // e
        ASTIntegerValue *IntVal = Builder.CreateIntegerValue(SourceLoc, 3, true);
        ASTGlobalVar *eVar = Builder.CreateGlobalVar(Module, SourceLoc, IntType, "e", TopScopes, Builder.CreateExpr(IntVal));

        // f
        ASTIntegerValue *UIntVal = Builder.CreateIntegerValue(SourceLoc, 3);
        ASTGlobalVar *fVar = Builder.CreateGlobalVar(Module, SourceLoc, UIntType, "f", TopScopes, Builder.CreateExpr(UIntVal));

        // g
        ASTIntegerValue *LongVal = Builder.CreateIntegerValue(SourceLoc, 4, true);
        ASTGlobalVar *gVar = Builder.CreateGlobalVar(Module, SourceLoc, LongType, "g", TopScopes, Builder.CreateExpr(LongVal));

        // h
        ASTIntegerValue *ULongVal = Builder.CreateIntegerValue(SourceLoc, 4);
        ASTGlobalVar *hVar = Builder.CreateGlobalVar(Module, SourceLoc, ULongType, "h", TopScopes, Builder.CreateExpr(ULongVal));

        // i
        ASTFloatingValue *FloatVal = Builder.CreateFloatingValue(SourceLoc, 1.5);
        ASTGlobalVar *iVar = Builder.CreateGlobalVar(Module, SourceLoc, FloatType, "i", TopScopes, Builder.CreateExpr(FloatVal));

        // j
        ASTFloatingValue *DoubleVal = Builder.CreateFloatingValue(SourceLoc, 2.5);
        ASTGlobalVar *jVar = Builder.CreateGlobalVar(Module, SourceLoc, DoubleType, "j", TopScopes, Builder.CreateExpr(DoubleVal));

        // k (empty array)
        llvm::SmallVector<ASTValue *, 8> Empty;
        ASTArrayValue *ArrayValEmpty = Builder.CreateArrayValue(SourceLoc, Empty);
        ASTGlobalVar *kVar = Builder.CreateGlobalVar(Module, SourceLoc, ArrayInt0Type, "k", TopScopes, Builder.CreateExpr(ArrayValEmpty));

        // l (array with 2 val)
        llvm::SmallVector<ASTValue *, 8> Values;
        Values.push_back(Builder.CreateIntegerValue(SourceLoc, 1)); // ArrayVal = {1}
        Values.push_back(Builder.CreateIntegerValue(SourceLoc, 2)); // ArrayVal = {1, 1}
        ASTArrayValue *ArrayVal = Builder.CreateArrayValue(SourceLoc, Values);
        ASTValueExpr *SizeExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, ArrayVal->size()));
        ASTArrayType *ArrayInt2Type = Builder.CreateArrayType(SourceLoc, IntType, SizeExpr);
        ASTGlobalVar *lVar = Builder.CreateGlobalVar(Module, SourceLoc, ArrayInt2Type, "l", TopScopes, Builder.CreateExpr(ArrayVal));

        // m (string)
        ASTStringType *StringType = Builder.CreateStringType(SourceLoc);
        ASTStringValue *StringVal = Builder.CreateStringValue(SourceLoc, "hello");
        ASTGlobalVar *mVar = Builder.CreateGlobalVar(Module, SourceLoc, StringType, "m", TopScopes, Builder.CreateExpr(StringVal));

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        EXPECT_FALSE(Diags.hasErrorOccurred());
        std::string output;

        // a
        GlobalVariable *aGVar = (GlobalVariable *) CGM->GenGlobalVar(aVar)->getPointer();
        testing::internal::CaptureStdout();
        aGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@a = global i1 true");

        // b
        GlobalVariable *bGVar = (GlobalVariable *) CGM->GenGlobalVar(bVar)->getPointer();
        testing::internal::CaptureStdout();
        bGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@b = global i8 1");

        // c
        GlobalVariable *cGVar = (GlobalVariable *) CGM->GenGlobalVar(cVar)->getPointer();
        testing::internal::CaptureStdout();
        cGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@c = global i16 -2");

        // d
        GlobalVariable *dGVar = (GlobalVariable *) CGM->GenGlobalVar(dVar)->getPointer();
        testing::internal::CaptureStdout();
        dGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@d = global i16 2");

        // e
        GlobalVariable *eGVar = (GlobalVariable *) CGM->GenGlobalVar(eVar)->getPointer();
        testing::internal::CaptureStdout();
        eGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@e = global i32 -3");

        // f
        GlobalVariable *fGVar = (GlobalVariable *) CGM->GenGlobalVar(fVar)->getPointer();
        testing::internal::CaptureStdout();
        fGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@f = global i32 3");

        // g
        GlobalVariable *gGVar = (GlobalVariable *) CGM->GenGlobalVar(gVar)->getPointer();
        testing::internal::CaptureStdout();
        gGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@g = global i64 -4");

        // h
        GlobalVariable *hGVar = (GlobalVariable *) CGM->GenGlobalVar(hVar)->getPointer();
        testing::internal::CaptureStdout();
        hGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@h = global i64 4");

        // i
        GlobalVariable *iGVar = (GlobalVariable *) CGM->GenGlobalVar(iVar)->getPointer();
        testing::internal::CaptureStdout();
        iGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@i = global float 1.500000e+00");

        // j
        GlobalVariable *jGVar = (GlobalVariable *) CGM->GenGlobalVar(jVar)->getPointer();
        testing::internal::CaptureStdout();
        jGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@j = global double 2.500000e+00");

        GlobalVariable *kGVar = (GlobalVariable *) CGM->GenGlobalVar(kVar)->getPointer();
        testing::internal::CaptureStdout();
        kGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@k = global [0 x i32] zeroinitializer");

        GlobalVariable *lGVar = (GlobalVariable *) CGM->GenGlobalVar(lVar)->getPointer();
        testing::internal::CaptureStdout();
        lGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@l = global [2 x i32] [i32 1, i32 2]");

        GlobalVariable *mGVar = (GlobalVariable *) CGM->GenGlobalVar(mVar)->getPointer();
        testing::internal::CaptureStdout();
        mGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@m = global [6 x i8] c\"hello\\00\"");
    }

    TEST_F(CodeGenTest, CGFuncParamTypes) {
        ASTModule *Module = CreateModule();

        llvm::SmallVector<ASTParam *, 8> Params;
        Params.push_back(Builder.CreateParam(SourceLoc, IntType, "P1", EmptyScopes));
        Params.push_back(Builder.CreateParam(SourceLoc, FloatType, "P2", EmptyScopes));
        Params.push_back(Builder.CreateParam(SourceLoc, BoolType, "P3", EmptyScopes));
        Params.push_back(Builder.CreateParam(SourceLoc, LongType, "P4", EmptyScopes));
        Params.push_back(Builder.CreateParam(SourceLoc, DoubleType, "P5", EmptyScopes));
        Params.push_back(Builder.CreateParam(SourceLoc, ByteType, "P6", EmptyScopes));
        Params.push_back(Builder.CreateParam(SourceLoc, ShortType, "P7", EmptyScopes));
        Params.push_back(Builder.CreateParam(SourceLoc, UShortType, "P8", EmptyScopes));
        Params.push_back(Builder.CreateParam(SourceLoc, UIntType, "P9", EmptyScopes));
        Params.push_back(Builder.CreateParam(SourceLoc, ULongType, "P10", EmptyScopes));
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);

        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);
        // func(int P1, float P2, bool P3, long P4, double P5, byte P6, short P7, ushort P8, uint P9, ulong P10) {
        // }
        
        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1, float %2, i1 %3, i64 %4, double %5, i8 %6, i16 %7, i16 %8, i32 %9, i64 %10) {\n"
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

    TEST_F(CodeGenTest, CGFuncUseGlobalVar) {
        ASTModule *Module = CreateModule();

        // float G = 2.0
        ASTFloatingValue *FloatingVal = Builder.CreateFloatingValue(SourceLoc, 2.0);
        ASTGlobalVar *GVar = Builder.CreateGlobalVar(Module, SourceLoc, FloatType, "G", TopScopes,
                                                     Builder.CreateExpr(FloatingVal));

        // func()
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, IntType, "func", TopScopes, Params, Body);

        // int A = 1
        ASTLocalVar *VarA = Builder.CreateLocalVar(Body, SourceLoc, IntType, "A", EmptyScopes);
        SemaBuilderStmt *VarAStmt = Builder.CreateAssignmentStmt(Body, VarA);
        ASTExpr *ExprA = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        VarAStmt->setExpr(ExprA);

        // GlobalVar
        // G = 1
        ASTVarRef *VarRefG = CreateVarRef(GVar);
        SemaBuilderStmt * GVarStmt = Builder.CreateAssignmentStmt(Body, VarRefG);
        ASTExpr *ExprG = Builder.CreateExpr(Builder.CreateFloatingValue(SourceLoc, 1));
        GVarStmt->setExpr(ExprG);

        // return A
        SemaBuilderStmt *Return = Builder.CreateReturnStmt(Body, SourceLoc);
        ASTExpr *ExprRA = Builder.CreateExpr(CreateVarRef(VarA));
        Return->setExpr(ExprRA);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        CodeGenGlobalVar *CGGV = CGM->GenGlobalVar(GVar);
        Value *G = CGGV->getPointer();

        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());

        testing::internal::CaptureStdout();
        G->print(llvm::outs());
        std::string output1 = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output1, "@G = global float 2.000000e+00");

        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output2 = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output2, "define i32 @func(%error* %0) {\n"
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

    TEST_F(CodeGenTest, CGValue) {
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);
        
        // int a = 1
        ASTLocalVar *LocalVar = Builder.CreateLocalVar(Body, SourceLoc, Builder.CreateIntType(SourceLoc), "a", EmptyScopes);
        SemaBuilderStmt *VarStmt = Builder.CreateAssignmentStmt(Body, LocalVar);
        ASTValueExpr *ValueExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        VarStmt->setExpr(ValueExpr);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0) {\n"
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
        ASTBlockStmt *BodyTest = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Test = Builder.CreateFunction(Module, SourceLoc, IntType, "test", TopScopes, Params, BodyTest);

        // func()
        ASTBlockStmt *BodyFunc = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, IntType, "func", TopScopes, Params, BodyFunc);

        // call test()
        SemaBuilderStmt *ExprStmt = Builder.CreateExprStmt(BodyFunc, SourceLoc);
        ASTCall *TestCall = CreateCall(Test, Args);
        ASTCallExpr *Expr = Builder.CreateExpr(TestCall);
        ExprStmt->setExpr(Expr);

        //return test()
        SemaBuilderStmt *Return = Builder.CreateReturnStmt(BodyFunc, SourceLoc);
        ASTCallExpr *ReturnExpr = Builder.CreateExpr(TestCall);
        Return->setExpr(ReturnExpr);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        CodeGenFunction *CGF_Test = CGM->GenFunction(Test);
        CGF_Test->GenBody();
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = load %error*, %error** %1, align 8\n"
                          "  %3 = call i32 @test(%error* %2)\n"
                          "  %4 = call i32 @test(%error* %2)\n"
                          "  ret i32 %4\n"
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
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTParam *bParam = Builder.CreateParam(SourceLoc, IntType, "b", EmptyScopes);
        Params.push_back(bParam);
        ASTParam *cParam = Builder.CreateParam(SourceLoc, IntType, "c", EmptyScopes);
        Params.push_back(cParam);
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, IntType, "func", TopScopes, Params, Body);

        SemaBuilderStmt *Return = Builder.CreateReturnStmt(Body, SourceLoc);
        // Create this expression: 1 + a * b / (c - 2)
        // E1 + (E2 * E3) / (E4 - E5)
        // E1 + (G2 / G3)
        // E1 + G1
        ASTValueExpr *E1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *E2 = Builder.CreateExpr(CreateVarRef(aParam));
        ASTVarRefExpr *E3 = Builder.CreateExpr(CreateVarRef(bParam));
        ASTVarRefExpr *E4 = Builder.CreateExpr(CreateVarRef(cParam));
        ASTValueExpr *E5 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));

        ASTBinaryOpExpr *G2 = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_MUL), E2, E3);
        ASTBinaryOpExpr *G3 = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_SUB), E4, E5);
        ASTBinaryOpExpr *G1 = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_DIV), G2, G3);
        ASTBinaryOpExpr *Group = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_ADD), E1, G1);

        Return->setExpr(Group);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @func(%error* %0, i32 %1, i32 %2, i32 %3) {\n"
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
        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTParam *bParam = Builder.CreateParam(SourceLoc, IntType, "b", EmptyScopes);
        Params.push_back(bParam);
        ASTParam *cParam = Builder.CreateParam(SourceLoc, IntType, "c", EmptyScopes);
        Params.push_back(cParam);
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // a = 0
        SemaBuilderStmt *aVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(aParam));
        ASTExpr *AssignExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 0));
        aVarStmt->setExpr(AssignExpr);

        // b = 0
        SemaBuilderStmt *bVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(bParam));
        AssignExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 0));
        bVarStmt->setExpr(AssignExpr);

        // c = a + b
        SemaBuilderStmt * cAddVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_ADD),
                Builder.CreateExpr(CreateVarRef(aParam)),
                Builder.CreateExpr(CreateVarRef(bParam)));
        cAddVarStmt->setExpr(AssignExpr);

        // c = a - b
        SemaBuilderStmt * cSubVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_SUB),
                Builder.CreateExpr(CreateVarRef(aParam)),
                Builder.CreateExpr(CreateVarRef(bParam)));
        cSubVarStmt->setExpr(AssignExpr);

        // c = a * b
        SemaBuilderStmt * cMulVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_MUL),
                Builder.CreateExpr(CreateVarRef(aParam)),
                Builder.CreateExpr(CreateVarRef(bParam)));
        cMulVarStmt->setExpr(AssignExpr);

        // c = a / b
        SemaBuilderStmt * cDivVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_DIV),
                Builder.CreateExpr(CreateVarRef(aParam)),
                Builder.CreateExpr(CreateVarRef(bParam)));
        cDivVarStmt->setExpr(AssignExpr);

        // c = a % b
        SemaBuilderStmt * cModVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_MOD),
                Builder.CreateExpr(CreateVarRef(aParam)),
                Builder.CreateExpr(CreateVarRef(bParam)));
        cModVarStmt->setExpr(AssignExpr);

        // c = a & b
        SemaBuilderStmt * cAndVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_AND),
                Builder.CreateExpr(CreateVarRef(aParam)),
                Builder.CreateExpr(CreateVarRef(bParam)));
        cAndVarStmt->setExpr(AssignExpr);

        // c = a | b
        SemaBuilderStmt * cOrVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_OR),
                Builder.CreateExpr(CreateVarRef(aParam)),
                Builder.CreateExpr(CreateVarRef(bParam)));
        cOrVarStmt->setExpr(AssignExpr);

        // c = a xor b
        SemaBuilderStmt * cXorVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_XOR),
                Builder.CreateExpr(CreateVarRef(aParam)),
                Builder.CreateExpr(CreateVarRef(bParam)));
        cXorVarStmt->setExpr(AssignExpr);

        // c = a << b
        SemaBuilderStmt * cShlVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_SHIFT_L),
                Builder.CreateExpr(CreateVarRef(aParam)),
                Builder.CreateExpr(CreateVarRef(bParam)));
        cShlVarStmt->setExpr(AssignExpr);

        // c = a >> b
        SemaBuilderStmt * cShrVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_SHIFT_R),
                Builder.CreateExpr(CreateVarRef(aParam)),
                Builder.CreateExpr(CreateVarRef(bParam)));
        cShrVarStmt->setExpr(AssignExpr);

        // ++c
        SemaBuilderStmt *cPreIncVarStmt = Builder.CreateExprStmt(Body, SourceLoc);
        AssignExpr = Builder.CreateUnaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTUnaryOpExprKind::UNARY_ARITH_PRE_INCR),
                Builder.CreateExpr(CreateVarRef(cParam)));
        cPreIncVarStmt->setExpr(AssignExpr);

        // c++
        SemaBuilderStmt *cPostIncVarStmt = Builder.CreateExprStmt(Body, SourceLoc);
        AssignExpr = Builder.CreateUnaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTUnaryOpExprKind::UNARY_ARITH_POST_INCR),
                Builder.CreateExpr(CreateVarRef(cParam)));
        cPostIncVarStmt->setExpr(AssignExpr);

        // ++c
        SemaBuilderStmt *cPreDecVarStmt = Builder.CreateExprStmt(Body, SourceLoc);
        AssignExpr = Builder.CreateUnaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTUnaryOpExprKind::UNARY_ARITH_PRE_DECR),
                Builder.CreateExpr(CreateVarRef(cParam)));
        cPreDecVarStmt->setExpr(AssignExpr);

        // c++
        SemaBuilderStmt *cPostDecVarStmt = Builder.CreateExprStmt(Body, SourceLoc);
        AssignExpr = Builder.CreateUnaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTUnaryOpExprKind::UNARY_ARITH_PRE_DECR),
                Builder.CreateExpr(CreateVarRef(cParam)));
        cPostDecVarStmt->setExpr(AssignExpr);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1, i32 %2, i32 %3) {\n"
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
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);

        ASTLocalVar *aVar = Builder.CreateLocalVar(Body, SourceLoc, IntType, "a", EmptyScopes);
        ASTLocalVar *bVar = Builder.CreateLocalVar(Body, SourceLoc, IntType, "b", EmptyScopes);
        ASTLocalVar *cVar = Builder.CreateLocalVar(Body, SourceLoc, BoolType, "c", EmptyScopes);

        // a = 0
        SemaBuilderStmt *aVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(aVar));
        ASTExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 0));
        aVarStmt->setExpr(Expr1);

        // b = 0
        SemaBuilderStmt *bVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(bVar));
        ASTExpr *Expr2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 0));
        bVarStmt->setExpr(Expr2);

        // c = a == b
        SemaBuilderStmt * cEqVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr3 = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                Builder.CreateExpr(CreateVarRef(aVar)),
                Builder.CreateExpr(CreateVarRef(bVar)));
        cEqVarStmt->setExpr(Expr3);

        // c = a != b
        SemaBuilderStmt * cNeqVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr4 = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_NE),
                Builder.CreateExpr(CreateVarRef(aVar)),
                Builder.CreateExpr(CreateVarRef(bVar)));
        cNeqVarStmt->setExpr(Expr4);

        // c = a > b
        SemaBuilderStmt * cGtVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr5 = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_GT),
                Builder.CreateExpr(CreateVarRef(aVar)),
                Builder.CreateExpr(CreateVarRef(bVar)));
        cGtVarStmt->setExpr(Expr5);

        // c = a >= b
        SemaBuilderStmt * cGteVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr6 = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_GTE),
                Builder.CreateExpr(CreateVarRef(aVar)),
                Builder.CreateExpr(CreateVarRef(bVar)));
        cGteVarStmt->setExpr(Expr6);

        // c = a < b
        SemaBuilderStmt * cLtVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr7 = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_LT),
                Builder.CreateExpr(CreateVarRef(aVar)),
                Builder.CreateExpr(CreateVarRef(bVar)));
        cLtVarStmt->setExpr(Expr7);

        // c = a <= b
        SemaBuilderStmt * cLteVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr8 = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_LTE),
                Builder.CreateExpr(CreateVarRef(aVar)),
                Builder.CreateExpr(CreateVarRef(bVar)));
        cLteVarStmt->setExpr(Expr8);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0) {\n"
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
        CodeGenModule *CGM = CG->GenerateModule(*Module);

        // func()
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);

        ASTLocalVar *aVar = Builder.CreateLocalVar(Body, SourceLoc, BoolType, "a", EmptyScopes);
        ASTLocalVar *bVar = Builder.CreateLocalVar(Body, SourceLoc, BoolType, "b", EmptyScopes);
        ASTLocalVar *cVar = Builder.CreateLocalVar(Body, SourceLoc, BoolType, "c", EmptyScopes);

        // a = false
        SemaBuilderStmt *aVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(aVar));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateBoolValue(SourceLoc, false));
        aVarStmt->setExpr(Expr1);

        // b = false
        SemaBuilderStmt *bVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(bVar));
        ASTValueExpr *Expr2 = Builder.CreateExpr(Builder.CreateBoolValue(SourceLoc, false));
        bVarStmt->setExpr(Expr2);

        // c = a and b
        SemaBuilderStmt * cAndVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr3 = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_LOGIC_AND),
                Builder.CreateExpr(CreateVarRef(aVar)),
                Builder.CreateExpr(CreateVarRef(bVar)));
        cAndVarStmt->setExpr(Expr3);

        // c = a or b
        SemaBuilderStmt * cOrVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr4 = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_LOGIC_OR),
                Builder.CreateExpr(CreateVarRef(aVar)),
                Builder.CreateExpr(CreateVarRef(bVar)));
        cOrVarStmt->setExpr(Expr4);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0) {\n"
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
        CodeGenModule *CGM = CG->GenerateModule(*Module);

        // func()
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);

        ASTLocalVar *aVar = Builder.CreateLocalVar(Body, SourceLoc, BoolType, "a", EmptyScopes);
        ASTLocalVar *bVar = Builder.CreateLocalVar(Body, SourceLoc, BoolType, "b", EmptyScopes);
        ASTLocalVar *cVar = Builder.CreateLocalVar(Body, SourceLoc, BoolType, "c", EmptyScopes);

        // a = false
        SemaBuilderStmt *aVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(aVar));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateBoolValue(SourceLoc, false));
        aVarStmt->setExpr(Expr1);

        // b = false
        SemaBuilderStmt *bVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(bVar));
        ASTValueExpr *Expr2 = Builder.CreateExpr(Builder.CreateBoolValue(SourceLoc, false));
        bVarStmt->setExpr(Expr2);

        // c = a == b ? a : b
        SemaBuilderStmt * cVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTBinaryOpExpr *Cond = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                Builder.CreateExpr(CreateVarRef(aVar)),
                Builder.CreateExpr(CreateVarRef(bVar)));

        ASTTernaryOpExpr *TernaryExpr = Builder.CreateTernaryOpExpr(Cond,
                                                                    Builder.CreateOperatorExpr(SourceLoc,
                                                                                               ASTTernaryOpExprKind::TERNARY_IF),
                                                                    Builder.CreateExpr(CreateVarRef(aVar)),
                                                                    Builder.CreateOperatorExpr(SourceLoc,
                                                                                               ASTTernaryOpExprKind::TERNARY_ELSE),
                                                                    Builder.CreateExpr(CreateVarRef(bVar)));
        cVarStmt->setExpr(TernaryExpr);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0) {\n"
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
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // int a = 0
        ASTLocalVar *aVar = Builder.CreateLocalVar(Body, SourceLoc, IntType, "a", EmptyScopes);
        SemaBuilderStmt *aVarStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(aVar));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 0));
        aVarStmt->setExpr(Expr1);

        // if (a == 1)
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(CreateVarRef(aVar));
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTBinaryOpExpr *IfCond = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ), aVarRef, Value1);

        // Create/Add if block
        SemaBuilderIfStmt *IfBuilder = Builder.CreateIfBuilder(Body);
        ASTBlockStmt *IfBlock = Builder.CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);

        // { a = 2 }
        SemaBuilderStmt *a2VarStmt = Builder.CreateAssignmentStmt(IfBlock, CreateVarRef(aVar));
        ASTValueExpr *Expr2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        a2VarStmt->setExpr(Expr2);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0) {\n"
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
        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // if (a == 1)
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(CreateVarRef(aParam));
        ASTBinaryOpExpr *IfCond = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                aVarRef, Value1);

        // Create/Add if block
        SemaBuilderIfStmt *IfBuilder = Builder.CreateIfBuilder(Body);
        ASTBlockStmt *IfBlock = Builder.CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);

        // { a = 1 }
        SemaBuilderStmt *aVarStmt = Builder.CreateAssignmentStmt(IfBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        aVarStmt->setExpr(Expr1);

        // else {a = 2}
        ASTBlockStmt *ElseBlock = Builder.CreateBlockStmt(SourceLoc);
        IfBuilder->Else(SourceLoc, ElseBlock);
        SemaBuilderStmt *aVarStmt2 = Builder.CreateAssignmentStmt(ElseBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        aVarStmt2->setExpr(Expr2);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1) {\n"
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

        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // if (a == 1)
        SemaBuilderIfStmt *IfBuilder = Builder.CreateIfBuilder(Body);
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(CreateVarRef(aParam));
        ASTBinaryOpExpr *IfCond = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                aVarRef, Value1);
        ASTBlockStmt *IfBlock = Builder.CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);


        // { a = 11 }
        SemaBuilderStmt *aVarStmt = Builder.CreateAssignmentStmt(IfBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 11));
        aVarStmt->setExpr(Expr1);

        // elsif (a == 2)
        ASTBlockStmt *ElsifBlock = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        ASTBinaryOpExpr *ElsifCond = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                aVarRef, Value2);
        IfBuilder->ElseIf(SourceLoc, ElsifCond, ElsifBlock);
        // { a = 22 }
        SemaBuilderStmt *aVarStmt2 = Builder.CreateAssignmentStmt(ElsifBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 22));
        aVarStmt2->setExpr(Expr2);

        // elsif (a == 3) { a = 33 }
        ASTBlockStmt *ElsifBlock2 = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value3 = Builder.CreateExpr( Builder.CreateIntegerValue(SourceLoc, 3));
        ASTBinaryOpExpr *ElsifCond2 = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                aVarRef, Value3);
        IfBuilder->ElseIf(SourceLoc, ElsifCond2, ElsifBlock2);
        SemaBuilderStmt *aVarStmt3 = Builder.CreateAssignmentStmt(ElsifBlock2, CreateVarRef(aParam));
        ASTValueExpr *Expr3 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 33));
        aVarStmt3->setExpr(Expr3);

        // else {a == 44}
        ASTBlockStmt *ElseBlock = Builder.CreateBlockStmt(SourceLoc);
        IfBuilder->Else(SourceLoc, ElseBlock);
        SemaBuilderStmt *aVarStmt4 = Builder.CreateAssignmentStmt(ElseBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr4 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 44));
        aVarStmt4->setExpr(Expr4);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1) {\n"
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
        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // if a == 1
        SemaBuilderIfStmt *IfBuilder = Builder.CreateIfBuilder(Body);
        ASTBlockStmt *IfBlock = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(CreateVarRef(aParam));
        ASTBinaryOpExpr *IfCond = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ), aVarRef, Value1);

        // { a = 11 }
        IfBuilder->If(SourceLoc, IfCond, IfBlock);
        SemaBuilderStmt *aVarStmt = Builder.CreateAssignmentStmt(IfBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 11));
        aVarStmt->setExpr(Expr1);

        // elsif (a == 2)
        ASTBlockStmt *ElsifBlock = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        ASTBinaryOpExpr *ElsifCond = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                aVarRef, Value2);
        IfBuilder->ElseIf(SourceLoc, ElsifCond, ElsifBlock);
        // { a = 22 }
        SemaBuilderStmt *aVarStmt2 = Builder.CreateAssignmentStmt(ElsifBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 22));
        aVarStmt2->setExpr(Expr2);

        // elsif (a == 3) { a = 33 }
        ASTBlockStmt *ElsifBlock2 = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value3 = Builder.CreateExpr( Builder.CreateIntegerValue(SourceLoc, 3));
        ASTBinaryOpExpr *ElsifCond2 = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                aVarRef, Value3);
        IfBuilder->ElseIf(SourceLoc, ElsifCond2, ElsifBlock2);
        SemaBuilderStmt *aVarStmt3 = Builder.CreateAssignmentStmt(ElsifBlock2, CreateVarRef(aParam));
        ASTValueExpr *Expr3 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 33));
        aVarStmt3->setExpr(Expr3);

        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1) {\n"
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
        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // switch a
        SemaBuilderSwitchStmt *SwitchBuilder = Builder.CreateSwitchBuilder(Body);
        ASTVarRefExpr *aVarRefExpr = Builder.CreateExpr(CreateVarRef(aParam));
        SwitchBuilder->Switch(SourceLoc, aVarRefExpr);

        // case 1: a = 1 break
        ASTBlockStmt *Case1Block = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Case1Value = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        SwitchBuilder->Case(SourceLoc, Case1Value, Case1Block);
        SemaBuilderStmt *aVarStmt1 = Builder.CreateAssignmentStmt(Case1Block, CreateVarRef(aParam));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        aVarStmt1->setExpr(Expr1);

        // case 2: a = 2 break
        ASTBlockStmt *Case2Block = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Case2Value = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        SwitchBuilder->Case(SourceLoc, Case2Value, Case2Block);
        SemaBuilderStmt *aVarStmt2 = Builder.CreateAssignmentStmt(Case2Block, CreateVarRef(aParam));
        ASTValueExpr *Expr2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        aVarStmt2->setExpr(Expr2);

        // default: a = 3
        ASTBlockStmt *DefaultBlock = Builder.CreateBlockStmt(SourceLoc);
        SwitchBuilder->Default(SourceLoc, DefaultBlock);
        SemaBuilderStmt *aVarStmt3 = Builder.CreateAssignmentStmt(DefaultBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr3 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 3));
        aVarStmt3->setExpr(Expr3);

        // Add switch
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1) {\n"
                          "entry:\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  %3 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %2, align 8\n"
                          "  store i32 %1, i32* %3, align 4\n"
                          "  %4 = load i32, i32* %3, align 4\n"
                          "  switch i32 %4, label %default [\n"
                          "    i8 1, label %case\n"
                          "    i8 2, label %case1\n"
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
        llvm::SmallVector<ASTParam *, 8> Params;

        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // while a == 1
        SemaBuilderLoopStmt *LoopBuilder = Builder.CreateLoopBuilder(Body, SourceLoc);
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(CreateVarRef(aParam));
        ASTBinaryOpExpr *Cond = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ), aVarRef, Value1);
        ASTBlockStmt *BlockStmt = Builder.CreateBlockStmt(SourceLoc);
        LoopBuilder->Loop(Cond, BlockStmt);

        // { a = 1 }
        SemaBuilderStmt *aVarStmt = Builder.CreateAssignmentStmt(BlockStmt, CreateVarRef(aParam));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        aVarStmt->setExpr(Expr1);

        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1) {\n"
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
        llvm::SmallVector<ASTParam *, 8> Params;
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // for int i = 1; i < 1; ++i
        SemaBuilderLoopStmt *LoopBuilder = Builder.CreateLoopBuilder(Body, SourceLoc);

        // Init
        // int i = 1
        ASTBlockStmt *InitBlock = Builder.CreateBlockStmt(SourceLoc);
        LoopBuilder->Init(InitBlock);
        ASTLocalVar *iVar = Builder.CreateLocalVar(InitBlock, SourceLoc, IntType, "i", EmptyScopes);
        ASTVarRef *iVarRef = CreateVarRef(iVar);
        SemaBuilderStmt *iVarStmt = Builder.CreateAssignmentStmt(InitBlock, iVarRef);
        ASTValueExpr *Value1Expr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        iVarStmt->setExpr(Value1Expr);


        // Condition
        // i < 1
        ASTBinaryOpExpr *Cond = Builder.CreateBinaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_LTE),
                Builder.CreateExpr(iVarRef), Value1Expr);
        ASTBlockStmt *LoopBlock = Builder.CreateBlockStmt(SourceLoc);
        LoopBuilder->Loop(Cond, LoopBlock);

        // Post
        // ++i
        ASTBlockStmt *PostBlock = Builder.CreateBlockStmt(SourceLoc);
        LoopBuilder->Post(PostBlock);
        ASTUnaryOpExpr *IncExpr = Builder.CreateUnaryOpExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTUnaryOpExprKind::UNARY_ARITH_PRE_INCR),
                Builder.CreateExpr(CreateVarRef(iVar)));
        SemaBuilderStmt *iVarIncStmt = Builder.CreateExprStmt(PostBlock, SourceLoc);
        iVarIncStmt->setExpr(IncExpr);

        // Loop Block
        // { a = 1 }
        SemaBuilderStmt *aVarStmt = Builder.CreateAssignmentStmt(LoopBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        aVarStmt->setExpr(Expr1);

        // Resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(*Module);
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1) {\n"
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

    TEST_F(CodeGenTest, CGClassMethods) {
        ASTModule *Module = CreateModule();

        // TestClass {
        //   int a() { return 1 }
        //   public int b() { return 1 }
        //   private const int c { return 1 }
        // }
        llvm::SmallVector<ASTClassType *, 4> SuperClasses;
        ASTClass *TestClass = Builder.CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass",
                                                  TopScopes, SuperClasses);

        // int a() { return 1 }
        ASTBlockStmt *aFuncBody = Builder.CreateBlockStmt(SourceLoc);
        ASTClassMethod *aFunc = Builder.CreateClassMethod(SourceLoc, *TestClass, IntType,
                                                          "a",
                                                          TopScopes, Params, aFuncBody);

        SemaBuilderStmt *aFuncReturn = Builder.CreateReturnStmt(aFuncBody, SourceLoc);
        ASTValueExpr *aFuncExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLocation(), 1));
        aFuncReturn->setExpr(aFuncExpr);


        // public int b() { return 1 }
        ASTBlockStmt *bFuncBody = Builder.CreateBlockStmt(SourceLoc);
        ASTClassMethod *bFunc = Builder.CreateClassMethod(SourceLoc, *TestClass, IntType,
                                                          "b", TopScopes, Params, bFuncBody);
        SemaBuilderStmt *bFuncReturn = Builder.CreateReturnStmt(bFuncBody, SourceLoc);
        ASTValueExpr *bFuncExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLocation(), 1));
        bFuncReturn->setExpr(bFuncExpr);

        // private const int c { return 1 }
        ASTBlockStmt *cFuncBody = Builder.CreateBlockStmt(SourceLoc);
        ASTClassMethod *cFunc = Builder.CreateClassMethod(SourceLoc, *TestClass, IntType,
                                                          "c", TopScopes, Params, cFuncBody);
        SemaBuilderStmt *cFuncReturn = Builder.CreateReturnStmt(cFuncBody, SourceLoc);
        ASTValueExpr *cFuncExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLocation(), 1));
        cFuncReturn->setExpr(cFuncExpr);

        // int main() {
        //  TestClass test = new TestClass()
        //  int a = test.a()
        //  int b = test.b()
        //  int c = test.c()
        //  delete test
        // }
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // TestClass test = new TestClass()
        ASTClassType *TestClassType = Builder.CreateClassType(TestClass);
        ASTLocalVar *TestVar = Builder.CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyScopes);
        ASTClassMethod *DefaultConstructor = TestClass->getDefaultConstructor();
        ASTCall *ConstructorCall = CreateNew(DefaultConstructor, Args);
        ASTCallExpr *NewExpr = Builder.CreateExpr(ConstructorCall);
        SemaBuilderStmt *testNewStmt = Builder.CreateAssignmentStmt(Body, TestVar);
        testNewStmt->setExpr(NewExpr);

        // int a = test.a()
        ASTType *aType = aFunc->getReturnType();
        ASTLocalVar *aVar = Builder.CreateLocalVar(Body, SourceLoc, aType, "a", EmptyScopes);
        ASTCallExpr *aCallExpr = Builder.CreateExpr(CreateCall(aFunc, Args, CreateVarRef(TestVar)));
        SemaBuilderStmt *aStmt = Builder.CreateAssignmentStmt(Body, aVar);
        aStmt->setExpr(aCallExpr);

        // int b = test.b()
        ASTType *bType = bFunc->getReturnType();
        ASTLocalVar *bVar = Builder.CreateLocalVar(Body, SourceLoc, bType, "b", EmptyScopes);
        ASTCallExpr *bCallExpr = Builder.CreateExpr(CreateCall(bFunc, Args, CreateVarRef(TestVar)));
        SemaBuilderStmt *bStmt = Builder.CreateAssignmentStmt(Body, bVar);
        bStmt->setExpr(bCallExpr);

        // int c = test.c()
        ASTType *cType = cFunc->getReturnType();
        ASTLocalVar *cVar = Builder.CreateLocalVar(Body, SourceLoc, cType, "c", EmptyScopes);
        ASTCallExpr *cCallExpr = Builder.CreateExpr(CreateCall(cFunc, Args, CreateVarRef(TestVar)));
        SemaBuilderStmt *cStmt = Builder.CreateAssignmentStmt(Body, cVar);
        cStmt->setExpr(cCallExpr);

        // delete test
        ASTDeleteStmt *DeleteStmt = Builder.CreateDeleteStmt(Body, SourceLoc, (ASTVarRef *) CreateVarRef(TestVar));

        bool Success = S->Resolve();
        EXPECT_TRUE(Success);

        if (Success) {

            CodeGenModule *CGM = CG->GenerateModule(*Module);
            // Generate Code
            CodeGenClass *CGC = CGM->GenClass(TestClass);
            for (auto &F : CGC->getConstructors()) {
                F->GenBody();
            }
            for (auto &F : CGC->getFunctions()) {
                F->GenBody();
            }
            CodeGenFunction *CGF = CGM->GenFunction(Func);
            CGF->GenBody();

            EXPECT_FALSE(Diags.hasErrorOccurred());
            std::string output = getOutput(CGM->getModule());

            EXPECT_EQ(output, "%error = type { i8, i32, i8* }\n"
                              "%TestClass = type { %TestClass_vtable* }\n"
                              "%TestClass_vtable = type { i32 (%error*, %TestClass*), i32 (%error*, %TestClass*), i32 (%error*, %TestClass*) }\n"
                              "\n"
                              "define void @TestClass_TestClass(%error* %0, %TestClass* %1) {\n"
                              "entry:\n"
                              "  %2 = alloca %error*, align 8\n"
                              "  %3 = alloca %TestClass*, align 8\n"
                              "  store %error* %0, %error** %2, align 8\n"
                              "  store %TestClass* %1, %TestClass** %3, align 8\n"
                              "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
                              "  ret void\n"
                              "}\n"
                              "\n"
                              "define i32 @TestClass_a(%error* %0, %TestClass* %1) {\n"
                              "entry:\n"
                              "  %2 = alloca %error*, align 8\n"
                              "  %3 = alloca %TestClass*, align 8\n"
                              "  store %error* %0, %error** %2, align 8\n"
                              "  store %TestClass* %1, %TestClass** %3, align 8\n"
                              "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
                              "  ret i32 1\n"
                              "}\n"
                              "\n"
                              "define i32 @TestClass_b(%error* %0, %TestClass* %1) {\n"
                              "entry:\n"
                              "  %2 = alloca %error*, align 8\n"
                              "  %3 = alloca %TestClass*, align 8\n"
                              "  store %error* %0, %error** %2, align 8\n"
                              "  store %TestClass* %1, %TestClass** %3, align 8\n"
                              "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
                              "  ret i32 1\n"
                              "}\n"
                              "\n"
                              "define i32 @TestClass_c(%error* %0, %TestClass* %1) {\n"
                              "entry:\n"
                              "  %2 = alloca %error*, align 8\n"
                              "  %3 = alloca %TestClass*, align 8\n"
                              "  store %error* %0, %error** %2, align 8\n"
                              "  store %TestClass* %1, %TestClass** %3, align 8\n"
                              "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
                              "  ret i32 1\n"
                              "}\n"
                              "\n"
                              "define void @func(%error* %0) {\n"
                              "entry:\n"
                              "  %1 = alloca %error*, align 8\n"
                              "  %2 = alloca %TestClass*, align 8\n"
                              "  %3 = alloca i32, align 4\n"
                              "  %4 = alloca i32, align 4\n"
                              "  %5 = alloca i32, align 4\n"
                              "  store %error* %0, %error** %1, align 8\n"
                              "  %6 = load %error*, %error** %1, align 8\n"
                              "  %7 = tail call i8* @malloc(i8 ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i8))\n"
                              "  call void @TestClass_TestClass(%error* %6, i8* %7)\n"
                              "  store i8* %7, %TestClass** %2, align 8\n"
                              "  %8 = load %TestClass*, %TestClass** %2, align 8\n"
                              "  %9 = call i32 @TestClass_a(%error* %6, %TestClass* %8)\n"
                              "  store i32 %9, i32* %3, align 4\n"
                              "  %10 = load %TestClass*, %TestClass** %2, align 8\n"
                              "  %11 = call i32 @TestClass_b(%error* %6, %TestClass* %10)\n"
                              "  store i32 %11, i32* %4, align 4\n"
                              "  %12 = load %TestClass*, %TestClass** %2, align 8\n"
                              "  %13 = call i32 @TestClass_c(%error* %6, %TestClass* %12)\n"
                              "  store i32 %13, i32* %5, align 4\n"
                              "  %14 = load %TestClass*, %TestClass** %2, align 8\n"
                              "  %15 = bitcast %TestClass* %14 to i8*\n"
                              "  tail call void @free(i8* %15)\n"
                              "  ret void\n"
                              "}\n"
                              "\n"
                              "declare noalias i8* @malloc(i64)\n"
                              "\n"
                              "declare void @free(i8*)\n"
                              );
        }
    }

    TEST_F(CodeGenTest, CGClassAttributes) {
        ASTModule *Module = CreateModule();

        // TestClass {
        //   int a
        //   int getA() { return a }
        // }
        llvm::SmallVector<ASTClassType *, 4> SuperClasses;
        ASTClass *TestClass = Builder.CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass", TopScopes,
                                                  SuperClasses);

        // int a
        ASTClassAttribute *aAttribute = Builder.CreateClassAttribute(SourceLoc, *TestClass,
                                                                     Builder.CreateIntType(SourceLoc),
                                                                     "a", TopScopes);

        // int getA() { return a }
        ASTBlockStmt *MethodBody = Builder.CreateBlockStmt(SourceLoc);
        ASTClassMethod *getAMethod = Builder.CreateClassMethod(SourceLoc, *TestClass, IntType,
                                                               "getA", TopScopes, Params, MethodBody);

        SemaBuilderStmt *MethodReturn = Builder.CreateReturnStmt(MethodBody, SourceLoc);
        MethodReturn->setExpr(Builder.CreateExpr(CreateVarRef(aAttribute)));

        // int main() {
        //  TestClass test = new TestClass()
        //  int x = test.getA()
        //  test.a = 2
        //  delete test
        // }
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // TestClass test = new TestClass()
        ASTType *TestClassType = Builder.CreateClassType(TestClass);
        ASTLocalVar *TestVar = Builder.CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyScopes);
        ASTClassMethod *DefaultConstructor = TestClass->getDefaultConstructor();
        ASTCall *ConstructorCall = CreateNew(DefaultConstructor, Args);
        ASTCallExpr *NewExpr = Builder.CreateExpr(ConstructorCall);
        SemaBuilderStmt *testNewStmt = Builder.CreateAssignmentStmt(Body, TestVar);
        testNewStmt->setExpr(NewExpr);

        // int x = test.m()
        ASTType *xType = getAMethod->getReturnType();
        ASTLocalVar *xVar = Builder.CreateLocalVar(Body, SourceLoc, xType, "x", EmptyScopes);
        ASTCallExpr *xCallExpr = Builder.CreateExpr(CreateCall(getAMethod, Args, CreateVarRef(TestVar)));
        SemaBuilderStmt *xStmt = Builder.CreateAssignmentStmt(Body, xVar);
        xStmt->setExpr(xCallExpr);

        //  test.a = 2
        SemaBuilderStmt *attrStmt = Builder.CreateAssignmentStmt(Body, CreateVarRef(aAttribute, CreateVarRef(TestVar)));
        ASTValueExpr *value2Expr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLocation(), 2));
        attrStmt->setExpr(value2Expr);

        // delete test
        ASTDeleteStmt *DeleteStmt = Builder.CreateDeleteStmt(Body, SourceLoc, CreateVarRef(TestVar));

        bool Success = S->Resolve();
        EXPECT_TRUE(Success);

        if (Success) {

            CodeGenModule *CGM = CG->GenerateModule(*Module);
            // Generate Code
            CodeGenClass *CGC = CGM->GenClass(TestClass);
            for (auto &F : CGC->getConstructors()) {
                F->GenBody();
            }
            for (auto &F : CGC->getFunctions()) {
                F->GenBody();
            }
            CodeGenFunction *CGF = CGM->GenFunction(Func);
            CGF->GenBody();

            EXPECT_FALSE(Diags.hasErrorOccurred());
            std::string output = getOutput(CGM->getModule());

            EXPECT_EQ(output, "%error = type { i8, i32, i8* }\n"
                              "%TestClass = type { %TestClass_vtable*, i32 }\n"
                              "%TestClass_vtable = type { i32 (%error*, %TestClass*) }\n"
                              "\n"
                              "define void @TestClass_TestClass(%error* %0, %TestClass* %1) {\n"
                              "entry:\n"
                              "  %2 = alloca %error*, align 8\n"
                              "  %3 = alloca %TestClass*, align 8\n"
                              "  store %error* %0, %error** %2, align 8\n"
                              "  store %TestClass* %1, %TestClass** %3, align 8\n"
                              "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
                              "  %5 = getelementptr inbounds %TestClass, %TestClass* %4, i32 0, i32 1\n"
                              "  store i32 0, i32* %5, align 4\n"
                              "  ret void\n"
                              "}\n"
                              "\n"
                              "define i32 @TestClass_getA(%error* %0, %TestClass* %1) {\n"
                              "entry:\n"
                              "  %2 = alloca %error*, align 8\n"
                              "  %3 = alloca %TestClass*, align 8\n"
                              "  store %error* %0, %error** %2, align 8\n"
                              "  store %TestClass* %1, %TestClass** %3, align 8\n"
                              "  %4 = load %TestClass*, %TestClass** %3, align 8\n"
                              "  %5 = getelementptr inbounds %TestClass, %TestClass* %4, i32 0, i32 1\n"
                              "  %6 = load i32, i32* %5, align 4\n"
                              "  ret i32 %6\n"
                              "}\n"
                              "\n"
                              "define void @func(%error* %0) {\n"
                              "entry:\n"
                              "  %1 = alloca %error*, align 8\n"
                              "  %2 = alloca %TestClass*, align 8\n"
                              "  %3 = alloca i32, align 4\n"
                              "  store %error* %0, %error** %1, align 8\n"
                              "  %4 = load %error*, %error** %1, align 8\n"
                              // TestClass test = new TestClass()
                              "  %malloccall = tail call i8* @malloc(%TestClass ptrtoint (%TestClass* getelementptr (%TestClass, %TestClass* null, i32 1) to %TestClass))\n"
                              "  %5 = bitcast i8* %malloccall to %TestClass*\n"
                              "  call void @TestClass_TestClass(%error* %4, %TestClass* %5)\n"
                              "  store %TestClass* %5, %TestClass** %2, align 8\n"
                              "  %6 = load %TestClass*, %TestClass** %2, align 8\n"
                              // int a = test.a()
                              "  %7 = call i32 @TestClass_getA(%error* %4, %TestClass* %6)\n"
                              "  store i32 %7, i32* %3, align 4\n"
                              // test.a = 2
                              "  %8 = getelementptr inbounds %TestClass, %TestClass* %6, i32 0, i32 1\n"
                              "  store i32 2, i32* %8, align 4\n"
                              // delete test
                              "  %9 = load %TestClass*, %TestClass** %2, align 8\n"
                              "  %10 = bitcast %TestClass* %9 to i8*\n"
                              "  tail call void @free(i8* %10)\n"
                              "  ret void\n"
                              "}\n"
                              "\n"
                              "declare noalias i8* @malloc(i64)\n"
                              "\n"
                              "declare void @free(i8*)\n"
            );
        }
    }

    TEST_F(CodeGenTest, CGStruct) {
        ASTModule *Module = CreateModule();

        // struct TestStruct {
        //   int a
        //   int b
        //   const int c
        // }

        llvm::SmallVector<ASTClassType *, 4> SuperClasses;
        ASTClass *TestStruct = Builder.CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct",
                                                   TopScopes, SuperClasses);
        ASTClassAttribute *aField = Builder.CreateClassAttribute(SourceLoc, *TestStruct, Builder.CreateIntType(SourceLoc),
                                                                 "a", TopScopes);
        ASTClassAttribute *bField = Builder.CreateClassAttribute(SourceLoc, *TestStruct, Builder.CreateIntType(SourceLoc),
                                                                 "b", TopScopes);
        ASTClassAttribute *cField = Builder.CreateClassAttribute(SourceLoc, *TestStruct, Builder.CreateIntType(SourceLoc),
                                                                 "c", TopScopes);

        // int func() {
        //  TestStruct test = new TestStruct();
        //  int a = test.a
        //  test.b = 2
        //  return 1
        // }
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // TestStruct test = new TestStruct()
        ASTType *TestClassType = Builder.CreateClassType(TestStruct);
        ASTLocalVar *TestVar = Builder.CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyScopes);
        ASTClassMethod *DefaultConstructor = TestStruct->getDefaultConstructor();
        ASTCall *ConstructorCall = CreateNew(DefaultConstructor, Args);
        ASTCallExpr *NewExpr = Builder.CreateExpr(ConstructorCall);
        SemaBuilderStmt *testNewStmt = Builder.CreateAssignmentStmt(Body, TestVar);
        testNewStmt->setExpr(NewExpr);

        // int a = test.a
        ASTLocalVar *aVar = Builder.CreateLocalVar(Body, SourceLoc, IntType, "a", EmptyScopes);
        ASTVarRef *Instance = CreateVarRef(TestVar);
        ASTVarRef *test_aVarRef = CreateVarRef(aField,Instance);
        ASTVarRefExpr *test_aRefExpr = Builder.CreateExpr(test_aVarRef);
        SemaBuilderStmt *aVarStmt = Builder.CreateAssignmentStmt(Body, aVar);
        aVarStmt->setExpr(test_aRefExpr);

        // test.b = 2
        ASTVarRef *Instance2 = CreateVarRef(TestVar);
        ASTVarRef *test_bVarRef = CreateVarRef(bField, Instance2);
        SemaBuilderStmt *test_bVarStmt = Builder.CreateAssignmentStmt(Body, test_bVarRef);
        ASTExpr *Expr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        test_bVarStmt->setExpr(Expr);

        // delete test
        ASTDeleteStmt *Delete = Builder.CreateDeleteStmt(Body, SourceLoc, (ASTVarRef *) Instance);

        bool Success = S->Resolve();
        EXPECT_TRUE(Success);

        if (Success) {
            CodeGenModule *CGM = CG->GenerateModule(*Module);

            // Generate Code
            CodeGenClass *CGC = CGM->GenClass(TestStruct);
            for (auto &F : CGC->getConstructors()) {
                F->GenBody();
            }
            for (auto &F : CGC->getFunctions()) {
                F->GenBody();
            }
            CodeGenFunction *CGF = CGM->GenFunction(Func);
            CGF->GenBody();

            EXPECT_FALSE(Diags.hasErrorOccurred());
            std::string output = getOutput(CGM->getModule());

            EXPECT_EQ(output, "%TestStruct = type { i32, i32, i32 }\n"
                              "%error = type { i8, i32, i8* }\n"
                              "\n"
                              "define void @TestStruct_TestStruct(%TestStruct* %0) {\n"
                              "entry:\n"
                              "  %1 = alloca %TestStruct*, align 8\n"
                              "  store %TestStruct* %0, %TestStruct** %1, align 8\n"
                              "  %2 = load %TestStruct*, %TestStruct** %1, align 8\n"
                              "  %3 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 0\n"
                              "  store i32 0, i32* %3, align 4\n"
                              "  %4 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 1\n"
                              "  store i32 0, i32* %4, align 4\n"
                              "  %5 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 2\n"
                              "  store i32 0, i32* %5, align 4\n"
                              "  ret void\n"
                              "}\n"
                              "\n"
                              "define void @func(%error* %0) {\n"
                              "entry:\n"
                              "  %1 = alloca %error*, align 8\n"
                              "  %2 = alloca %TestStruct*, align 8\n"
                              "  %3 = alloca i32, align 4\n"
                              "  store %error* %0, %error** %1, align 8\n"
                              "  %malloccall = tail call i8* @malloc(%TestStruct trunc (i64 mul nuw (i64 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i64), i64 3) to %TestStruct))\n"
                              "  %4 = bitcast i8* %malloccall to %TestStruct*\n"
                              "  call void @TestStruct_TestStruct(%TestStruct* %4)\n"
                              "  store %TestStruct* %4, %TestStruct** %2, align 8\n"
                              "  %5 = load %TestStruct*, %TestStruct** %2, align 8\n"
                              "  %6 = getelementptr inbounds %TestStruct, %TestStruct* %5, i32 0, i32 0\n"
                              "  %7 = load i32, i32* %6, align 4\n"
                              "  store i32 %7, i32* %3, align 4\n"
                              "  %8 = getelementptr inbounds %TestStruct, %TestStruct* %5, i32 0, i32 1\n"
                              "  store i32 2, i32* %8, align 4\n"
                              "  %9 = load %TestStruct*, %TestStruct** %2, align 8\n"
                              "  %10 = bitcast %TestStruct* %9 to i8*\n"
                              "  tail call void @free(i8* %10)\n"
                              "  ret void\n"
                              "}\n"
                              "\n"
                              "declare noalias i8* @malloc(i64)\n"
                              "\n"
                              "declare void @free(i8*)\n");
        }
    }

    TEST_F(CodeGenTest, CGEnum) {
        ASTModule *Module = CreateModule();

        // enum TestEnum {
        //   A
        //   B
        //   C
        // }
        llvm::SmallVector<ASTEnumType *, 4> SuperEnums;
        ASTEnum *TestEnum = Builder.CreateEnum(Module, SourceLoc, "TestEnum", TopScopes, SuperEnums);
        ASTEnumEntry *A = Builder.CreateEnumEntry(SourceLoc, *TestEnum, "A", EmptyScopes);
        ASTEnumEntry *B = Builder.CreateEnumEntry(SourceLoc, *TestEnum, "B", EmptyScopes);
        ASTEnumEntry *C = Builder.CreateEnumEntry(SourceLoc, *TestEnum, "C", EmptyScopes);

        // int main() {
        //  TestEnum a = TestEnum.A;
        //  TestEnum b = a
        //  return 1
        // }
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes, Params, Body);

        ASTEnumType *TestEnumType = Builder.CreateEnumType(TestEnum);

        //  TestEnum a = TestEnum.A;
        ASTLocalVar *aVar = Builder.CreateLocalVar(Body, SourceLoc, TestEnumType, "a", EmptyScopes);
        ASTVarRef *Enum_AVarRef = CreateVarRef(A, Builder.CreateIdentifier(SourceLoc, TestEnumType->getName()));
        ASTVarRefExpr *Enum_ARefExpr = Builder.CreateExpr(Enum_AVarRef);
        SemaBuilderStmt *aVarStmt = Builder.CreateAssignmentStmt(Body, aVar);
        aVarStmt->setExpr(Enum_ARefExpr);

        //  TestEnum b = a
        ASTLocalVar *bVar = Builder.CreateLocalVar(Body, SourceLoc, TestEnumType, "b", EmptyScopes);
        SemaBuilderStmt *bVarStmt = Builder.CreateAssignmentStmt(Body, bVar);
        ASTVarRefExpr *aRefExpr = Builder.CreateExpr(CreateVarRef(aVar));
        bVarStmt->setExpr(aRefExpr);

        bool Success = S->Resolve();
        EXPECT_TRUE(Success);

        if (Success) {
            CodeGenModule *CGM = CG->GenerateModule(*Module);

            CGM->GenEnum(TestEnum);

            // Generate Code
            CodeGenFunction *CGF = CGM->GenFunction(Func);
            CGF->GenBody();

            EXPECT_FALSE(Diags.hasErrorOccurred());
            std::string output = getOutput(CGM->getModule());

            EXPECT_EQ(output, "%error = type { i8, i32, i8* }\n"
                              "\n"
                              "define void @func(%error* %0) {\n"
                              "entry:\n"
                              "  %1 = alloca %error*, align 8\n"
                              "  %2 = alloca i32, align 4\n"
                              "  %3 = alloca i32, align 4\n"
                              "  store %error* %0, %error** %1, align 8\n"
                              "  store i32 1, i32* %2, align 4\n"
                              "  %4 = load i32, i32* %2, align 4\n"
                              "  store i32 %4, i32* %3, align 4\n"
                              "  ret void\n"
                              "}\n");
        }
    }

    TEST_F(CodeGenTest, CGErrorFail) {
        ASTModule *Module = CreateModule();

        // int testFail0() {
        //   fail
        // }
        ASTBlockStmt *Body0 = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail0 = Builder.CreateFunction(Module, SourceLoc, IntType, "testFail0", TopScopes, Params, Body0);
        SemaBuilderStmt *Fail0Stmt = Builder.CreateFailStmt(Body0, SourceLoc);

        // int testFail1() {
        //   fail true
        // }
        ASTBlockStmt *Body1 = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail1 = Builder.CreateFunction(Module, SourceLoc, IntType, "testFail1", TopScopes, Params, Body1);
        ASTBoolValue *BoolVal = Builder.CreateBoolValue(SourceLoc, true);
        SemaBuilderStmt *Fail1Stmt = Builder.CreateFailStmt(Body1, SourceLoc);
        Fail1Stmt->setExpr(Builder.CreateExpr(BoolVal));

        // int testFail2() {
        //   fail 10
        // }
        ASTBlockStmt *Body2 = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail2 = Builder.CreateFunction(Module, SourceLoc, IntType, "testFail2", TopScopes, Params, Body2);
        ASTIntegerValue *IntVal = Builder.CreateIntegerValue(SourceLoc, 10);
        SemaBuilderStmt *Fail2Stmt = Builder.CreateFailStmt(Body2, SourceLoc);
        Fail2Stmt->setExpr(Builder.CreateExpr(IntVal));

        // int testFail3() {
        //  fail "Error"
        // }
        ASTBlockStmt *Body3 = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail3 = Builder.CreateFunction(Module, SourceLoc, IntType, "testFail3", TopScopes, Params, Body3);
        ASTStringValue *StrVal = Builder.CreateStringValue(SourceLoc, "Error");
        SemaBuilderStmt *Fail3Stmt = Builder.CreateFailStmt(Body3, SourceLoc);
        Fail3Stmt->setExpr(Builder.CreateExpr(StrVal));

        // int testFail4() {
        //  fail new TestStruct()
        // }
        llvm::SmallVector<ASTClassType *, 4> SuperClasses;
        ASTClass *TestStruct = Builder.CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct", TopScopes, SuperClasses);
        ASTClassAttribute *aField = Builder.CreateClassAttribute(SourceLoc, *TestStruct, Builder.CreateIntType(SourceLoc), "a", TopScopes);

        ASTBlockStmt *Body4 = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail4 = Builder.CreateFunction(Module, SourceLoc, IntType, "testFail4", TopScopes, Params, Body4);
        // TestStruct test = new TestStruct()
        ASTType *TestClassType = Builder.CreateClassType(TestStruct);
        ASTClassMethod *DefaultConstructor = TestStruct->getDefaultConstructor();
        ASTCall *ConstructorCall = CreateNew(DefaultConstructor, Args);
        // fail new TestStruct()
        SemaBuilderStmt *Fail4Stmt = Builder.CreateFailStmt(Body4, SourceLoc);
        Fail4Stmt->setExpr(Builder.CreateExpr(ConstructorCall));

        // main() {
        //   testFail0()
        //   testFail1()
        //   testFail2()
        //   testFail3()
        //   testFail4()
        // }
        ASTBlockStmt *MainBody = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Main = Builder.CreateFunction(Module, SourceLoc, VoidType, "main", TopScopes, Params, MainBody);

        // call testFail0()
        SemaBuilderStmt *CallTestFail0 = Builder.CreateExprStmt(MainBody, SourceLoc);
        ASTCallExpr *CallExpr0 = Builder.CreateExpr(CreateCall(TestFail0, Args));
        CallTestFail0->setExpr(CallExpr0);

        // call testFail1()
        SemaBuilderStmt *CallTestFail1 = Builder.CreateExprStmt(MainBody, SourceLoc);
        ASTCallExpr *CallExpr1 = Builder.CreateExpr(CreateCall(TestFail1, Args));
        CallTestFail1->setExpr(CallExpr1);

        // call testFail2()
        SemaBuilderStmt *CallTestFail2 = Builder.CreateExprStmt(MainBody, SourceLoc);
        ASTCallExpr *CallExpr2 = Builder.CreateExpr(CreateCall(TestFail2, Args));
        CallTestFail2->setExpr(CallExpr2);

        // call testFail3()
        SemaBuilderStmt *CallTestFail3 = Builder.CreateExprStmt(MainBody, SourceLoc);
        ASTCallExpr *CallExpr3 = Builder.CreateExpr(CreateCall(TestFail3, Args));
        CallTestFail3->setExpr(CallExpr3);

        // call testFail4()
        SemaBuilderStmt *CallTestFail4 = Builder.CreateExprStmt(MainBody, SourceLoc);
        ASTCallExpr *CallExpr4 = Builder.CreateExpr(CreateCall(TestFail4, Args));
        CallTestFail4->setExpr(CallExpr4);

        // Validate and Resolve
        EXPECT_TRUE(S->Resolve());

        CodeGenModule *CGM = CG->GenerateModule(*Module);

        // Generate Code
        CodeGenClass *CGC = CGM->GenClass(TestStruct);
        for (auto &F : CGC->getConstructors()) {
            F->GenBody();
        }

        // Generate Code
        CodeGenFunction *CGF_TestFail0 = CGM->GenFunction(TestFail0);
        CGF_TestFail0->GenBody();
        llvm::Function *F_TestFail0 = CGF_TestFail0->getFunction();

        CodeGenFunction *CGF_TestFail1 = CGM->GenFunction(TestFail1);
        CGF_TestFail1->GenBody();
        llvm::Function *F_TestFail1 = CGF_TestFail1->getFunction();

        CodeGenFunction *CGF_TestFail2 = CGM->GenFunction(TestFail2);
        CGF_TestFail2->GenBody();
        llvm::Function *F_TestFail2 = CGF_TestFail2->getFunction();

        CodeGenFunction *CGF_TestFail3 = CGM->GenFunction(TestFail3);
        CGF_TestFail3->GenBody();
        llvm::Function *F_TestFail3 = CGF_TestFail3->getFunction();

        CodeGenFunction *CGF_TestFail4 = CGM->GenFunction(TestFail4);
        CGF_TestFail4->GenBody();
        llvm::Function *F_TestFail4 = CGF_TestFail4->getFunction();

        CodeGenFunction *CGF_Main = CGM->GenFunction(Main);
        CGF_Main->GenBody();
        llvm::Function *F_Main = CGF_Main->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F_TestFail0->print(llvm::outs());
        F_TestFail1->print(llvm::outs());
        F_TestFail2->print(llvm::outs());
        F_TestFail3->print(llvm::outs());
        F_TestFail4->print(llvm::outs());
        F_Main->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @testFail0(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = load %error*, %error** %1, align 8\n"
                          "  %3 = getelementptr inbounds %error, %error* %2, i32 0, i32 0\n"
                          "  store i8 1, i8* %3, align 1\n"
                          "  ret i32 0\n"
                          "}\n"
                          "define i32 @testFail1(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = load %error*, %error** %1, align 8\n"
                          "  %3 = getelementptr inbounds %error, %error* %2, i32 0, i32 0\n"
                          "  store i8 1, i8* %3, align 1\n"
                          "  %4 = getelementptr inbounds %error, %error* %2, i32 0, i32 1\n"
                          "  store i1 true, i32* %4, align 1\n"
                          "  ret i32 0\n"
                          "}\n"
                          "define i32 @testFail2(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = load %error*, %error** %1, align 8\n"
                          "  %3 = getelementptr inbounds %error, %error* %2, i32 0, i32 0\n"
                          "  store i8 1, i8* %3, align 1\n"
                          "  %4 = getelementptr inbounds %error, %error* %2, i32 0, i32 1\n"
                          "  store i8 10, i32* %4, align 1\n"
                          "  ret i32 0\n"
                          "}\n"
                          "define i32 @testFail3(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %2 = load %error*, %error** %1, align 8\n"
                          "  %3 = getelementptr inbounds %error, %error* %2, i32 0, i32 0\n"
                          "  store i8 2, i8* %3, align 1\n"
                          "  %4 = getelementptr inbounds %error, %error* %2, i32 0, i32 2\n"
                          "  store i8* getelementptr inbounds ([6 x i8], [6 x i8]* @0, i32 0, i32 0), i8** %4, align 8\n"
                          "  ret i32 0\n"
                          "}\n"
                          "define i32 @testFail4(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  %malloccall = tail call i8* @malloc(%TestStruct ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to %TestStruct))\n"
                          "  %2 = bitcast i8* %malloccall to %TestStruct*\n"
                          "  call void @TestStruct_TestStruct(%TestStruct* %2)\n"
                          "  %3 = load %error*, %error** %1, align 8\n"
                          "  %4 = getelementptr inbounds %error, %error* %3, i32 0, i32 0\n"
                          "  store i8 3, i8* %4, align 1\n"
                          "  %5 = getelementptr inbounds %error, %error* %3, i32 0, i32 2\n"
                          "  store %TestStruct* %2, i8** %5, align 8\n"
                          "  ret i32 0\n"
                          "}\n"
                          "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca %error*, align 8\n"
                          "  %1 = load %error*, %error** %0, align 8\n"
                          "  %2 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
                          "  store i8 0, i8* %2, align 1\n"
                          "  %3 = getelementptr inbounds %error, %error* %1, i32 0, i32 1\n"
                          "  store i32 0, i32* %3, align 4\n"
                          "  %4 = getelementptr inbounds %error, %error* %1, i32 0, i32 2\n"
                          "  store i8* null, i8** %4, align 8\n"
                          "  %5 = call i32 @testFail0(%error* %1)\n"
                          "  %6 = call i32 @testFail1(%error* %1)\n"
                          "  %7 = call i32 @testFail2(%error* %1)\n"
                          "  %8 = call i32 @testFail3(%error* %1)\n"
                          "  %9 = call i32 @testFail4(%error* %1)\n"
                          "  %10 = getelementptr inbounds %error, %error* %1, i32 0, i32 0\n"
                          "  %11 = load i8, i8* %10, align 1\n"
                          "  %12 = icmp ne i8 %11, 0\n"
                          "  %13 = zext i1 %12 to i32\n"
                          "  ret i32 %13\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGErrorHandle) {
        ASTModule *Module = CreateModule();

        // main() {
        //   error A = handle fail
        // }
        ASTBlockStmt *Body = Builder.CreateBlockStmt(SourceLoc);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "main", TopScopes, Params, Body);
        ASTLocalVar *VarA = Builder.CreateLocalVar(Body, SourceLoc, ErrorType, "A", EmptyScopes);
        ASTVarRef *ErrorVarRef = Builder.CreateVarRef(VarA);
        ASTHandleStmt *HandleStmt = Builder.CreateHandleStmt(Body, SourceLoc, ErrorVarRef);
        ASTBlockStmt *HandleBlock = Builder.CreateBlockStmt(HandleStmt, SourceLoc);
        SemaBuilderStmt *Fail0Stmt = Builder.CreateFailStmt(HandleBlock, SourceLoc);

        // Validate and Resolve
        EXPECT_TRUE(S->Resolve());

        CodeGenModule *CGM = CG->GenerateModule(*Module);

        CodeGenFunction *CGF_Main = CGM->GenFunction(Func);
        CGF_Main->GenBody();
        llvm::Function *F_Main = CGF_Main->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F_Main->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca %error*, align 8\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = load %error*, %error** %0, align 8\n"
                          "  %3 = getelementptr inbounds %error, %error* %2, i32 0, i32 0\n"
                          "  store i8 0, i8* %3, align 1\n"
                          "  %4 = getelementptr inbounds %error, %error* %2, i32 0, i32 1\n"
                          "  store i32 0, i32* %4, align 4\n"
                          "  %5 = getelementptr inbounds %error, %error* %2, i32 0, i32 2\n"
                          "  store i8* null, i8** %5, align 8\n"
                          "  %6 = getelementptr inbounds %error, %error* %2, i32 0, i32 0\n"
                          "  %7 = load i8, i8* %6, align 1\n"
                          "  %8 = icmp ne i8 %7, 0\n"
                          "  %9 = zext i1 %8 to i32\n"
                          "  ret i32 %9\n"
                          "}\n");
    }
} // anonymous namespace
