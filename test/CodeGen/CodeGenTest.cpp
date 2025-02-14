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
#include "CodeGenTest.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenGlobalVar.h"
#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGenClass.h"
#include "Sema/SemaBuilderScopes.h"
#include "Sema/SemaBuilderStmt.h"
#include "Sema/SemaBuilderIfStmt.h"
#include "Sema/SemaBuilderSwitchStmt.h"
#include "Sema/SemaBuilderLoopStmt.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTVarRef.h"
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

#include <Sym/SymFunction.h>
#include <Sym/SymGlobalVar.h>
#include <Sym/SymModule.h>


namespace {

    using namespace fly;

    TEST_F(CodeGenTest, CGDefaultValueGlobalVar) {
        SymModule *Module = CreateModule();

        // default Bool value
        ASTValueExpr *DefaultBoolVal = getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(BoolType->getType()));
        ASTVar *aVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, BoolType, "a", TopScopes, DefaultBoolVal);

        // default Byte value
        ASTValueExpr *DefaultByteVal = getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(ByteType->getType()));
        ASTVar *bVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, ByteType, "b", TopScopes, DefaultByteVal);

        // default Short value
        ASTValueExpr *DefaultShortVal = getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(ShortType->getType()));
        ASTVar *cVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, ShortType, "c", TopScopes, DefaultShortVal);

        // default UShort value
        ASTValueExpr *DefaultUShortVal = getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(UShortType->getType()));
        ASTVar *dVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, UShortType, "d", TopScopes, DefaultUShortVal);

        // default Int value
        ASTValueExpr *DefaultIntVal = getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(IntType->getType()));
        ASTVar *eVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, IntType, "e", TopScopes, DefaultIntVal);

        // default UInt value
        ASTValueExpr *DefaultUintVal = getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(UIntType->getType()));
        ASTVar *fVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, UIntType, "f", TopScopes, DefaultUintVal);

        // default Long value
        ASTValueExpr *DefaultLongVal = getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(LongType->getType()));
        ASTVar *gVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, LongType, "g", TopScopes, DefaultLongVal);

        // default ULong value
        ASTValueExpr *DefaultULongVal = getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(ULongType->getType()));
        ASTVar *hVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, ULongType, "h", TopScopes, DefaultULongVal);

        // default Float value
        ASTValueExpr *DefaultFloatVal = getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(FloatType->getType()));
        ASTVar *iVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, FloatType, "i", TopScopes, DefaultFloatVal);

        // default Double value
        ASTValueExpr *DefaultDoubleVal = getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(DoubleType->getType()));
        ASTVar *jVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, DoubleType, "j", TopScopes, DefaultDoubleVal);

        // default Array value
        ASTValueExpr *DefaultArrayVal = getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(ArrayInt0Type->getType()));
        ASTVar *kVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, ArrayInt0Type, "k", TopScopes, DefaultArrayVal);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        EXPECT_FALSE(Diags.hasErrorOccurred());
        std::string output;

        // a
        GlobalVariable *aGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(aVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        aGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@a = global i1 false");

        // b
        GlobalVariable *bGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(bVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        bGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@b = global i8 0");

        // c
        GlobalVariable *cGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(cVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        cGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@c = global i16 0");

        // d
        GlobalVariable *dGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(dVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        dGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@d = global i16 0");

        // e
        GlobalVariable *eGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(eVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        eGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@e = global i32 0");

        // f
        GlobalVariable *fGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(fVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        fGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@f = global i32 0");

        // g
        GlobalVariable *gGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(gVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        gGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@g = global i64 0");

        // h
        GlobalVariable *hGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(hVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        hGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@h = global i64 0");

        // i
        GlobalVariable *iGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(iVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        iGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@i = global float 0.000000e+00");

        // l
        GlobalVariable *jGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(jVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        jGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@j = global double 0.000000e+00");

        GlobalVariable *kGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(kVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        kGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@k = global [0 x i32] zeroinitializer");
    }

    TEST_F(CodeGenTest, CGValuedGlobalVar) {
        SymModule *Module = CreateModule();

        // a
        ASTBoolValue *BoolVal = getASTBuilder().CreateBoolValue(SourceLoc, true);
        ASTVar *aVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, BoolType, "a", TopScopes, getASTBuilder().CreateExpr(BoolVal));

        // b
        ASTIntegerValue *ByteVal = getASTBuilder().CreateIntegerValue(SourceLoc, "1");
        ASTVar *bVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, ByteType, "b", TopScopes, getASTBuilder().CreateExpr(ByteVal));

        // c
        ASTIntegerValue *ShortVal = getASTBuilder().CreateIntegerValue(SourceLoc, "-2");
        ASTVar *cVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, ShortType, "c", TopScopes, getASTBuilder().CreateExpr(ShortVal));

        // d
        ASTIntegerValue *UShortVal = getASTBuilder().CreateIntegerValue(SourceLoc, "2");
        ASTVar *dVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, UShortType, "d", TopScopes, getASTBuilder().CreateExpr(UShortVal));

        // e
        ASTIntegerValue *IntVal = getASTBuilder().CreateIntegerValue(SourceLoc, "-3");
        ASTVar *eVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, IntType, "e", TopScopes, getASTBuilder().CreateExpr(IntVal));

        // f
        ASTIntegerValue *UIntVal = getASTBuilder().CreateIntegerValue(SourceLoc, "3");
        ASTVar *fVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, UIntType, "f", TopScopes, getASTBuilder().CreateExpr(UIntVal));

        // g
        ASTIntegerValue *LongVal = getASTBuilder().CreateIntegerValue(SourceLoc, "-4");
        ASTVar *gVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, LongType, "g", TopScopes, getASTBuilder().CreateExpr(LongVal));

        // h
        ASTIntegerValue *ULongVal = getASTBuilder().CreateIntegerValue(SourceLoc, "4");
        ASTVar *hVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, ULongType, "h", TopScopes, getASTBuilder().CreateExpr(ULongVal));

        // i
        ASTFloatingValue *FloatVal = getASTBuilder().CreateFloatingValue(SourceLoc, "1.5");
        ASTVar *iVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, FloatType, "i", TopScopes, getASTBuilder().CreateExpr(FloatVal));

        // j
        ASTFloatingValue *DoubleVal = getASTBuilder().CreateFloatingValue(SourceLoc, "2.5");
        ASTVar *jVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, DoubleType, "j", TopScopes, getASTBuilder().CreateExpr(DoubleVal));

        // k (empty array)
        llvm::SmallVector<ASTValue *, 8> Empty;
        ASTArrayValue *ArrayValEmpty = getASTBuilder().CreateArrayValue(SourceLoc, Empty);
        ASTVar *kVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, ArrayInt0Type, "k", TopScopes, getASTBuilder().CreateExpr(ArrayValEmpty));

        // l (array with 2 val)
        llvm::SmallVector<ASTValue *, 8> Values;
        Values.push_back(getASTBuilder().CreateIntegerValue(SourceLoc, "1")); // ArrayVal = {1}
        Values.push_back(getASTBuilder().CreateIntegerValue(SourceLoc, "2")); // ArrayVal = {1, 1}
        ASTArrayValue *ArrayVal = getASTBuilder().CreateArrayValue(SourceLoc, Values);
        ASTValueExpr *SizeExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "2"));
        ASTArrayTypeRef *ArrayInt2Type = getASTBuilder().CreateArrayTypeRef(SourceLoc, IntType, SizeExpr);
        ASTVar *lVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, ArrayInt2Type, "l", TopScopes, getASTBuilder().CreateExpr(ArrayVal));

        // m (string)
        ASTTypeRef *StringType = getASTBuilder().CreateStringTypeRef(SourceLoc);
        ASTStringValue *StringVal = getASTBuilder().CreateStringValue(SourceLoc, "hello");
        ASTVar *mVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, StringType, "m", TopScopes, getASTBuilder().CreateExpr(StringVal));

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        EXPECT_FALSE(Diags.hasErrorOccurred());
        std::string output;

        // a
        GlobalVariable *aGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(aVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        aGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@a = global i1 true");

        // b
        GlobalVariable *bGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(bVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        bGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@b = global i8 1");

        // c
        GlobalVariable *cGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(cVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        cGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@c = global i16 -2");

        // d
        GlobalVariable *dGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(dVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        dGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@d = global i16 2");

        // e
        GlobalVariable *eGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(eVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        eGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@e = global i32 -3");

        // f
        GlobalVariable *fGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(fVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        fGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@f = global i32 3");

        // g
        GlobalVariable *gGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(gVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        gGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@g = global i64 -4");

        // h
        GlobalVariable *hGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(hVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        hGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@h = global i64 4");

        // i
        GlobalVariable *iGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(iVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        iGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@i = global float 1.500000e+00");

        // j
        GlobalVariable *jGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(jVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        jGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@j = global double 2.500000e+00");

        GlobalVariable *kGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(kVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        kGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@k = global [0 x i32] zeroinitializer");

        GlobalVariable *lGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(lVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        lGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@l = global [2 x i32] [i32 1, i32 2]");

        GlobalVariable *mGVar = (GlobalVariable *) CGM->GenGlobalVar(static_cast<SymGlobalVar *>(mVar->getSym()))->getPointer();
        testing::internal::CaptureStdout();
        mGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@m = global [6 x i8] c\"hello\\00\"");
    }

    TEST_F(CodeGenTest, CGFuncParamTypes) {
        SymModule *Module = CreateModule();

        llvm::SmallVector<ASTVar *, 8> Params;
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, IntType, "P1", EmptyScopes));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, FloatType, "P2", EmptyScopes));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, BoolType, "P3", EmptyScopes));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, LongType, "P4", EmptyScopes));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, DoubleType, "P5", EmptyScopes));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, ByteType, "P6", EmptyScopes));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, ShortType, "P7", EmptyScopes));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, UShortType, "P8", EmptyScopes));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, UIntType, "P9", EmptyScopes));
        Params.push_back(getASTBuilder().CreateParam(SourceLoc, ULongType, "P10", EmptyScopes));
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);

        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);
        // func(int P1, float P2, bool P3, long P4, double P5, byte P6, short P7, ushort P8, uint P9, ulong P10) {
        // }
        
        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // float G = 2.0
        ASTFloatingValue *FloatingVal = getASTBuilder().CreateFloatingValue(SourceLoc, "2.0");
        ASTVar *GVar = getASTBuilder().CreateGlobalVar(Module->getAST(), SourceLoc, FloatType, "G", TopScopes,
                                                     getASTBuilder().CreateExpr(FloatingVal));

        // func()
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, IntType, "func", TopScopes, Params, Body);

        // int A = 1
        ASTVar *VarA = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntType, "A", EmptyScopes);
        SemaBuilderStmt *VarAStmt = getASTBuilder().CreateAssignmentStmt(Body, VarA);
        ASTExpr *ExprA = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "1"));
        VarAStmt->setExpr(ExprA);

        // GlobalVar
        // G = 1
        ASTVarRef *VarRefG = CreateVarRef(GVar);
        SemaBuilderStmt * GVarStmt = getASTBuilder().CreateAssignmentStmt(Body, VarRefG);
        ASTExpr *ExprG = getASTBuilder().CreateExpr(getASTBuilder().CreateFloatingValue(SourceLoc, "1"));
        GVarStmt->setExpr(ExprG);

        // return A
        SemaBuilderStmt *Return = getASTBuilder().CreateReturnStmt(Body, SourceLoc);
        ASTExpr *ExprRA = getASTBuilder().CreateExpr(CreateVarRef(VarA));
        Return->setExpr(ExprRA);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        CodeGenGlobalVar *CGGV = CGM->GenGlobalVar(static_cast<SymGlobalVar *>(GVar->getSym()));
        Value *G = CGGV->getPointer();

        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);
        
        // int a = 1
        ASTVar *LocalVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntType, "a", EmptyScopes);
        SemaBuilderStmt *VarStmt = getASTBuilder().CreateAssignmentStmt(Body, LocalVar);
        ASTValueExpr *ValueExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "1"));
        VarStmt->setExpr(ValueExpr);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // test()
        ASTBlockStmt *BodyTest = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Test = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, IntType, "test", TopScopes, Params, BodyTest);

        // func()
        ASTBlockStmt *BodyFunc = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, IntType, "func", TopScopes, Params, BodyFunc);

        // call test()
        SemaBuilderStmt *ExprStmt = getASTBuilder().CreateExprStmt(BodyFunc, SourceLoc);
        ASTCall *TestCall = CreateCall(Test, Args, ASTCallKind::CALL_FUNCTION);
        ASTCallExpr *Expr = getASTBuilder().CreateExpr(TestCall);
        ExprStmt->setExpr(Expr);

        //return test()
        SemaBuilderStmt *Return = getASTBuilder().CreateReturnStmt(BodyFunc, SourceLoc);
        ASTCallExpr *ReturnExpr = getASTBuilder().CreateExpr(TestCall);
        Return->setExpr(ReturnExpr);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        CodeGenFunction *CGF_Test = CGM->GenFunction(static_cast<SymFunction *>(Test->getSym()));
        CGF_Test->GenBody();
        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // func()
        llvm::SmallVector<ASTVar *, 8> Params;
        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTVar *bParam = getASTBuilder().CreateParam(SourceLoc, IntType, "b", EmptyScopes);
        Params.push_back(bParam);
        ASTVar *cParam = getASTBuilder().CreateParam(SourceLoc, IntType, "c", EmptyScopes);
        Params.push_back(cParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, IntType, "func", TopScopes, Params, Body);

        SemaBuilderStmt *Return = getASTBuilder().CreateReturnStmt(Body, SourceLoc);
        // Create this expression: 1 + a * b / (c - 2)
        // E1 + (E2 * E3) / (E4 - E5)
        // E1 + (G2 / G3)
        // E1 + G1
        ASTValueExpr *E1 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "1"));
        ASTVarRefExpr *E2 = getASTBuilder().CreateExpr(CreateVarRef(aParam));
        ASTVarRefExpr *E3 = getASTBuilder().CreateExpr(CreateVarRef(bParam));
        ASTVarRefExpr *E4 = getASTBuilder().CreateExpr(CreateVarRef(cParam));
        ASTValueExpr *E5 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "2"));

        ASTBinaryOpExpr *G2 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_MUL, E2, E3);
        ASTBinaryOpExpr *G3 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_SUB, E4, E5);
        ASTBinaryOpExpr *G1 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_DIV, G2, G3);
        ASTBinaryOpExpr *Group = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_ADD, E1, G1);

        Return->setExpr(Group);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // func()
        llvm::SmallVector<ASTVar *, 8> Params;
        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTVar *bParam = getASTBuilder().CreateParam(SourceLoc, IntType, "b", EmptyScopes);
        Params.push_back(bParam);
        ASTVar *cParam = getASTBuilder().CreateParam(SourceLoc, IntType, "c", EmptyScopes);
        Params.push_back(cParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // a = 0
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(aParam));
        ASTExpr *AssignExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "0"));
        aVarStmt->setExpr(AssignExpr);

        // b = 0
        SemaBuilderStmt *bVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(bParam));
        AssignExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "0"));
        bVarStmt->setExpr(AssignExpr);

        // c = a + b
        SemaBuilderStmt * cAddVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_ADD,
                getASTBuilder().CreateExpr(CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(CreateVarRef(bParam)));
        cAddVarStmt->setExpr(AssignExpr);

        // c = a - b
        SemaBuilderStmt * cSubVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_SUB,
                getASTBuilder().CreateExpr(CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(CreateVarRef(bParam)));
        cSubVarStmt->setExpr(AssignExpr);

        // c = a * b
        SemaBuilderStmt * cMulVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_MUL,
                getASTBuilder().CreateExpr(CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(CreateVarRef(bParam)));
        cMulVarStmt->setExpr(AssignExpr);

        // c = a / b
        SemaBuilderStmt * cDivVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_DIV,
                getASTBuilder().CreateExpr(CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(CreateVarRef(bParam)));
        cDivVarStmt->setExpr(AssignExpr);

        // c = a % b
        SemaBuilderStmt * cModVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_MOD,
                getASTBuilder().CreateExpr(CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(CreateVarRef(bParam)));
        cModVarStmt->setExpr(AssignExpr);

        // c = a & b
        SemaBuilderStmt * cAndVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_AND,
                getASTBuilder().CreateExpr(CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(CreateVarRef(bParam)));
        cAndVarStmt->setExpr(AssignExpr);

        // c = a | b
        SemaBuilderStmt * cOrVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_OR,
                getASTBuilder().CreateExpr(CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(CreateVarRef(bParam)));
        cOrVarStmt->setExpr(AssignExpr);

        // c = a xor b
        SemaBuilderStmt * cXorVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_XOR,
                getASTBuilder().CreateExpr(CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(CreateVarRef(bParam)));
        cXorVarStmt->setExpr(AssignExpr);

        // c = a << b
        SemaBuilderStmt * cShlVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_SHIFT_L,
                getASTBuilder().CreateExpr(CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(CreateVarRef(bParam)));
        cShlVarStmt->setExpr(AssignExpr);

        // c = a >> b
        SemaBuilderStmt * cShrVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cParam));
        AssignExpr = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_SHIFT_R,
                getASTBuilder().CreateExpr(CreateVarRef(aParam)),
                getASTBuilder().CreateExpr(CreateVarRef(bParam)));
        cShrVarStmt->setExpr(AssignExpr);

        // ++c
        SemaBuilderStmt *cPreIncVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        AssignExpr = getASTBuilder().CreateUnaryOpExpr(SourceLoc, ASTUnaryOpExprKind::OP_UNARY_PRE_INCR,
                getASTBuilder().CreateExpr(CreateVarRef(cParam)));
        cPreIncVarStmt->setExpr(AssignExpr);

        // c++
        SemaBuilderStmt *cPostIncVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        AssignExpr = getASTBuilder().CreateUnaryOpExpr(SourceLoc, ASTUnaryOpExprKind::OP_UNARY_POST_INCR,
                getASTBuilder().CreateExpr(CreateVarRef(cParam)));
        cPostIncVarStmt->setExpr(AssignExpr);

        // --c
        SemaBuilderStmt *cPreDecVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        AssignExpr = getASTBuilder().CreateUnaryOpExpr(SourceLoc, ASTUnaryOpExprKind::OP_UNARY_PRE_DECR,
                getASTBuilder().CreateExpr(CreateVarRef(cParam)));
        cPreDecVarStmt->setExpr(AssignExpr);

        // c--
        SemaBuilderStmt *cPostDecVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        AssignExpr = getASTBuilder().CreateUnaryOpExpr(SourceLoc, ASTUnaryOpExprKind::OP_UNARY_POST_DECR,
                getASTBuilder().CreateExpr(CreateVarRef(cParam)));
        cPostDecVarStmt->setExpr(AssignExpr);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);

        ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntType, "a", EmptyScopes);
        ASTVar *bVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntType, "b", EmptyScopes);
        ASTVar *cVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolType, "c", EmptyScopes);

        // a = 0
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(aVar));
        ASTExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "0"));
        aVarStmt->setExpr(Expr1);

        // b = 0
        SemaBuilderStmt *bVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(bVar));
        ASTExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "0"));
        bVarStmt->setExpr(Expr2);

        // c = a == b
        SemaBuilderStmt * cEqVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr3 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                getASTBuilder().CreateExpr(CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(CreateVarRef(bVar)));
        cEqVarStmt->setExpr(Expr3);

        // c = a != b
        SemaBuilderStmt * cNeqVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr4 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_NE,
                getASTBuilder().CreateExpr(CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(CreateVarRef(bVar)));
        cNeqVarStmt->setExpr(Expr4);

        // c = a > b
        SemaBuilderStmt * cGtVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr5 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_GT,
                getASTBuilder().CreateExpr(CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(CreateVarRef(bVar)));
        cGtVarStmt->setExpr(Expr5);

        // c = a >= b
        SemaBuilderStmt * cGteVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr6 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_GTE,
                getASTBuilder().CreateExpr(CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(CreateVarRef(bVar)));
        cGteVarStmt->setExpr(Expr6);

        // c = a < b
        SemaBuilderStmt * cLtVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr7 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_LT,
                getASTBuilder().CreateExpr(CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(CreateVarRef(bVar)));
        cLtVarStmt->setExpr(Expr7);

        // c = a <= b
        SemaBuilderStmt * cLteVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr8 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_LTE,
                getASTBuilder().CreateExpr(CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(CreateVarRef(bVar)));
        cLteVarStmt->setExpr(Expr8);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());

        // func()
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);

        ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolType, "a", EmptyScopes);
        ASTVar *bVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolType, "b", EmptyScopes);
        ASTVar *cVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolType, "c", EmptyScopes);

        // a = false
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(aVar));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateBoolValue(SourceLoc, false));
        aVarStmt->setExpr(Expr1);

        // b = false
        SemaBuilderStmt *bVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(bVar));
        ASTValueExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateBoolValue(SourceLoc, false));
        bVarStmt->setExpr(Expr2);

        // c = a and b
        SemaBuilderStmt * cAndVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr3 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_LOGIC_AND,
                getASTBuilder().CreateExpr(CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(CreateVarRef(bVar)));
        cAndVarStmt->setExpr(Expr3);

        // c = a or b
        SemaBuilderStmt * cOrVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTExpr *Expr4 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_LOGIC_OR,
                getASTBuilder().CreateExpr(CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(CreateVarRef(bVar)));
        cOrVarStmt->setExpr(Expr4);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());

        // func()
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);

        ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolType, "a", EmptyScopes);
        ASTVar *bVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolType, "b", EmptyScopes);
        ASTVar *cVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, BoolType, "c", EmptyScopes);

        // a = false
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(aVar));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateBoolValue(SourceLoc, false));
        aVarStmt->setExpr(Expr1);

        // b = false
        SemaBuilderStmt *bVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(bVar));
        ASTValueExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateBoolValue(SourceLoc, false));
        bVarStmt->setExpr(Expr2);

        // c = a == b ? a : b
        SemaBuilderStmt * cVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(cVar));
        ASTBinaryOpExpr *Cond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                getASTBuilder().CreateExpr(CreateVarRef(aVar)),
                getASTBuilder().CreateExpr(CreateVarRef(bVar)));

        ASTTernaryOpExpr *TernaryExpr = getASTBuilder().CreateTernaryOpExpr(Cond, SourceLoc,
                                                                    getASTBuilder().CreateExpr(CreateVarRef(aVar)),
                                                                    SourceLoc,
                                                                    getASTBuilder().CreateExpr(CreateVarRef(bVar)));
        cVarStmt->setExpr(TernaryExpr);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // int a = 0
        ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntType, "a", EmptyScopes);
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(aVar));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "0"));
        aVarStmt->setExpr(Expr1);

        // if (a == 1)
        ASTVarRefExpr *aVarRef = getASTBuilder().CreateExpr(CreateVarRef(aVar));
        ASTValueExpr *Value1 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "1"));
        ASTBinaryOpExpr *IfCond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ, aVarRef, Value1);

        // Create/Add if block
        SemaBuilderIfStmt *IfBuilder = getASTBuilder().CreateIfBuilder(Body);
        ASTBlockStmt *IfBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);

        // { a = 2 }
        SemaBuilderStmt *a2VarStmt = getASTBuilder().CreateAssignmentStmt(IfBlock, CreateVarRef(aVar));
        ASTValueExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "2"));
        a2VarStmt->setExpr(Expr2);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // func()
        llvm::SmallVector<ASTVar *, 8> Params;
        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // if (a == 1)
        ASTValueExpr *Value1 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "1"));
        ASTVarRefExpr *aVarRef = getASTBuilder().CreateExpr(CreateVarRef(aParam));
        ASTBinaryOpExpr *IfCond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                aVarRef, Value1);

        // Create/Add if block
        SemaBuilderIfStmt *IfBuilder = getASTBuilder().CreateIfBuilder(Body);
        ASTBlockStmt *IfBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);

        // { a = 1 }
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(IfBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "1"));
        aVarStmt->setExpr(Expr1);

        // else {a = 2}
        ASTBlockStmt *ElseBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        IfBuilder->Else(SourceLoc, ElseBlock);
        SemaBuilderStmt *aVarStmt2 = getASTBuilder().CreateAssignmentStmt(ElseBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "2"));
        aVarStmt2->setExpr(Expr2);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // func()

        llvm::SmallVector<ASTVar *, 8> Params;
        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // if (a == 1)
        SemaBuilderIfStmt *IfBuilder = getASTBuilder().CreateIfBuilder(Body);
        ASTValueExpr *Value1 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "1"));
        ASTVarRefExpr *aVarRef = getASTBuilder().CreateExpr(CreateVarRef(aParam));
        ASTBinaryOpExpr *IfCond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                aVarRef, Value1);
        ASTBlockStmt *IfBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        IfBuilder->If(SourceLoc, IfCond, IfBlock);


        // { a = 11 }
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(IfBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "11"));
        aVarStmt->setExpr(Expr1);

        // elsif (a == 2)
        ASTBlockStmt *ElsifBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value2 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "2"));
        ASTBinaryOpExpr *ElsifCond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                aVarRef, Value2);
        IfBuilder->ElseIf(SourceLoc, ElsifCond, ElsifBlock);
        // { a = 22 }
        SemaBuilderStmt *aVarStmt2 = getASTBuilder().CreateAssignmentStmt(ElsifBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "22"));
        aVarStmt2->setExpr(Expr2);

        // elsif (a == 3) { a = 33 }
        ASTBlockStmt *ElsifBlock2 = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value3 = getASTBuilder().CreateExpr( getASTBuilder().CreateIntegerValue(SourceLoc, "3"));
        ASTBinaryOpExpr *ElsifCond2 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                aVarRef, Value3);
        IfBuilder->ElseIf(SourceLoc, ElsifCond2, ElsifBlock2);
        SemaBuilderStmt *aVarStmt3 = getASTBuilder().CreateAssignmentStmt(ElsifBlock2, CreateVarRef(aParam));
        ASTValueExpr *Expr3 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "33"));
        aVarStmt3->setExpr(Expr3);

        // else {a == 44}
        ASTBlockStmt *ElseBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        IfBuilder->Else(SourceLoc, ElseBlock);
        SemaBuilderStmt *aVarStmt4 = getASTBuilder().CreateAssignmentStmt(ElseBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr4 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "44"));
        aVarStmt4->setExpr(Expr4);

        // validate and resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // main()
        llvm::SmallVector<ASTVar *, 8> Params;
        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // if a == 1
        SemaBuilderIfStmt *IfBuilder = getASTBuilder().CreateIfBuilder(Body);
        ASTBlockStmt *IfBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value1 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "1"));
        ASTVarRefExpr *aVarRef = getASTBuilder().CreateExpr(CreateVarRef(aParam));
        ASTBinaryOpExpr *IfCond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ, aVarRef, Value1);

        // { a = 11 }
        IfBuilder->If(SourceLoc, IfCond, IfBlock);
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(IfBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "11"));
        aVarStmt->setExpr(Expr1);

        // elsif (a == 2)
        ASTBlockStmt *ElsifBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value2 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "2"));
        ASTBinaryOpExpr *ElsifCond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                aVarRef, Value2);
        IfBuilder->ElseIf(SourceLoc, ElsifCond, ElsifBlock);
        // { a = 22 }
        SemaBuilderStmt *aVarStmt2 = getASTBuilder().CreateAssignmentStmt(ElsifBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "22"));
        aVarStmt2->setExpr(Expr2);

        // elsif (a == 3) { a = 33 }
        ASTBlockStmt *ElsifBlock2 = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value3 = getASTBuilder().CreateExpr( getASTBuilder().CreateIntegerValue(SourceLoc, "3"));
        ASTBinaryOpExpr *ElsifCond2 = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ,
                aVarRef, Value3);
        IfBuilder->ElseIf(SourceLoc, ElsifCond2, ElsifBlock2);
        SemaBuilderStmt *aVarStmt3 = getASTBuilder().CreateAssignmentStmt(ElsifBlock2, CreateVarRef(aParam));
        ASTValueExpr *Expr3 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "33"));
        aVarStmt3->setExpr(Expr3);

        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // main()
        llvm::SmallVector<ASTVar *, 8> Params;
        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // switch a
        SemaBuilderSwitchStmt *SwitchBuilder = getASTBuilder().CreateSwitchBuilder(Body);
        ASTVarRefExpr *aVarRefExpr = getASTBuilder().CreateExpr(CreateVarRef(aParam));
        SwitchBuilder->Switch(SourceLoc, aVarRefExpr);

        // case 1: a = 1 break
        ASTBlockStmt *Case1Block = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTValueExpr *Case1Value = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "1"));
        SwitchBuilder->Case(SourceLoc, Case1Value, Case1Block);
        SemaBuilderStmt *aVarStmt1 = getASTBuilder().CreateAssignmentStmt(Case1Block, CreateVarRef(aParam));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "1"));
        aVarStmt1->setExpr(Expr1);

        // case 2: a = 2 break
        ASTBlockStmt *Case2Block = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTValueExpr *Case2Value = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "2"));
        SwitchBuilder->Case(SourceLoc, Case2Value, Case2Block);
        SemaBuilderStmt *aVarStmt2 = getASTBuilder().CreateAssignmentStmt(Case2Block, CreateVarRef(aParam));
        ASTValueExpr *Expr2 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "2"));
        aVarStmt2->setExpr(Expr2);

        // default: a = 3
        ASTBlockStmt *DefaultBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        SwitchBuilder->Default(SourceLoc, DefaultBlock);
        SemaBuilderStmt *aVarStmt3 = getASTBuilder().CreateAssignmentStmt(DefaultBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr3 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "3"));
        aVarStmt3->setExpr(Expr3);

        // Add switch
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // main()
        llvm::SmallVector<ASTVar *, 8> Params;

        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // while a == 1
        SemaBuilderLoopStmt *LoopBuilder = getASTBuilder().CreateLoopBuilder(Body, SourceLoc);
        ASTValueExpr *Value1 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "1"));
        ASTVarRefExpr *aVarRef = getASTBuilder().CreateExpr(CreateVarRef(aParam));
        ASTBinaryOpExpr *Cond = getASTBuilder().CreateBinaryOpExpr(SourceLoc, ASTBinaryOpExprKind::OP_BINARY_EQ, aVarRef, Value1);
        ASTBlockStmt *BlockStmt = getASTBuilder().CreateBlockStmt(SourceLoc);
        LoopBuilder->Loop(Cond, BlockStmt);

        // { a = 1 }
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(BlockStmt, CreateVarRef(aParam));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "1"));
        aVarStmt->setExpr(Expr1);

        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // main()
        llvm::SmallVector<ASTVar *, 8> Params;
        ASTVar *aParam = getASTBuilder().CreateParam(SourceLoc, IntType, "a", EmptyScopes);
        Params.push_back(aParam);
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // for int i = 1; i < 1; ++i
        SemaBuilderLoopStmt *LoopBuilder = getASTBuilder().CreateLoopBuilder(Body, SourceLoc);

        // Init
        // int i = 1
        ASTBlockStmt *InitBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        LoopBuilder->Init(InitBlock);
        ASTVar *iVar = getASTBuilder().CreateLocalVar(InitBlock, SourceLoc, IntType, "i", EmptyScopes);
        ASTVarRef *iVarRef = CreateVarRef(iVar);
        SemaBuilderStmt *iVarStmt = getASTBuilder().CreateAssignmentStmt(InitBlock, iVarRef);
        ASTValueExpr *Value1Expr = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "1"));
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
                getASTBuilder().CreateExpr(CreateVarRef(iVar)));
        SemaBuilderStmt *iVarIncStmt = getASTBuilder().CreateExprStmt(PostBlock, SourceLoc);
        iVarIncStmt->setExpr(IncExpr);

        // Loop Block
        // { a = 1 }
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(LoopBlock, CreateVarRef(aParam));
        ASTValueExpr *Expr1 = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "1"));
        aVarStmt->setExpr(Expr1);

        // Resolve
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
        CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // TestClass {
        //   int a() { return 1 }
        //   public int b() { return 1 }
        //   private const int c { return 1 }
        // }
        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
        ASTClass *TestClass = getASTBuilder().CreateClass(Module->getAST(), SourceLoc, ASTClassKind::CLASS, "TestClass",
                                                  TopScopes, SuperClasses);

        // int a() { return 1 }
        ASTBlockStmt *aFuncBody = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *aFunc = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, IntType,
                                                          "a", TopScopes, Params, aFuncBody);

        SemaBuilderStmt *aFuncReturn = getASTBuilder().CreateReturnStmt(aFuncBody, SourceLoc);
        ASTValueExpr *aFuncExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLocation(), "1"));
        aFuncReturn->setExpr(aFuncExpr);


        // public int b() { return 1 }
        ASTBlockStmt *bFuncBody = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *bFunc = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, IntType,
                                                          "b", TopScopes, Params, bFuncBody);
        SemaBuilderStmt *bFuncReturn = getASTBuilder().CreateReturnStmt(bFuncBody, SourceLoc);
        ASTValueExpr *bFuncExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLocation(), "1"));
        bFuncReturn->setExpr(bFuncExpr);

        // private const int c { return 1 }
        ASTBlockStmt *cFuncBody = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *cFunc = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, IntType,
                                                          "c", TopScopes, Params, cFuncBody);
        SemaBuilderStmt *cFuncReturn = getASTBuilder().CreateReturnStmt(cFuncBody, SourceLoc);
        ASTValueExpr *cFuncExpr = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLocation(), "1"));
        cFuncReturn->setExpr(cFuncExpr);

        // int main() {
        //  TestClass test = new TestClass()
        //  int a = test.a()
        //  int b = test.b()
        //  int c = test.c()
        //  delete test
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // TestClass test = new TestClass()
        ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestClass);
        ASTVar *TestVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyScopes);
        ASTCall *ConstructorCall = CreateCall(TestClass->getName(), Args, ASTCallKind::CALL_NEW);
        ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
        SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(Body, TestVar);
        testNewStmt->setExpr(NewExpr);

        // int a = test.a()
        ASTTypeRef *aType = aFunc->getReturnTypeRef();
        ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, aType, "a", EmptyScopes);
        ASTCallExpr *aCallExpr = getASTBuilder().CreateExpr(CreateCall(aFunc, Args, ASTCallKind::CALL_FUNCTION, CreateVarRef(TestVar)));
        SemaBuilderStmt *aStmt = getASTBuilder().CreateAssignmentStmt(Body, aVar);
        aStmt->setExpr(aCallExpr);

        // int b = test.b()
        ASTTypeRef *bType = bFunc->getReturnTypeRef();
        ASTVar *bVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, bType, "b", EmptyScopes);
        ASTCallExpr *bCallExpr = getASTBuilder().CreateExpr(CreateCall(bFunc, Args, ASTCallKind::CALL_FUNCTION, CreateVarRef(TestVar)));
        SemaBuilderStmt *bStmt = getASTBuilder().CreateAssignmentStmt(Body, bVar);
        bStmt->setExpr(bCallExpr);

        // int c = test.c()
        ASTTypeRef *cType = cFunc->getReturnTypeRef();
        ASTVar *cVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, cType, "c", EmptyScopes);
        ASTCallExpr *cCallExpr = getASTBuilder().CreateExpr(CreateCall(cFunc, Args, ASTCallKind::CALL_FUNCTION, CreateVarRef(TestVar)));
        SemaBuilderStmt *cStmt = getASTBuilder().CreateAssignmentStmt(Body, cVar);
        cStmt->setExpr(cCallExpr);

        // delete test
        ASTDeleteStmt *DeleteStmt = getASTBuilder().CreateDeleteStmt(Body, SourceLoc, CreateVarRef(TestVar));

        bool Success = S->Resolve();
        EXPECT_TRUE(Success);

        if (Success) {

            CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
            // Generate Code
            CodeGenClass *CGC = CGM->GenClass(Module->getClasses().lookup(TestClass->getName()));
            for (auto &F : CGC->getConstructors()) {
                F->GenBody();
            }
            for (auto &F : CGC->getFunctions()) {
                F->GenBody();
            }
            CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // TestClass {
        //   int a
        //   int getA() { return a }
        // }
        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
        ASTClass *TestClass = getASTBuilder().CreateClass(Module->getAST(), SourceLoc, ASTClassKind::CLASS, "TestClass", TopScopes,
                                                  SuperClasses);

        // int a
        ASTVar *aAttribute = getASTBuilder().CreateClassAttribute(SourceLoc, TestClass, IntType, "a", TopScopes);

        // int getA() { return a }
        ASTBlockStmt *MethodBody = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *getAMethod = getASTBuilder().CreateClassMethod(SourceLoc, TestClass, IntType,
                                                               "getA", TopScopes, Params, MethodBody);

        SemaBuilderStmt *MethodReturn = getASTBuilder().CreateReturnStmt(MethodBody, SourceLoc);
        MethodReturn->setExpr(getASTBuilder().CreateExpr(CreateVarRef(aAttribute)));

        // int main() {
        //  TestClass test = new TestClass()
        //  int x = test.getA()
        //  test.a = 2
        //  delete test
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // TestClass test = new TestClass()
        ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestClass);
        ASTVar *TestVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyScopes);
        ASTCall *ConstructorCall = CreateCall(TestClass->getName(), Args, ASTCallKind::CALL_NEW);
        ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
        SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(Body, TestVar);
        testNewStmt->setExpr(NewExpr);

        // int x = test.m()
        ASTTypeRef *xType = getAMethod->getReturnTypeRef();
        ASTVar *xVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, xType, "x", EmptyScopes);
        ASTCallExpr *xCallExpr = getASTBuilder().CreateExpr(CreateCall(getAMethod, Args, ASTCallKind::CALL_FUNCTION, CreateVarRef(TestVar)));
        SemaBuilderStmt *xStmt = getASTBuilder().CreateAssignmentStmt(Body, xVar);
        xStmt->setExpr(xCallExpr);

        //  test.a = 2
        SemaBuilderStmt *attrStmt = getASTBuilder().CreateAssignmentStmt(Body, CreateVarRef(aAttribute, CreateVarRef(TestVar)));
        ASTValueExpr *value2Expr = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLocation(), "2"));
        attrStmt->setExpr(value2Expr);

        // delete test
        ASTDeleteStmt *DeleteStmt = getASTBuilder().CreateDeleteStmt(Body, SourceLoc, CreateVarRef(TestVar));

        bool Success = S->Resolve();
        EXPECT_TRUE(Success);

        if (Success) {

            CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());
            // Generate Code
            CodeGenClass *CGC = CGM->GenClass(Module->getClasses().lookup(TestClass->getName()));
            for (auto &F : CGC->getConstructors()) {
                F->GenBody();
            }
            for (auto &F : CGC->getFunctions()) {
                F->GenBody();
            }
            CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // struct TestStruct {
        //   int a
        //   int b
        //   const int c
        // }

        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
        ASTClass *TestStruct = getASTBuilder().CreateClass(Module->getAST(), SourceLoc, ASTClassKind::STRUCT, "TestStruct",
                                                   TopScopes, SuperClasses);
        ASTVar *aField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntType, "a", TopScopes);
        ASTVar *bField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntType, "b", TopScopes);
        ASTVar *cField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntType, "c", TopScopes);

        // int func() {
        //  TestStruct test = new TestStruct();
        //  int a = test.a
        //  test.b = 2
        //  return 1
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);

        // TestStruct test = new TestStruct()
        ASTTypeRef *TestClassType = getASTBuilder().CreateTypeRef(TestStruct);
        ASTVar *TestVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestClassType, "test", EmptyScopes);
        ASTCall *ConstructorCall = CreateCall(TestStruct->getName(), Args, ASTCallKind::CALL_NEW);
        ASTCallExpr *NewExpr = getASTBuilder().CreateExpr(ConstructorCall);
        SemaBuilderStmt *testNewStmt = getASTBuilder().CreateAssignmentStmt(Body, TestVar);
        testNewStmt->setExpr(NewExpr);

        // int a = test.a
        ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, IntType, "a", EmptyScopes);
        ASTVarRef *Instance = CreateVarRef(TestVar);
        ASTVarRef *test_aVarRef = CreateVarRef(aField,Instance);
        ASTVarRefExpr *test_aRefExpr = getASTBuilder().CreateExpr(test_aVarRef);
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(Body, aVar);
        aVarStmt->setExpr(test_aRefExpr);

        // test.b = 2
        ASTVarRef *Instance2 = CreateVarRef(TestVar);
        ASTVarRef *test_bVarRef = CreateVarRef(bField, Instance2);
        SemaBuilderStmt *test_bVarStmt = getASTBuilder().CreateAssignmentStmt(Body, test_bVarRef);
        ASTExpr *Expr = getASTBuilder().CreateExpr(getASTBuilder().CreateIntegerValue(SourceLoc, "2"));
        test_bVarStmt->setExpr(Expr);

        // delete test
        ASTDeleteStmt *Delete = getASTBuilder().CreateDeleteStmt(Body, SourceLoc, (ASTVarRef *) Instance);

        bool Success = S->Resolve();
        EXPECT_TRUE(Success);

        if (Success) {
            CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());

            // Generate Code
            CodeGenClass *CGC = CGM->GenClass(Module->getClasses().lookup(TestStruct->getName()));
            for (auto &F : CGC->getConstructors()) {
                F->GenBody();
            }
            for (auto &F : CGC->getFunctions()) {
                F->GenBody();
            }
            CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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
        SymModule *Module = CreateModule();

        // enum TestEnum {
        //   A
        //   B
        //   C
        // }
        llvm::SmallVector<ASTTypeRef *, 4> SuperEnums;
        ASTEnum *TestEnum = getASTBuilder().CreateEnum(Module->getAST(), SourceLoc, "TestEnum", TopScopes, SuperEnums);
        ASTVar *A = getASTBuilder().CreateEnumEntry(SourceLoc, TestEnum, "A", EmptyScopes);
        ASTVar *B = getASTBuilder().CreateEnumEntry(SourceLoc, TestEnum, "B", EmptyScopes);
        ASTVar *C = getASTBuilder().CreateEnumEntry(SourceLoc, TestEnum, "C", EmptyScopes);

        // int main() {
        //  TestEnum a = TestEnum.A;
        //  TestEnum b = a
        //  return 1
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);

        ASTTypeRef *TestEnumType = getASTBuilder().CreateTypeRef(TestEnum);

        //  TestEnum a = TestEnum.A;
        ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestEnumType, "a", EmptyScopes);
        ASTVarRef *Enum_AVarRef = CreateVarRef(A, getASTBuilder().CreateUndefinedRef(SourceLoc, TestEnumType->getName()));
        ASTVarRefExpr *Enum_ARefExpr = getASTBuilder().CreateExpr(Enum_AVarRef);
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(Body, aVar);
        aVarStmt->setExpr(Enum_ARefExpr);

        //  TestEnum b = a
        ASTVar *bVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestEnumType, "b", EmptyScopes);
        SemaBuilderStmt *bVarStmt = getASTBuilder().CreateAssignmentStmt(Body, bVar);
        ASTVarRefExpr *aRefExpr = getASTBuilder().CreateExpr(CreateVarRef(aVar));
        bVarStmt->setExpr(aRefExpr);

        bool Success = S->Resolve();
        EXPECT_TRUE(Success);

        if (Success) {
            CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());

            CGM->GenEnum(Module->getEnums().lookup(TestEnum->getName()));

            // Generate Code
            CodeGenFunction *CGF = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
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

	TEST_F(CodeGenTest, CGMain) {
        SymModule *Module = CreateModule();

        // main() {
        //
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "main", TopScopes, Params, Body);

        // Validate and Resolve
        EXPECT_TRUE(S->Resolve());

        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());

        CodeGenFunction *CGF_Main = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
        CGF_Main->GenBody();
        llvm::Function *F_Main = CGF_Main->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F_Main->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
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

	TEST_F(CodeGenTest, CGErrorHandler) {
        SymModule *Module = CreateModule();

        // func() {
        //   error A = handle fail
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "func", TopScopes, Params, Body);
        ASTVar *ErrorA = getASTBuilder().CreateLocalVar(Body, SourceLoc, ErrorType, "A", EmptyScopes);
        ASTVarRef *ErrorVarRef = getASTBuilder().CreateVarRef(ErrorA);
        ASTBlockStmt *HandleBlock = getASTBuilder().CreateBlockStmt(SourceLoc);
        getASTBuilder().CreateHandleStmt(Body, SourceLoc, HandleBlock, ErrorVarRef);

        SemaBuilderStmt *Fail0Stmt = getASTBuilder().CreateFailStmt(HandleBlock, SourceLoc);

        // Validate and Resolve
        EXPECT_TRUE(S->Resolve());

        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());

        CodeGenFunction *CGF_Main = CGM->GenFunction(static_cast<SymFunction *>(Func->getSym()));
        CGF_Main->GenBody();
        llvm::Function *F_Main = CGF_Main->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F_Main->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca %error*, align 8\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  br label %handle\n"
                          "\n"
                          "handle:                                           ; preds = %entry\n"
                          "  %3 = load %error*, %error** %2, align 8\n"
                          "  %4 = getelementptr inbounds %error, %error* %3, i32 0, i32 0\n"
                          "  store i8 1, i8* %4, align 1\n"
                          "  %5 = getelementptr inbounds %error, %error* %3, i32 0, i32 1\n"
                          "  store i32 1, i32* %5, align 4\n"
                          "  br label %safe\n"
                          "\n"
                          "safe:                                             ; preds = %handle\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGErrorFail) {
        SymModule *Module = CreateModule();

        // int testFail0() {
        //   fail
        // }
        ASTBlockStmt *Body0 = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail0 = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, IntType, "testFail0", TopScopes, Params, Body0);
        SemaBuilderStmt *Fail0Stmt = getASTBuilder().CreateFailStmt(Body0, SourceLoc);

        // int testFail1() {
        //   fail true
        // }
        ASTBlockStmt *Body1 = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail1 = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, IntType, "testFail1", TopScopes, Params, Body1);
        ASTBoolValue *BoolVal = getASTBuilder().CreateBoolValue(SourceLoc, true);
        SemaBuilderStmt *Fail1Stmt = getASTBuilder().CreateFailStmt(Body1, SourceLoc);
        Fail1Stmt->setExpr(getASTBuilder().CreateExpr(BoolVal));

        // int testFail2() {
        //   fail 10
        // }
        ASTBlockStmt *Body2 = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail2 = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, IntType, "testFail2", TopScopes, Params, Body2);
        ASTIntegerValue *IntVal = getASTBuilder().CreateIntegerValue(SourceLoc, "10");
        SemaBuilderStmt *Fail2Stmt = getASTBuilder().CreateFailStmt(Body2, SourceLoc);
        Fail2Stmt->setExpr(getASTBuilder().CreateExpr(IntVal));

        // int testFail3() {
        //  fail "Error"
        // }
        ASTBlockStmt *Body3 = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail3 = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, IntType, "testFail3", TopScopes, Params, Body3);
        ASTStringValue *StrVal = getASTBuilder().CreateStringValue(SourceLoc, "Error");
        SemaBuilderStmt *Fail3Stmt = getASTBuilder().CreateFailStmt(Body3, SourceLoc);
        Fail3Stmt->setExpr(getASTBuilder().CreateExpr(StrVal));

        // int testFail4() {
        //  fail new TestStruct()
        // }
        llvm::SmallVector<ASTTypeRef *, 4> SuperClasses;
        ASTClass *TestStruct = getASTBuilder().CreateClass(Module->getAST(), SourceLoc, ASTClassKind::STRUCT, "TestStruct", TopScopes, SuperClasses);
        ASTVar *aField = getASTBuilder().CreateClassAttribute(SourceLoc, TestStruct, IntType, "a", TopScopes);

        ASTBlockStmt *Body4 = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *TestFail4 = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, IntType, "testFail4", TopScopes, Params, Body4);
        // TestStruct test = new TestStruct()
        ASTCall *ConstructorCall = CreateCall(TestStruct->getName(), Args, ASTCallKind::CALL_NEW);
        // fail new TestStruct()
        SemaBuilderStmt *Fail4Stmt = getASTBuilder().CreateFailStmt(Body4, SourceLoc);
        Fail4Stmt->setExpr(getASTBuilder().CreateExpr(ConstructorCall));

        // main() {
        //   testFail0()
        //   testFail1()
        //   testFail2()
        //   testFail3()
        //   testFail4()
        // }
        ASTBlockStmt *MainBody = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Main = getASTBuilder().CreateFunction(Module->getAST(), SourceLoc, VoidType, "main", TopScopes, Params, MainBody);

        // call testFail0()
        SemaBuilderStmt *CallTestFail0 = getASTBuilder().CreateExprStmt(MainBody, SourceLoc);
        ASTCallExpr *CallExpr0 = getASTBuilder().CreateExpr(CreateCall(TestFail0, Args, ASTCallKind::CALL_FUNCTION));
        CallTestFail0->setExpr(CallExpr0);

        // call testFail1()
        SemaBuilderStmt *CallTestFail1 = getASTBuilder().CreateExprStmt(MainBody, SourceLoc);
        ASTCallExpr *CallExpr1 = getASTBuilder().CreateExpr(CreateCall(TestFail1, Args, ASTCallKind::CALL_FUNCTION));
        CallTestFail1->setExpr(CallExpr1);

        // call testFail2()
        SemaBuilderStmt *CallTestFail2 = getASTBuilder().CreateExprStmt(MainBody, SourceLoc);
        ASTCallExpr *CallExpr2 = getASTBuilder().CreateExpr(CreateCall(TestFail2, Args, ASTCallKind::CALL_FUNCTION));
        CallTestFail2->setExpr(CallExpr2);

        // call testFail3()
        SemaBuilderStmt *CallTestFail3 = getASTBuilder().CreateExprStmt(MainBody, SourceLoc);
        ASTCallExpr *CallExpr3 = getASTBuilder().CreateExpr(CreateCall(TestFail3, Args, ASTCallKind::CALL_FUNCTION));
        CallTestFail3->setExpr(CallExpr3);

        // call testFail4()
        SemaBuilderStmt *CallTestFail4 = getASTBuilder().CreateExprStmt(MainBody, SourceLoc);
        ASTCallExpr *CallExpr4 = getASTBuilder().CreateExpr(CreateCall(TestFail4, Args, ASTCallKind::CALL_FUNCTION));
        CallTestFail4->setExpr(CallExpr4);

        // Validate and Resolve
        EXPECT_TRUE(S->Resolve());

        CodeGenModule *CGM = CG->GenerateModule(Module->getNameSpace());

        // Generate Code
        CodeGenClass *CGC = CGM->GenClass(Module->getClasses().lookup(TestStruct->getName()));
        for (auto &F : CGC->getConstructors()) {
            F->GenBody();
        }

        // Generate Code
        CodeGenFunction *CGF_TestFail0 = CGM->GenFunction(static_cast<SymFunction *>(TestFail0->getSym()));
        CGF_TestFail0->GenBody();
        llvm::Function *F_TestFail0 = CGF_TestFail0->getFunction();

        CodeGenFunction *CGF_TestFail1 = CGM->GenFunction(static_cast<SymFunction *>(TestFail1->getSym()));
        CGF_TestFail1->GenBody();
        llvm::Function *F_TestFail1 = CGF_TestFail1->getFunction();

        CodeGenFunction *CGF_TestFail2 = CGM->GenFunction(static_cast<SymFunction *>(TestFail2->getSym()));
        CGF_TestFail2->GenBody();
        llvm::Function *F_TestFail2 = CGF_TestFail2->getFunction();

        CodeGenFunction *CGF_TestFail3 = CGM->GenFunction(static_cast<SymFunction *>(TestFail3->getSym()));
        CGF_TestFail3->GenBody();
        llvm::Function *F_TestFail3 = CGF_TestFail3->getFunction();

        CodeGenFunction *CGF_TestFail4 = CGM->GenFunction(static_cast<SymFunction *>(TestFail4->getSym()));
        CGF_TestFail4->GenBody();
        llvm::Function *F_TestFail4 = CGF_TestFail4->getFunction();

        CodeGenFunction *CGF_Main = CGM->GenFunction(static_cast<SymFunction *>(Main->getSym()));
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
                          "  %4 = getelementptr inbounds %error, %error* %2, i32 0, i32 1\n"
                          "  store i32 1, i32* %4, align 4\n"
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
                          "  store i32 10, i32* %4, align 4\n"
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
} // anonymous namespace
