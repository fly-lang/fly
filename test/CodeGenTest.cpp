//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "TestUtils.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenGlobalVar.h"
#include "CodeGen/CodeGenFunction.h"
#include "Sema/SemaBuilder.h"
#include "AST/ASTNode.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTVar.h"
#include "AST/ASTVarAssign.h"
#include "AST/ASTValue.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTParams.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTForBlock.h"
#include "Basic/Diagnostic.h"
#include "Basic/DiagnosticOptions.h"
#include "Basic/SourceLocation.h"
#include "Basic/TargetOptions.h"
#include "Basic/Builtins.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "gtest/gtest.h"
#include <fstream>
#include <iostream>
#include <vector>


namespace {
    using namespace fly;

    // The test fixture.
    class CodeGenTest : public ::testing::Test {

    public:
        const CompilerInstance CI;
        CodeGen *CG;
        CodeGenModule *CGM;
        DiagnosticsEngine &Diags;
        SourceLocation SourceLoc;
        SemaBuilder *Builder;

        ASTType *BoolType;
        ASTType *ByteType;
        ASTType *ShortType;
        ASTType *UShortType;
        ASTType *IntType;
        ASTType *UIntType;
        ASTType *LongType;
        ASTType *ULongType;
        ASTType *FloatType;
        ASTType *DoubleType;
        ASTType *ArrayInt0Type;

        CodeGenTest() : CI(*TestUtils::CreateCompilerInstance()),
                CG(TestUtils::CreateCodeGen(CI)),
                CGM(CG->CreateModule("CodeGenTest")),
                Diags(CI.getDiagnostics()),
                Builder(Sema::Builder(CI.getDiagnostics())),
                BoolType(SemaBuilder::CreateBoolType(SourceLoc)),
                ByteType(SemaBuilder::CreateByteType(SourceLoc)),
                ShortType(SemaBuilder::CreateShortType(SourceLoc)),
                UShortType(SemaBuilder::CreateUShortType(SourceLoc)),
                IntType(SemaBuilder::CreateIntType(SourceLoc)),
                UIntType(SemaBuilder::CreateUIntType(SourceLoc)),
                LongType(SemaBuilder::CreateLongType(SourceLoc)),
                ULongType(SemaBuilder::CreateULongType(SourceLoc)),
                FloatType(SemaBuilder::CreateFloatType(SourceLoc)),
                DoubleType(SemaBuilder::CreateDoubleType(SourceLoc)),
                ArrayInt0Type(SemaBuilder::CreateArrayType(SourceLoc, IntType,
                                                       Builder->CreateExpr(nullptr, SemaBuilder::CreateIntegerValue(SourceLoc, 0))))
                {
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmPrinters();
        }

        ASTNode *CreateNode() {
            Diags.getClient()->BeginSourceFile();
            const std::string Name = "CodeGenTest";
            std::string NameSpace = "default";
            auto *Node = Builder->CreateNode(Name, NameSpace);
            Diags.getClient()->EndSourceFile();
            return Node;
        }

        virtual ~CodeGenTest() {
            llvm::outs().flush();
        }
    };

    TEST_F(CodeGenTest, CGDefaultValueGlobalVar) {
        ASTNode *Node = CreateNode();

        // default Bool value
        ASTValue *DefaultBoolVal = SemaBuilder::CreateDefaultValue(BoolType);
        ASTGlobalVar *aVar = Builder->CreateGlobalVar(Node, SourceLoc, BoolType, "a", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, aVar, DefaultBoolVal));

        // default Byte value
        ASTValue *DefaultByteVal = SemaBuilder::CreateDefaultValue(ByteType);
        ASTGlobalVar *bVar = Builder->CreateGlobalVar(Node, SourceLoc, ByteType, "b", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, bVar, DefaultByteVal));

        // default Short value
        ASTValue *DefaultShortVal = SemaBuilder::CreateDefaultValue(ShortType);
        ASTGlobalVar *cVar = Builder->CreateGlobalVar(Node, SourceLoc, ShortType, "c", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, cVar, DefaultShortVal));

        // default UShort value
        ASTValue *DefaultUShortVal = SemaBuilder::CreateDefaultValue(UShortType);
        ASTGlobalVar *dVar = Builder->CreateGlobalVar(Node, SourceLoc, UShortType, "d", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, dVar, DefaultUShortVal));

        // default Int value
        ASTValue *DefaultIntVal = SemaBuilder::CreateDefaultValue(IntType);
        ASTGlobalVar *eVar = Builder->CreateGlobalVar(Node, SourceLoc, IntType, "e", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, eVar, DefaultIntVal));

        // default UInt value
        ASTValue *DefaultUintVal = SemaBuilder::CreateDefaultValue(UIntType);
        ASTGlobalVar *fVar = Builder->CreateGlobalVar(Node, SourceLoc, UIntType, "f", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, fVar, DefaultUintVal));

        // default Long value
        ASTValue *DefaultLongVal = SemaBuilder::CreateDefaultValue(LongType);
        ASTGlobalVar *gVar = Builder->CreateGlobalVar(Node, SourceLoc, LongType, "g", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, gVar, DefaultLongVal));

        // default ULong value
        ASTValue *DefaultULongVal = SemaBuilder::CreateDefaultValue(ULongType);
        ASTGlobalVar *hVar = Builder->CreateGlobalVar(Node, SourceLoc, ULongType, "h", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, hVar, DefaultULongVal));

        // default Float value
        ASTValue *DefaultFloatVal = SemaBuilder::CreateDefaultValue(FloatType);
        ASTGlobalVar *iVar = Builder->CreateGlobalVar(Node, SourceLoc, FloatType, "i", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, iVar, DefaultFloatVal));

        // default Double value
        ASTValue *DefaultDoubleVal = SemaBuilder::CreateDefaultValue(DoubleType);
        ASTGlobalVar *jVar = Builder->CreateGlobalVar(Node, SourceLoc, DoubleType, "j", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node,jVar, DefaultDoubleVal));

        // default Array value
        ASTValue *DefaultArrayVal = SemaBuilder::CreateDefaultValue(ArrayInt0Type);
        ASTGlobalVar *kVar = Builder->CreateGlobalVar(Node, SourceLoc, ArrayInt0Type, "k", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, kVar, DefaultArrayVal));

        // Generate Code
        EXPECT_FALSE(Diags.hasErrorOccurred());
        std::string output;

        // a
        GlobalVariable *aGVar = (GlobalVariable *)CGM->GenGlobalVar(aVar)->getPointer();
        testing::internal::CaptureStdout();
        aGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@a = global i1 false");

        // b
        GlobalVariable *bGVar = (GlobalVariable *)CGM->GenGlobalVar(bVar)->getPointer();
        testing::internal::CaptureStdout();
        bGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@b = global i8 0");

        // c
        GlobalVariable *cGVar = (GlobalVariable *)CGM->GenGlobalVar(cVar)->getPointer();
        testing::internal::CaptureStdout();
        cGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@c = global i16 0");

        // d
        GlobalVariable *dGVar = (GlobalVariable *)CGM->GenGlobalVar(dVar)->getPointer();
        testing::internal::CaptureStdout();
        dGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@d = global i16 0");

        // e
        GlobalVariable *eGVar = (GlobalVariable *)CGM->GenGlobalVar(eVar)->getPointer();
        testing::internal::CaptureStdout();
        eGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@e = global i32 0");

        // f
        GlobalVariable *fGVar = (GlobalVariable *)CGM->GenGlobalVar(fVar)->getPointer();
        testing::internal::CaptureStdout();
        fGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@f = global i32 0");

        // g
        GlobalVariable *gGVar = (GlobalVariable *)CGM->GenGlobalVar(gVar)->getPointer();
        testing::internal::CaptureStdout();
        gGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@g = global i64 0");

        // h
        GlobalVariable *hGVar = (GlobalVariable *)CGM->GenGlobalVar(hVar)->getPointer();
        testing::internal::CaptureStdout();
        hGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@h = global i64 0");

        // i
        GlobalVariable *iGVar = (GlobalVariable *)CGM->GenGlobalVar(iVar)->getPointer();
        testing::internal::CaptureStdout();
        iGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@i = global float 0.000000e+00");

        // l
        GlobalVariable *jGVar = (GlobalVariable *)CGM->GenGlobalVar(jVar)->getPointer();
        testing::internal::CaptureStdout();
        jGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@j = global double 0.000000e+00");

        GlobalVariable *kGVar = (GlobalVariable *)CGM->GenGlobalVar(kVar)->getPointer();
        testing::internal::CaptureStdout();
        kGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@k = global [0 x i32] zeroinitializer");
    }

    TEST_F(CodeGenTest, CGValuedGlobalVar) {
        ASTNode *Node = CreateNode();

        // a
        ASTBoolValue *BoolVal = SemaBuilder::CreateBoolValue(SourceLoc, true);
        ASTGlobalVar *aVar = Builder->CreateGlobalVar(Node, SourceLoc, BoolType, "a", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, aVar, BoolVal));

        // b
        ASTIntegerValue *ByteVal = SemaBuilder::CreateIntegerValue(SourceLoc, 1);
        ASTGlobalVar *bVar = Builder->CreateGlobalVar(Node, SourceLoc, ByteType, "b", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, bVar, ByteVal));

        // c
        ASTIntegerValue *ShortVal = SemaBuilder::CreateIntegerValue(SourceLoc, -2);
        ASTGlobalVar *cVar = Builder->CreateGlobalVar(Node, SourceLoc, ShortType, "c", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, cVar, ShortVal));

        // d
        ASTIntegerValue *UShortVal = SemaBuilder::CreateIntegerValue(SourceLoc, 2);
        ASTGlobalVar *dVar = Builder->CreateGlobalVar(Node, SourceLoc, UShortType, "d", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, dVar, UShortVal));

        // e
        ASTIntegerValue *IntVal = SemaBuilder::CreateIntegerValue(SourceLoc, -3);
        ASTGlobalVar *eVar = Builder->CreateGlobalVar(Node, SourceLoc, IntType, "e", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, eVar, IntVal));

        // f
        ASTIntegerValue *UIntVal = SemaBuilder::CreateIntegerValue(SourceLoc, 3);
        ASTGlobalVar *fVar = Builder->CreateGlobalVar(Node, SourceLoc, UIntType, "f", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, fVar, UIntVal));

        // g
        ASTIntegerValue *LongVal = SemaBuilder::CreateIntegerValue(SourceLoc, -4);
        ASTGlobalVar *gVar = Builder->CreateGlobalVar(Node, SourceLoc, LongType, "g", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, gVar, LongVal));

        // h
        ASTIntegerValue *ULongVal = SemaBuilder::CreateIntegerValue(SourceLoc, 4);
        ASTGlobalVar *hVar = Builder->CreateGlobalVar(Node, SourceLoc, ULongType, "h", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, hVar, ULongVal));

        // i
        ASTFloatingValue *FloatVal = SemaBuilder::CreateFloatingValue(SourceLoc, 1.5);
        ASTGlobalVar *iVar = Builder->CreateGlobalVar(Node, SourceLoc, FloatType, "i", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, iVar, FloatVal));

        // j
        ASTFloatingValue *DoubleVal = SemaBuilder::CreateFloatingValue(SourceLoc, 2.5);
        ASTGlobalVar *jVar = Builder->CreateGlobalVar(Node, SourceLoc, DoubleType, "j", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node,jVar, DoubleVal));

        // k (empty array)
        ASTArrayValue *ArrayValEmpty = SemaBuilder::CreateArrayValue(SourceLoc);
        ASTGlobalVar *kVar = Builder->CreateGlobalVar(Node, SourceLoc, ArrayInt0Type, "k", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, kVar, ArrayValEmpty));

        // l (2 val)
        ASTArrayValue *ArrayVal = SemaBuilder::CreateArrayValue(SourceLoc);
        ASTArrayType *ArrayInt2Type = SemaBuilder::CreateArrayType(SourceLoc, IntType,
          Builder->CreateExpr(nullptr, SemaBuilder::CreateIntegerValue(SourceLoc, 2)));
        Builder->AddArrayValue(ArrayVal, SemaBuilder::CreateIntegerValue(SourceLoc, 1)); // ArrayVal = {1}
        Builder->AddArrayValue(ArrayVal, SemaBuilder::CreateIntegerValue(SourceLoc, 2)); // ArrayVal = {1, 1}
        ASTGlobalVar *lVar = Builder->CreateGlobalVar(Node, SourceLoc, ArrayInt2Type, "l", VisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, lVar, ArrayVal));

        // Generate Code
        EXPECT_FALSE(Diags.hasErrorOccurred());
        std::string output;

        // a
        GlobalVariable *aGVar = (GlobalVariable *)CGM->GenGlobalVar(aVar)->getPointer();
        testing::internal::CaptureStdout();
        aGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@a = global i1 true");

        // b
        GlobalVariable *bGVar = (GlobalVariable *)CGM->GenGlobalVar(bVar)->getPointer();
        testing::internal::CaptureStdout();
        bGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@b = global i8 1");

        // c
        GlobalVariable *cGVar = (GlobalVariable *)CGM->GenGlobalVar(cVar)->getPointer();
        testing::internal::CaptureStdout();
        cGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@c = global i16 -2");

        // d
        GlobalVariable *dGVar = (GlobalVariable *)CGM->GenGlobalVar(dVar)->getPointer();
        testing::internal::CaptureStdout();
        dGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@d = global i16 2");

        // e
        GlobalVariable *eGVar = (GlobalVariable *)CGM->GenGlobalVar(eVar)->getPointer();
        testing::internal::CaptureStdout();
        eGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@e = global i32 -3");

        // f
        GlobalVariable *fGVar = (GlobalVariable *)CGM->GenGlobalVar(fVar)->getPointer();
        testing::internal::CaptureStdout();
        fGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@f = global i32 3");

        // g
        GlobalVariable *gGVar = (GlobalVariable *)CGM->GenGlobalVar(gVar)->getPointer();
        testing::internal::CaptureStdout();
        gGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@g = global i64 -4");

        // h
        GlobalVariable *hGVar = (GlobalVariable *)CGM->GenGlobalVar(hVar)->getPointer();
        testing::internal::CaptureStdout();
        hGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@h = global i64 4");

        // i
        GlobalVariable *iGVar = (GlobalVariable *)CGM->GenGlobalVar(iVar)->getPointer();
        testing::internal::CaptureStdout();
        iGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@i = global float 1.500000e+00");

        // j
        GlobalVariable *jGVar = (GlobalVariable *)CGM->GenGlobalVar(jVar)->getPointer();
        testing::internal::CaptureStdout();
        jGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@j = global double 2.500000e+00");

        GlobalVariable *kGVar = (GlobalVariable *)CGM->GenGlobalVar(kVar)->getPointer();
        testing::internal::CaptureStdout();
        kGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@k = global [0 x i32] zeroinitializer");

        GlobalVariable *lGVar = (GlobalVariable *)CGM->GenGlobalVar(lVar)->getPointer();
        testing::internal::CaptureStdout();
        lGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@l = global [2 x i32] [i32 1, i32 2]");
    }

    TEST_F(CodeGenTest, CGFunc) {
        ASTNode *Node = CreateNode();

        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, Builder->CreateParam(SourceLoc, IntType, "P1", false)));
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, Builder->CreateParam(SourceLoc, FloatType, "P2", false)));
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, Builder->CreateParam(SourceLoc, BoolType, "P3", false)));
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, Builder->CreateParam(SourceLoc, LongType, "P4", false)));
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, Builder->CreateParam(SourceLoc, DoubleType, "P5", false)));
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, Builder->CreateParam(SourceLoc, ByteType, "P6", false)));
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, Builder->CreateParam(SourceLoc, ShortType, "P7", false)));
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, Builder->CreateParam(SourceLoc, UShortType, "P8", false)));
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, Builder->CreateParam(SourceLoc, UIntType, "P9", false)));
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, Builder->CreateParam(SourceLoc, ULongType, "P10", false)));

        ASTIntegerValue *IntVal = SemaBuilder::CreateIntegerValue(SourceLoc, 1);
        ASTReturn *Return = Builder->CreateReturn(SourceLoc);
        Builder->CreateExpr(Return, IntVal);
        EXPECT_TRUE(Builder->AddStmt(MainFn, Return));

        EXPECT_TRUE(Builder->AddFunction(Node, MainFn));
        EXPECT_TRUE(Builder->AddNode(Node));
        EXPECT_TRUE(Builder->Build());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0, float %1, i1 %2, i64 %3, double %4, i8 %5, i16 %6, i16 %7, i32 %8, i64 %9) {\n"
                          "entry:\n"
                          "  %10 = alloca i32, align 4\n"
                          "  %11 = alloca float, align 4\n"
                          "  %12 = alloca i8, align 1\n"
                          "  %13 = alloca i64, align 8\n"
                          "  %14 = alloca double, align 8\n"
                          "  %15 = alloca i8, align 1\n"
                          "  %16 = alloca i16, align 2\n"
                          "  %17 = alloca i16, align 2\n"
                          "  %18 = alloca i32, align 4\n"
                          "  %19 = alloca i64, align 8\n"
                          "  store i32 %0, i32* %10, align 4\n"
                          "  store float %1, float* %11, align 4\n"
                          "  %20 = zext i1 %2 to i8\n"
                          "  store i8 %20, i8* %12, align 1\n"
                          "  store i64 %3, i64* %13, align 8\n"
                          "  store double %4, double* %14, align 8\n"
                          "  store i8 %5, i8* %15, align 1\n"
                          "  store i16 %6, i16* %16, align 2\n"
                          "  store i16 %7, i16* %17, align 2\n"
                          "  store i32 %8, i32* %18, align 4\n"
                          "  store i64 %9, i64* %19, align 8\n"
                          "  ret i32 1\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGFuncUseGlobalVar) {
        ASTNode *Node = CreateNode();

        // float G = 2.0
        ASTFloatingValue *FloatingVal = SemaBuilder::CreateFloatingValue(SourceLoc, 2.0);
        ASTGlobalVar *GVar = Builder->CreateGlobalVar(Node, SourceLoc, FloatType, "G", VisibilityKind::V_PRIVATE, false);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, GVar, FloatingVal));

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);

        // int A
        ASTLocalVar *VarA = Builder->CreateLocalVar(SourceLoc, IntType, "A", false);
        EXPECT_TRUE(Builder->AddStmt(MainFn, VarA));
        
        // A = 1
        ASTVarAssign * VarAAssign = Builder->CreateVarAssign(Builder->CreateVarRef(VarA));
        Builder->CreateExpr(VarAAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder->AddStmt(MainFn, VarAAssign));

        // GlobalVar
        // G = 1
        ASTVarRef *VarRefG = Builder->CreateVarRef(GVar);
        ASTVarAssign * GVarAssign = Builder->CreateVarAssign(VarRefG);
        Builder->CreateExpr(GVarAssign, SemaBuilder::CreateFloatingValue(SourceLoc, 1));
        EXPECT_TRUE(Builder->AddStmt(MainFn, GVarAssign));

        // return A
        ASTReturn *Return = Builder->CreateReturn(SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(VarA));
        EXPECT_TRUE(Builder->AddStmt(MainFn, Return));
        
        // add to Node
        EXPECT_TRUE(Builder->AddFunction(Node, MainFn));
        EXPECT_TRUE(Builder->AddNode(Node));
        EXPECT_TRUE(Builder->Build());

        // Generate Code
        CGM->GenGlobalVar(GVar);
        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca i32, align 4\n"
                          "  store i32 1, i32* %0, align 4\n"
                          "  store float 1.000000e+00, float* @G, align 4\n"
                          "  %1 = load i32, i32* %0, align 4\n"
                          "  ret i32 %1\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGFuncRetFn) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);

        // test()
        ASTFunction *TestFn = Builder->CreateFunction(Node, SourceLoc, IntType, "test", VisibilityKind::V_DEFAULT);

        // call test()
        ASTExprStmt *ExprStmt = Builder->CreateExprStmt(SourceLoc);
        ASTFunctionCall *TestCall = Builder->CreateFunctionCall(ExprStmt, TestFn);
        Builder->CreateExpr(ExprStmt, TestCall);
        EXPECT_TRUE(Builder->AddStmt(MainFn, ExprStmt));

        //return test()
        ASTReturn *Return = Builder->CreateReturn(SourceLoc);
        Builder->CreateExpr(Return, TestCall);
        EXPECT_TRUE(Builder->AddStmt(MainFn, Return));

        // add to Node
        EXPECT_TRUE(Builder->AddFunction(Node, MainFn));
        EXPECT_TRUE(Builder->AddFunction(Node, TestFn));
        EXPECT_TRUE(Builder->AddNode(Node));
        EXPECT_TRUE(Builder->Build());


        // Generate Code
        CGM->GenFunction(TestFn);
        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = call i32 @test()\n"
                          "  %1 = call i32 @test()\n"
                          "  ret i32 %1\n"
                          "}\n");
    }

    /**
     * main(int a, int b, int c) {
     *  return 1 + a * b / (c - 2)
     * }
     */
    TEST_F(CodeGenTest, CGGroupExpr) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);
        ASTParam *aParam = Builder->CreateParam(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, aParam));
        ASTParam *bParam = Builder->CreateParam(SourceLoc, IntType, "b");
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, bParam));
        ASTParam *cParam = Builder->CreateParam(SourceLoc, IntType, "c");
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, cParam));

        ASTReturn *Return = Builder->CreateReturn(SourceLoc);
        // Create this expression: 1 + a * b / (c - 2)
        // E1 + (E2 * E3) / (E4 - E5)
        // E1 + (G2 / G3)
        // E1 + G1
        ASTValueExpr *E1 = Builder->CreateExpr(Return, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *E2 = Builder->CreateExpr(Return, Builder->CreateVarRef(aParam));
        ASTVarRefExpr *E3 = Builder->CreateExpr(Return, Builder->CreateVarRef(bParam));
        ASTVarRefExpr *E4 = Builder->CreateExpr(Return, Builder->CreateVarRef(cParam));
        ASTValueExpr *E5 = Builder->CreateExpr(Return, SemaBuilder::CreateIntegerValue(SourceLoc, 2));

        ASTBinaryGroupExpr *G2 = Builder->CreateBinaryExpr(Return, SourceLoc, ARITH_MUL, E2, E3);
        ASTBinaryGroupExpr *G3 = Builder->CreateBinaryExpr(Return, SourceLoc, ARITH_SUB, E4, E5);
        ASTBinaryGroupExpr *G1 = Builder->CreateBinaryExpr(Return, SourceLoc, ARITH_DIV, G2, G3);
        ASTBinaryGroupExpr *Group = Builder->CreateBinaryExpr(Return, SourceLoc, ARITH_ADD, E1, G1);

        EXPECT_TRUE(Builder->AddStmt(MainFn, Return));

        // Add to Node
        EXPECT_TRUE(Builder->AddFunction(Node, MainFn));
        EXPECT_TRUE(Builder->AddNode(Node));
        EXPECT_TRUE(Builder->Build());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0, i32 %1, i32 %2) {\n"
                          "entry:\n"
                          "  %3 = alloca i32, align 4\n"
                          "  %4 = alloca i32, align 4\n"
                          "  %5 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %3, align 4\n"
                          "  store i32 %1, i32* %4, align 4\n"
                          "  store i32 %2, i32* %5, align 4\n"
                          "  %6 = load i32, i32* %3, align 4\n"
                          "  %7 = load i32, i32* %4, align 4\n"
                          "  %8 = mul i32 %6, %7\n"
                          "  %9 = load i32, i32* %5, align 4\n"
                          "  %10 = sub i32 %9, 2\n"
                          "  %11 = sdiv i32 %8, %10\n"
                          "  %12 = add i32 1, %11\n"
                          "  ret i32 %12\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGArithOp) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);
        ASTParam *aParam = Builder->CreateParam(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, aParam));
        ASTParam *bParam = Builder->CreateParam(SourceLoc, IntType, "b");
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, bParam));
        ASTParam *cParam = Builder->CreateParam(SourceLoc, IntType, "c");
        EXPECT_TRUE(Builder->AddFunctionParam(MainFn, cParam));

        // a = 0
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 0));
        EXPECT_TRUE(Builder->AddStmt(MainFn, aVarAssign));

        // b = 0
        ASTVarAssign *bVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(bParam));
        Builder->CreateExpr(bVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 0));
        EXPECT_TRUE(Builder->AddStmt(MainFn, bVarAssign));

        // c = a + b
        ASTVarAssign * cAddVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cAddVarAssign, SourceLoc,ARITH_ADD,
                                             Builder->CreateExpr(cAddVarAssign, Builder->CreateVarRef(aParam)),
                                             Builder->CreateExpr(cAddVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cAddVarAssign));

        // c = a - b
        ASTVarAssign * cSubVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cSubVarAssign, SourceLoc,ARITH_SUB,
                                                               Builder->CreateExpr(cSubVarAssign, Builder->CreateVarRef(aParam)),
                                                               Builder->CreateExpr(cSubVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cSubVarAssign));

        // c = a * b
        ASTVarAssign * cMulVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cMulVarAssign, SourceLoc,ARITH_MUL,
                                                               Builder->CreateExpr(cMulVarAssign, Builder->CreateVarRef(aParam)),
                                                               Builder->CreateExpr(cMulVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cMulVarAssign));

        // c = a / b
        ASTVarAssign * cDivVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cDivVarAssign, SourceLoc,ARITH_DIV,
                                                               Builder->CreateExpr(cDivVarAssign, Builder->CreateVarRef(aParam)),
                                                               Builder->CreateExpr(cDivVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cDivVarAssign));

        // c = a % b
        ASTVarAssign * cModVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cModVarAssign, SourceLoc,ARITH_MOD,
                                                               Builder->CreateExpr(cModVarAssign, Builder->CreateVarRef(aParam)),
                                                               Builder->CreateExpr(cModVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cModVarAssign));

        // c = a & b
        ASTVarAssign * cAndVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cAndVarAssign, SourceLoc,ARITH_AND,
                                                               Builder->CreateExpr(cAndVarAssign, Builder->CreateVarRef(aParam)),
                                                               Builder->CreateExpr(cAndVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cAndVarAssign));

        // c = a | b
        ASTVarAssign * cOrVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cOrVarAssign, SourceLoc,ARITH_OR,
                                                               Builder->CreateExpr(cOrVarAssign, Builder->CreateVarRef(aParam)),
                                                               Builder->CreateExpr(cOrVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cOrVarAssign));

        // c = a xor b
        ASTVarAssign * cXorVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cXorVarAssign, SourceLoc,ARITH_XOR,
                                                               Builder->CreateExpr(cXorVarAssign, Builder->CreateVarRef(aParam)),
                                                               Builder->CreateExpr(cXorVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cXorVarAssign));

        // c = a << b
        ASTVarAssign * cShlVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cShlVarAssign, SourceLoc,ARITH_SHIFT_L,
                                                               Builder->CreateExpr(cShlVarAssign, Builder->CreateVarRef(aParam)),
                                                               Builder->CreateExpr(cShlVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cShlVarAssign));

        // c = a >> b
        ASTVarAssign * cShrVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cShrVarAssign, SourceLoc,ARITH_SHIFT_R,
                                                               Builder->CreateExpr(cShrVarAssign, Builder->CreateVarRef(aParam)),
                                                               Builder->CreateExpr(cShrVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cShrVarAssign));

        // ++c
        ASTExprStmt *cPreIncVarAssign = Builder->CreateExprStmt(SourceLoc);
        Builder->CreateUnaryExpr(cPreIncVarAssign, SourceLoc,ARITH_INCR, UNARY_PRE,
                                                               Builder->CreateExpr(cPreIncVarAssign, Builder->CreateVarRef(cParam)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cPreIncVarAssign));

        // c++
        ASTExprStmt *cPostIncVarAssign = Builder->CreateExprStmt(SourceLoc);
        Builder->CreateUnaryExpr(cPostIncVarAssign, SourceLoc,ARITH_INCR, UNARY_POST,
                                                              Builder->CreateExpr(cPostIncVarAssign, Builder->CreateVarRef(cParam)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cPostIncVarAssign));

        // ++c
        ASTExprStmt *cPreDecVarAssign = Builder->CreateExprStmt(SourceLoc);
        Builder->CreateUnaryExpr(cPreDecVarAssign, SourceLoc, ARITH_DECR, UNARY_PRE,
                                                              Builder->CreateExpr(cPreDecVarAssign, Builder->CreateVarRef(cParam)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cPreDecVarAssign));

        // c++
        ASTExprStmt *cPostDecVarAssign = Builder->CreateExprStmt(SourceLoc);
        Builder->CreateUnaryExpr(cPostDecVarAssign, SourceLoc, ARITH_DECR, UNARY_POST,
                                                              Builder->CreateExpr(cPostDecVarAssign, Builder->CreateVarRef(cParam)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cPostDecVarAssign));

        //return c
        ASTReturn *Return = Builder->CreateReturn(SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(cParam));
        EXPECT_TRUE(Builder->AddStmt(MainFn, Return));

        // Add to Node
        EXPECT_TRUE(Builder->AddFunction(Node, MainFn));
        EXPECT_TRUE(Builder->AddNode(Node));
        EXPECT_TRUE(Builder->Build());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0, i32 %1, i32 %2) {\n"
                          "entry:\n"
                          "  %3 = alloca i32, align 4\n"
                          "  %4 = alloca i32, align 4\n"
                          "  %5 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %3, align 4\n"
                          "  store i32 %1, i32* %4, align 4\n"
                          "  store i32 %2, i32* %5, align 4\n"
                          "  store i32 0, i32* %3, align 4\n"
                          "  store i32 0, i32* %4, align 4\n"
                          "  %6 = load i32, i32* %3, align 4\n"
                          "  %7 = load i32, i32* %4, align 4\n"
                          "  %8 = add i32 %6, %7\n"
                          "  store i32 %8, i32* %5, align 4\n"
                          "  %9 = sub i32 %6, %7\n"
                          "  store i32 %9, i32* %5, align 4\n"
                          "  %10 = mul i32 %6, %7\n"
                          "  store i32 %10, i32* %5, align 4\n"
                          "  %11 = sdiv i32 %6, %7\n"
                          "  store i32 %11, i32* %5, align 4\n"
                          "  %12 = srem i32 %6, %7\n"
                          "  store i32 %12, i32* %5, align 4\n"
                          "  %13 = and i32 %6, %7\n"
                          "  store i32 %13, i32* %5, align 4\n"
                          "  %14 = or i32 %6, %7\n"
                          "  store i32 %14, i32* %5, align 4\n"
                          "  %15 = xor i32 %6, %7\n"
                          "  store i32 %15, i32* %5, align 4\n"
                          "  %16 = shl i32 %6, %7\n"
                          "  store i32 %16, i32* %5, align 4\n"
                          "  %17 = ashr i32 %6, %7\n"
                          "  store i32 %17, i32* %5, align 4\n"
                          // Unary Operations
                          "  %18 = load i32, i32* %5, align 4\n"
                          "  %19 = add nsw i32 %18, 1\n"
                          "  store i32 %19, i32* %5, align 4\n"
                          "  %20 = load i32, i32* %5, align 4\n"
                          "  %21 = add nsw i32 %20, 1\n"
                          "  store i32 %21, i32* %5, align 4\n"
                          "  %22 = load i32, i32* %5, align 4\n"
                          "  %23 = add nsw i32 %22, -1\n"
                          "  store i32 %23, i32* %5, align 4\n"
                          "  %24 = load i32, i32* %5, align 4\n"
                          "  %25 = add nsw i32 %24, -1\n"
                          "  store i32 %25, i32* %5, align 4\n"
                          "  %26 = load i32, i32* %5, align 4\n"
                          "  ret i32 %26\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGComparatorOp) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);
        ASTLocalVar *aVar = Builder->CreateLocalVar(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder->AddStmt(MainFn, aVar));
        ASTLocalVar *bVar = Builder->CreateLocalVar(SourceLoc, IntType, "b");
        EXPECT_TRUE(Builder->AddStmt(MainFn, bVar));
        ASTLocalVar *cVar = Builder->CreateLocalVar(SourceLoc, BoolType, "c");
        EXPECT_TRUE(Builder->AddStmt(MainFn, cVar));

        // a = 0
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(aVar));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 0));
        EXPECT_TRUE(Builder->AddStmt(MainFn, aVarAssign));

        // b = 0
        ASTVarAssign *bVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(bVar));
        Builder->CreateExpr(bVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 0));
        EXPECT_TRUE(Builder->AddStmt(MainFn, bVarAssign));

        // Operation Equal
        ASTVarAssign * cEqVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cEqVarAssign, SourceLoc, COMP_EQ,
                                                               Builder->CreateExpr(cEqVarAssign, Builder->CreateVarRef(aVar)),
                                                               Builder->CreateExpr(cEqVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cEqVarAssign));

        // Operation Not Equal
        ASTVarAssign * cNeqVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cNeqVarAssign, SourceLoc, COMP_NE,
                                                               Builder->CreateExpr(cNeqVarAssign, Builder->CreateVarRef(aVar)),
                                                               Builder->CreateExpr(cNeqVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cNeqVarAssign));

        // Operation Greater Than
        ASTVarAssign * cGtVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cGtVarAssign, SourceLoc, COMP_GT,
                                                               Builder->CreateExpr(cGtVarAssign, Builder->CreateVarRef(aVar)),
                                                               Builder->CreateExpr(cGtVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cGtVarAssign));

        // Operation Greater Than or Equal
        ASTVarAssign * cGteVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cGteVarAssign, SourceLoc, COMP_GTE,
                                                               Builder->CreateExpr(cGteVarAssign, Builder->CreateVarRef(aVar)),
                                                               Builder->CreateExpr(cGteVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cGteVarAssign));

        // Operation Less Than
        ASTVarAssign * cLtVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cLtVarAssign, SourceLoc, COMP_LT,
                                                               Builder->CreateExpr(cLtVarAssign, Builder->CreateVarRef(aVar)),
                                                               Builder->CreateExpr(cLtVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cLtVarAssign));

        // Operation Less Than or Equal
        ASTVarAssign * cLteVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cLteVarAssign, SourceLoc, COMP_LTE,
                                                               Builder->CreateExpr(cLteVarAssign, Builder->CreateVarRef(aVar)),
                                                               Builder->CreateExpr(cLteVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cLteVarAssign));

        //return c
        ASTReturn *Return = Builder->CreateReturn(SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(cVar));
        EXPECT_TRUE(Builder->AddStmt(MainFn, Return));

        // Add to Node
        EXPECT_TRUE(Builder->AddFunction(Node, MainFn));
        EXPECT_TRUE(Builder->AddNode(Node));
        EXPECT_TRUE(Builder->Build());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca i32, align 4\n"
                          "  %1 = alloca i32, align 4\n"
                          "  %2 = alloca i8, align 1\n"
                          "  store i32 0, i32* %0, align 4\n"
                          "  store i32 0, i32* %1, align 4\n"
                          "  %3 = load i32, i32* %0, align 4\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  %5 = icmp eq i32 %3, %4\n"
                          "  %6 = zext i1 %5 to i8\n"
                          "  store i8 %6, i8* %2, align 1\n"
                          "  %7 = icmp ne i32 %3, %4\n"
                          "  %8 = zext i1 %7 to i8\n"
                          "  store i8 %8, i8* %2, align 1\n"
                          "  %9 = icmp sgt i32 %3, %4\n"
                          "  %10 = zext i1 %9 to i8\n"
                          "  store i8 %10, i8* %2, align 1\n"
                          "  %11 = icmp sge i32 %3, %4\n"
                          "  %12 = zext i1 %11 to i8\n"
                          "  store i8 %12, i8* %2, align 1\n"
                          "  %13 = icmp slt i32 %3, %4\n"
                          "  %14 = zext i1 %13 to i8\n"
                          "  store i8 %14, i8* %2, align 1\n"
                          "  %15 = icmp sle i32 %3, %4\n"
                          "  %16 = zext i1 %15 to i8\n"
                          "  store i8 %16, i8* %2, align 1\n"
                          "  %17 = load i8, i8* %2, align 1\n"
                          "  %18 = trunc i8 %17 to i1\n"
                          "  %19 = zext i1 %18 to i32\n"
                          "  ret i32 %19\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGLogicOp) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);
        ASTLocalVar *aVar = Builder->CreateLocalVar(SourceLoc, BoolType, "a");
        EXPECT_TRUE(Builder->AddStmt(MainFn, aVar));
        ASTLocalVar *bVar = Builder->CreateLocalVar(SourceLoc, BoolType, "b");
        EXPECT_TRUE(Builder->AddStmt(MainFn, bVar));
        ASTLocalVar *cVar = Builder->CreateLocalVar(SourceLoc, BoolType, "c");
        EXPECT_TRUE(Builder->AddStmt(MainFn, cVar));

        // a = false
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(aVar));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder->AddStmt(MainFn, aVarAssign));

        // b = false
        ASTVarAssign *bVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(bVar));
        Builder->CreateExpr(bVarAssign, SemaBuilder::CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder->AddStmt(MainFn, bVarAssign));

        // Operation And Logic
        ASTVarAssign * cAndVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cAndVarAssign, SourceLoc, LOGIC_AND,
                                                               Builder->CreateExpr(cAndVarAssign, Builder->CreateVarRef(aVar)),
                                                               Builder->CreateExpr(cAndVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cAndVarAssign));

        // Operation Or Logic
        ASTVarAssign * cOrVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cOrVarAssign, SourceLoc, LOGIC_OR,
                                                               Builder->CreateExpr(cOrVarAssign, Builder->CreateVarRef(aVar)),
                                                               Builder->CreateExpr(cOrVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cOrVarAssign));

        //return c
        ASTReturn *Return = Builder->CreateReturn(SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(cVar));
        EXPECT_TRUE(Builder->AddStmt(MainFn, Return));

        // Add to Node
        EXPECT_TRUE(Builder->AddFunction(Node, MainFn));
        EXPECT_TRUE(Builder->AddNode(Node));
        EXPECT_TRUE(Builder->Build());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca i8, align 1\n"
                          "  %1 = alloca i8, align 1\n"
                          "  %2 = alloca i8, align 1\n"
                          "  store i8 0, i8* %0, align 1\n"
                          "  store i8 0, i8* %1, align 1\n"
                          "  %3 = load i8, i8* %0, align 1\n"
                          "  %4 = trunc i8 %3 to i1\n"
                          "  br i1 %4, label %and, label %and1\n"
                          "\n"
                          "and:                                              ; preds = %entry\n"
                          "  %5 = load i8, i8* %1, align 1\n"
                          "  %6 = trunc i8 %5 to i1\n"
                          "  br label %and1\n"
                          "\n"
                          "and1:                                             ; preds = %and, %entry\n"
                          "  %7 = phi i1 [ false, %entry ], [ %6, %and ]\n"
                          "  %8 = zext i1 %7 to i8\n"
                          "  store i8 %8, i8* %2, align 1\n"
                          "  %9 = load i8, i8* %0, align 1\n"
                          "  %10 = trunc i8 %9 to i1\n"
                          "  br i1 %10, label %or2, label %or\n"
                          "\n"
                          "or:                                               ; preds = %and1\n"
                          "  %11 = load i8, i8* %1, align 1\n"
                          "  %12 = trunc i8 %11 to i1\n"
                          "  br label %or2\n"
                          "\n"
                          "or2:                                              ; preds = %or, %and1\n"
                          "  %13 = phi i1 [ true, %and1 ], [ %12, %or ]\n"
                          "  %14 = zext i1 %13 to i8\n"
                          "  store i8 %14, i8* %2, align 1\n"
                          "  %15 = load i8, i8* %2, align 1\n"
                          "  %16 = trunc i8 %15 to i1\n"
                          "  %17 = zext i1 %16 to i32\n"
                          "  ret i32 %17\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGTernaryOp) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);
        ASTLocalVar *aVar = Builder->CreateLocalVar(SourceLoc, BoolType, "a");
        EXPECT_TRUE(Builder->AddStmt(MainFn, aVar));
        ASTLocalVar *bVar = Builder->CreateLocalVar(SourceLoc, BoolType, "b");
        EXPECT_TRUE(Builder->AddStmt(MainFn, bVar));
        ASTLocalVar *cVar = Builder->CreateLocalVar(SourceLoc, IntType, "c");
        EXPECT_TRUE(Builder->AddStmt(MainFn, cVar));

        // a = false
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(aVar));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder->AddStmt(MainFn, aVarAssign));

        // b = false
        ASTVarAssign *bVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(bVar));
        Builder->CreateExpr(bVarAssign, SemaBuilder::CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder->AddStmt(MainFn, bVarAssign));

        // return
        ASTVarAssign * cOrVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(cVar));
        ASTBinaryGroupExpr *Cond = Builder->CreateBinaryExpr(cOrVarAssign, SourceLoc, COMP_EQ,
                                                             Builder->CreateExpr(cOrVarAssign,
                                                                                 Builder->CreateVarRef(aVar)),
                                                             Builder->CreateExpr(cOrVarAssign,
                                                                                 Builder->CreateVarRef(bVar)));
        Builder->CreateTernaryExpr(cOrVarAssign, SourceLoc, Cond,
                                                                             Builder->CreateExpr(cOrVarAssign,
                                                                                                 Builder->CreateVarRef(aVar)),
                                                                             Builder->CreateExpr(cOrVarAssign,
                                                                                                 Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(MainFn, cOrVarAssign));

        //return c
        ASTReturn *Return = Builder->CreateReturn(SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(cVar));
        EXPECT_TRUE(Builder->AddStmt(MainFn, Return));

        // Add to Node
        EXPECT_TRUE(Builder->AddFunction(Node, MainFn));
        EXPECT_TRUE(Builder->AddNode(Node));
        EXPECT_TRUE(Builder->Build());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca i8, align 1\n"
                          "  %1 = alloca i8, align 1\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store i8 0, i8* %0, align 1\n"
                          "  store i8 0, i8* %1, align 1\n"
                          "  %3 = load i8, i8* %0, align 1\n"
                          "  %4 = load i8, i8* %1, align 1\n"
                          "  %5 = icmp eq i8 %3, %4\n"
                          "  br i1 %5, label %terntrue, label %ternfalse\n"
                          "\n"
                          "terntrue:                                         ; preds = %entry\n"
                          "  %6 = load i8, i8* %0, align 1\n"
                          "  %7 = trunc i8 %6 to i1\n"
                          "  br label %ternend\n"
                          "\n"
                          "ternfalse:                                        ; preds = %entry\n"
                          "  %8 = load i8, i8* %1, align 1\n"
                          "  %9 = trunc i8 %8 to i1\n"
                          "  br label %ternend\n"
                          "\n"
                          "ternend:                                          ; preds = %ternfalse, %terntrue\n"
                          "  %10 = phi i1 [ %7, %terntrue ], [ %9, %ternfalse ]\n"
                          "  %11 = zext i1 %10 to i32\n"
                          "  store i32 %11, i32* %2, align 4\n"
                          "  %12 = load i32, i32* %2, align 4\n"
                          "  ret i32 %12\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, DISABLED_CGIfBlock) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);
        ASTLocalVar *aVar = Builder->CreateLocalVar(SourceLoc, IntType, "a");
        Builder->CreateExpr(aVar, SemaBuilder::CreateIntegerValue(SourceLoc, 0));
        EXPECT_TRUE(Builder->AddStmt(MainFn, aVar));

        // Create if block
        ASTIfBlock *IfBlock = Builder->CreateIfBlock(Builder->getBlock(MainFn), SourceLoc);

        // Compare (a == 1)
        ASTVarRefExpr *aVarRef = Builder->CreateExpr(IfBlock, Builder->CreateVarRef(aVar));
        ASTValueExpr *Value1 = Builder->CreateExpr(IfBlock, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        ASTBinaryGroupExpr *IfCond = Builder->CreateBinaryExpr(IfBlock, SourceLoc, COMP_EQ, aVarRef, Value1);

        // if (a == 1) { a = 2 }
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(Builder->CreateVarRef(aVar));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 2));
        EXPECT_TRUE(Builder->AddStmt(IfBlock, aVarAssign));
        EXPECT_TRUE(Builder->AddIfBlock(IfBlock, IfCond));
        EXPECT_TRUE(Builder->AddStmt(MainFn, IfBlock));

        // return a
        ASTReturn *Return = Builder->CreateReturn(SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(aVar));
        EXPECT_TRUE(Builder->AddStmt(MainFn, Return));

        // Add to Node
        EXPECT_TRUE(Builder->AddFunction(Node, MainFn));
        EXPECT_TRUE(Builder->AddNode(Node));
        EXPECT_TRUE(Builder->Build());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
                          "entry:\n"
                          "  %1 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %1, align 4\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  %3 = icmp eq i32 1, %2\n"
                          "  br i1 %3, label %ifthen, label %endif\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "endif:                                            ; preds = %ifthen, %entry\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  ret i32 %4\n"
                          "}\n");
    }

//    TEST_F(CodeGenTest, CGIfElseBlock) {
//        ASTNode *Node = CreateNode();
//
//        // main()
//        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);
//        ASTParam *Param = MainFn->addParam(SourceLoc, IntType, "a");
//
//        ASTValueExpr *OneCost = new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 1));
//        ASTVarRefExpr *ARef = new ASTVarRefExpr(new ASTVarRef(Param));
//        ASTGroupExpr *Cond = new ASTBinaryGroupExpr(SourceLoc,
//                                                    COMP_EQ,
//                                                    ARef,
//                                                    OneCost);
//
//        // if (a == 1) { a = 1 }
//        ASTIfBlock *IfBlock = MainFn->getBody()->AddIfBlock(SourceLoc, Cond);
//        ASTVarAssign *A2 = new ASTVarAssign(SourceLoc, IfBlock, Param);
//        A2->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 1)));
//        IfBlock->AddVarAssign(A2);
//
//        // else {a == 2}
//        ASTElseBlock *ElseBlock = IfBlock->AddElseBlock(SourceLoc);
//        ASTVarAssign *A3 = new ASTVarAssign(SourceLoc, ElseBlock, Param);
//        A3->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 2)));
//        ElseBlock->AddVarAssign(A3);
//
//        MainFn->getBody()->AddReturn(SourceLoc, ARef);
//
//        // Add to Node
//        Builder->AddFunction(Node, MainFn);
//
//        // Generate Code
//        CodeGenModule *CGM = Node->getCodeGen();
//        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
//        CGF->GenBody();
//        Function *F = CGF->getFunction();
//
//        EXPECT_FALSE(Diags.hasErrorOccurred());
//        testing::internal::CaptureStdout();
//        F->print(llvm::outs());
//        std::string output = testing::internal::GetCapturedStdout();
//
//        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
//                          "entry:\n"
//                          "  %1 = alloca i32, align 4\n"
//                          "  store i32 %0, i32* %1, align 4\n"
//                          "  %2 = load i32, i32* %1, align 4\n"
//                          "  %3 = icmp eq i32 %2, 1\n"
//                          "  br i1 %3, label %ifthen, label %else\n"
//                          "\n"
//                          "ifthen:                                           ; preds = %entry\n"
//                          "  store i32 1, i32* %1, align 4\n"
//                          "  br label %endif\n"
//                          "\n"
//                          "else:                                             ; preds = %entry\n"
//                          "  store i32 2, i32* %1, align 4\n"
//                          "  br label %endif\n"
//                          "\n"
//                          "endif:                                            ; preds = %else, %ifthen\n"
//                          "  %4 = load i32, i32* %1, align 4\n"
//                          "  ret i32 %4\n"
//                          "}\n");
//    }
//
//    TEST_F(CodeGenTest, CGIfElsifElseBlock) {
//        ASTNode *Node = CreateNode();
//
//        // main()
//        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);
//        ASTParam *Param = MainFn->addParam(SourceLoc, IntType, "a");
//
//        ASTValueExpr *OneCost = new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 1));
//        ASTVarRefExpr *ARef = new ASTVarRefExpr(new ASTVarRef(Param));
//        ASTGroupExpr *Cond = new ASTBinaryGroupExpr(SourceLoc,
//                                                    COMP_EQ,
//                                                    ARef,
//                                                    OneCost);
//
//        // if (a == 1) { a = 11 }
//        ASTIfBlock *IfBlock = MainFn->getBody()->AddIfBlock(SourceLoc, Cond);
//        ASTVarAssign *A2 = new ASTVarAssign(SourceLoc, IfBlock, Param);
//        A2->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 11)));
//        IfBlock->AddVarAssign(A2);
//
//        // elsif (a == 1) { a = 22 }
//        ASTElsifBlock *ElsifBlock = IfBlock->AddElsifBlock(SourceLoc, Cond);
//        ASTVarAssign *A3 = new ASTVarAssign(SourceLoc, ElsifBlock, Param);
//        A3->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 22)));
//        ElsifBlock->AddVarAssign(A3);
//
//        // elsif (a == 1) { a = 33 }
//        ASTElsifBlock *Elsif2Block = IfBlock->AddElsifBlock(SourceLoc, Cond);
//        ASTVarAssign *A4 = new ASTVarAssign(SourceLoc, Elsif2Block, Param);
//        A4->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 33)));
//        Elsif2Block->AddVarAssign(A4);
//
//        // else { a = 44 }
//        ASTElseBlock *ElseBlock = IfBlock->AddElseBlock(SourceLoc);
//        ASTVarAssign *A5 = new ASTVarAssign(SourceLoc, ElseBlock, Param);
//        A5->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 44)));
//        ElseBlock->AddVarAssign(A5);
//
//        MainFn->getBody()->AddReturn(SourceLoc, ARef);
//
//        // Add to Node
//        Builder->AddFunction(Node, MainFn);
//
//        // Generate Code
//        CodeGenModule *CGM = Node->getCodeGen();
//        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
//        CGF->GenBody();
//        Function *F = CGF->getFunction();
//
//        EXPECT_FALSE(Diags.hasErrorOccurred());
//        testing::internal::CaptureStdout();
//        F->print(llvm::outs());
//        std::string output = testing::internal::GetCapturedStdout();
//
//        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
//                          "entry:\n"
//                          "  %1 = alloca i32, align 4\n"
//                          "  store i32 %0, i32* %1, align 4\n"
//                          "  %2 = load i32, i32* %1, align 4\n"
//                          "  %3 = icmp eq i32 %2, 1\n"
//                          "  br i1 %3, label %ifthen, label %elsif\n"
//                          "\n"
//                          "ifthen:                                           ; preds = %entry\n"
//                          "  store i32 11, i32* %1, align 4\n"
//                          "  br label %endif\n"
//                          "\n"
//                          "elsif:                                            ; preds = %entry\n"
//                          "  %4 = load i32, i32* %1, align 4\n"
//                          "  %5 = icmp eq i32 %4, 1\n"
//                          "  br i1 %5, label %elsifthen, label %elsif1\n"
//                          "\n"
//                          "elsifthen:                                        ; preds = %elsif\n"
//                          "  store i32 22, i32* %1, align 4\n"
//                          "  br label %endif\n"
//                          "\n"
//                          "elsif1:                                           ; preds = %elsif\n"
//                          "  %6 = load i32, i32* %1, align 4\n"
//                          "  %7 = icmp eq i32 %6, 1\n"
//                          "  br i1 %7, label %elsifthen2, label %else\n"
//                          "\n"
//                          "elsifthen2:                                       ; preds = %elsif1\n"
//                          "  store i32 33, i32* %1, align 4\n"
//                          "  br label %endif\n"
//                          "\n"
//                          "else:                                             ; preds = %elsif1\n"
//                          "  store i32 44, i32* %1, align 4\n"
//                          "  br label %endif\n"
//                          "\n"
//                          "endif:                                            ; preds = %else, %elsifthen2, %elsifthen, %ifthen\n"
//                          "  %8 = load i32, i32* %1, align 4\n"
//                          "  ret i32 %8\n"
//                          "}\n");
//    }
//
//    TEST_F(CodeGenTest, CGIfElsifBlock) {
//        ASTNode *Node = CreateNode();
//
//        // main()
//        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);
//        ASTParam *Param = MainFn->addParam(SourceLoc, IntType, "a");
//
//        ASTValueExpr *OneCost = new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 1));
//        ASTVarRefExpr *ARef = new ASTVarRefExpr(new ASTVarRef(Param));
//        ASTGroupExpr *Cond = new ASTBinaryGroupExpr(SourceLoc,
//                                                    COMP_EQ,
//                                                    ARef,
//                                                    OneCost);
//
//        // if (a == 1) { a = 11 }
//        ASTIfBlock *IfBlock = MainFn->getBody()->AddIfBlock(SourceLoc, Cond);
//        ASTVarAssign *A2 = new ASTVarAssign(SourceLoc, IfBlock, Param);
//        A2->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 11)));
//        IfBlock->AddVarAssign(A2);
//
//        // elsif (a == 1) { a = 22 }
//        ASTElsifBlock *ElsifBlock = IfBlock->AddElsifBlock(SourceLoc, Cond);
//        ASTVarAssign *A3 = new ASTVarAssign(SourceLoc, ElsifBlock, Param);
//        A3->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 22)));
//        ElsifBlock->AddVarAssign(A3);
//
//        // elsif (a == 1) { a = 33 }
//        ASTElsifBlock *Elsif2Block = IfBlock->AddElsifBlock(SourceLoc, Cond);
//        ASTVarAssign *A4 = new ASTVarAssign(SourceLoc, Elsif2Block, Param);
//        A4->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 33)));
//        Elsif2Block->AddVarAssign(A4);
//
//        MainFn->getBody()->AddReturn(SourceLoc, ARef);
//
//        // Add to Node
//        Builder->AddFunction(Node, MainFn);
//
//        // Generate Code
//        CodeGenModule *CGM = Node->getCodeGen();
//        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
//        CGF->GenBody();
//        Function *F = CGF->getFunction();
//
//        EXPECT_FALSE(Diags.hasErrorOccurred());
//        testing::internal::CaptureStdout();
//        F->print(llvm::outs());
//        std::string output = testing::internal::GetCapturedStdout();
//
//        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
//                          "entry:\n  %1 = alloca i32, align 4\n"
//                          "  store i32 %0, i32* %1, align 4\n"
//                          "  %2 = load i32, i32* %1, align 4\n"
//                          "  %3 = icmp eq i32 %2, 1\n"
//                          "  br i1 %3, label %ifthen, label %elsif\n"
//                          "\n"
//                          "ifthen:                                           ; preds = %entry\n"
//                          "  store i32 11, i32* %1, align 4\n"
//                          "  br label %endif\n"
//                          "\nelsif:                                            ; preds = %entry\n"
//                          "  %4 = load i32, i32* %1, align 4\n"
//                          "  %5 = icmp eq i32 %4, 1\n"
//                          "  br i1 %5, label %elsifthen, label %elsif1\n"
//                          "\n"
//                          "elsifthen:                                        ; preds = %elsif\n"
//                          "  store i32 22, i32* %1, align 4\n"
//                          "  br label %endif\n"
//                          "\n"
//                          "elsif1:                                           ; preds = %elsif\n"
//                          "  %6 = load i32, i32* %1, align 4\n"
//                          "  %7 = icmp eq i32 %6, 1\n"
//                          "  br i1 %7, label %elsifthen2, label %endif\n"
//                          "\n"
//                          "elsifthen2:                                       ; preds = %elsif1\n"
//                          "  store i32 33, i32* %1, align 4\n"
//                          "  br label %endif\n"
//                          "\n"
//                          "endif:                                            ; preds = %elsifthen2, %elsif1, %elsifthen, %ifthen\n"
//                          "  %8 = load i32, i32* %1, align 4\n"
//                          "  ret i32 %8\n"
//                          "}\n");
//    }
//
//    TEST_F(CodeGenTest, CGSwitchBlock) {
//        ASTNode *Node = CreateNode();
//
//        // main()
//        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);
//        ASTParam *Param = MainFn->addParam(SourceLoc, IntType, "a");
//
//        ASTExpr *Cost1 = new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 1));
//        ASTExpr *Cost2 = new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 2));
//
//        ASTVarRefExpr *SwitchExpr = new ASTVarRefExpr(new ASTVarRef(Param));
//        ASTSwitchBlock *SwitchBlock = MainFn->getBody()->AddSwitchBlock(SourceLoc, SwitchExpr);
//
//        ASTSwitchCaseBlock *Case1Block = SwitchBlock->AddCase(SourceLoc, Cost1);
//        ASTVarAssign *A2 = new ASTVarAssign(SourceLoc, Case1Block, Param);
//        A2->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 1)));
//        Case1Block->AddVarAssign(A2);
//
//        ASTSwitchCaseBlock *Case2Block = SwitchBlock->AddCase(SourceLoc, Cost2);
//        ASTVarAssign *A3 = new ASTVarAssign(SourceLoc, Case2Block, Param);
//        A3->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 2)));
//        Case2Block->AddVarAssign(A3);
//        Case2Block->AddBreak(SourceLoc);
//
//        ASTBlock *DefaultBlock = SwitchBlock->setDefault(SourceLoc);
//        ASTVarAssign *A4 = new ASTVarAssign(SourceLoc, DefaultBlock, Param);
//        A4->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 3)));
//        DefaultBlock->AddVarAssign(A4);
//        DefaultBlock->AddBreak(SourceLoc);
//
//        MainFn->getBody()->AddReturn(SourceLoc, Cost1);
//
//        // Add to Node
//        Builder->AddFunction(Node, MainFn);
//
//        // Generate Code
//        CodeGenModule *CGM = Node->getCodeGen();
//        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
//        CGF->GenBody();
//        Function *F = CGF->getFunction();
//
//        EXPECT_FALSE(Diags.hasErrorOccurred());
//        testing::internal::CaptureStdout();
//        F->print(llvm::outs());
//        std::string output = testing::internal::GetCapturedStdout();
//
//        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
//                          "entry:\n"
//                          "  %1 = alloca i32, align 4\n"
//                          "  store i32 %0, i32* %1, align 4\n"
//                          "  %2 = load i32, i32* %1, align 4\n"
//                          "  switch i32 %2, label %default [\n"
//                          "    i32 1, label %case\n"
//                          "    i32 2, label %case1\n"
//                          "  ]\n"
//                          "\n"
//                          "case:                                             ; preds = %entry\n"
//                          "  store i32 1, i32* %1, align 4\n"
//                          "  br label %case1\n"
//                          "\n"
//                          "case1:                                            ; preds = %entry, %case\n"
//                          "  store i32 2, i32* %1, align 4\n"
//                          "  br label %endswitch\n"
//                          "\n"
//                          "default:                                          ; preds = %entry\n"
//                          "  store i32 3, i32* %1, align 4\n"
//                          "  br label %endswitch\n"
//                          "\n"
//                          "endswitch:                                        ; preds = %default, %case1\n"
//                          "  ret i32 1\n"
//                          "}\n");
//    }
//
//    TEST_F(CodeGenTest, CGWhileBlock) {
//        ASTNode *Node = CreateNode();
//
//        // main()
//        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);
//        ASTParam *Param = MainFn->addParam(SourceLoc, IntType, "a");
//
//        ASTValueExpr *OneCost = new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 1));
//        ASTVarRefExpr *ARef = new ASTVarRefExpr(new ASTVarRef(Param));
//        ASTGroupExpr *Group = new ASTBinaryGroupExpr(SourceLoc,
//                                                     COMP_EQ,
//                                                     ARef,
//                                                     OneCost);
//
//        ASTWhileBlock *WhileBlock = MainFn->getBody()->AddWhileBlock(SourceLoc, Group);
//
//        ASTVarAssign *A2 = new ASTVarAssign(SourceLoc, WhileBlock, Param);
//        A2->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 1)));
//        WhileBlock->AddVarAssign(A2);
//        WhileBlock->AddContinue(SourceLoc);
//
//        MainFn->getBody()->AddReturn(SourceLoc, OneCost);
//
//        // Add to Node
//        Builder->AddFunction(Node, MainFn);
//
//        // Generate Code
//        CodeGenModule *CGM = Node->getCodeGen();
//        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
//        CGF->GenBody();
//        Function *F = CGF->getFunction();
//
//        EXPECT_FALSE(Diags.hasErrorOccurred());
//        testing::internal::CaptureStdout();
//        F->print(llvm::outs());
//        std::string output = testing::internal::GetCapturedStdout();
//
//        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
//                          "entry:\n"
//                          "  %1 = alloca i32, align 4\n"
//                          "  store i32 %0, i32* %1, align 4\n"
//                          "  br label %whilecond\n"
//                          "\n"
//                          "whilecond:                                        ; preds = %whileloop, %entry\n"
//                          "  %2 = load i32, i32* %1, align 4\n"
//                          "  %3 = icmp eq i32 %2, 1\n"
//                          "  br i1 %3, label %whileloop, label %whileend\n"
//                          "\n"
//                          "whileloop:                                        ; preds = %whilecond\n"
//                          "  store i32 1, i32* %1, align 4\n"
//                          "  br label %whilecond\n"
//                          "\n"
//                          "whileend:                                         ; preds = %whilecond\n"
//                          "  ret i32 1\n"
//                          "}\n");
//    }
//
//    TEST_F(CodeGenTest, CGForInitCondPostBlock) {
//        ASTNode *Node = CreateNode();
//
//        // main()
//        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);
//        ASTParam *Param = MainFn->addParam(SourceLoc, IntType, "a");
//
//        ASTForBlock *ForBlock = MainFn->getBody()->AddForBlock(SourceLoc);
//        ASTValueExpr *OneCost = new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 1));
//
//        // Init
//        ASTLocalVar *InitVar = new ASTLocalVar(SourceLoc, ForBlock, IntType, "i");
//        InitVar->setExpr(OneCost);
//        ForBlock->AddLocalVar(InitVar);
//
//        //Cond
//
//        ASTVarRefExpr *InitVarRef = new ASTVarRefExpr(new ASTVarRef(InitVar));
//        ASTGroupExpr *Cond = new ASTBinaryGroupExpr(SourceLoc, COMP_LTE,
//                                                    InitVarRef, OneCost);
//        ForBlock->setCondition(Cond);
//
//        // Post
//        ASTBlock *PostBlock = ForBlock->getPost();
//        ASTExprStmt *ExprStmt = new ASTExprStmt(SourceLoc, PostBlock);
//        ExprStmt->setExpr(new ASTUnaryGroupExpr(SourceLoc, ARITH_INCR, UNARY_PRE,
//                                                new ASTVarRefExpr(new ASTVarRef(InitVar))));
//        PostBlock->AddExprStmt(ExprStmt);
//
//        ASTVarAssign *A2 = new ASTVarAssign(SourceLoc, ForBlock->getLoop(), Param);
//        A2->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 1)));
//        ForBlock->getLoop()->AddVarAssign(A2);
//        ForBlock->getLoop()->AddContinue(SourceLoc);
//
//        MainFn->getBody()->AddReturn(SourceLoc, OneCost);
//        Node->Resolve();
//
//        // Add to Node
//        Builder->AddFunction(Node, MainFn);
//
//        // Generate Code
//        CodeGenModule *CGM = Node->getCodeGen();
//        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
//        CGF->GenBody();
//        Function *F = CGF->getFunction();
//
//        EXPECT_FALSE(Diags.hasErrorOccurred());
//        testing::internal::CaptureStdout();
//        F->print(llvm::outs());
//        std::string output = testing::internal::GetCapturedStdout();
//
//        EXPECT_EQ(output, "define internal i32 @main(i32 %0) {\n"
//                          "entry:\n"
//                          "  %1 = alloca i32, align 4\n"
//                          "  %2 = alloca i32, align 4\n"
//                          "  store i32 %0, i32* %1, align 4\n"
//                          "  store i32 1, i32* %2, align 4\n"
//                          "  br label %forcond\n"
//                          "\n"
//                          "forcond:                                          ; preds = %forpost, %entry\n"
//                          "  %3 = load i32, i32* %2, align 4\n"
//                          "  %4 = icmp sle i32 %3, 1\n"
//                          "  br i1 %4, label %forloop, label %endfor\n"
//                          "\n"
//                          "forloop:                                          ; preds = %forcond\n"
//                          "  store i32 1, i32* %1, align 4\n"
//                          "  br label %forpost\n"
//                          "\n"
//                          "forpost:                                          ; preds = %forloop\n"
//                          "  %5 = load i32, i32* %2, align 4\n"
//                          "  %6 = add nsw i32 %5, 1\n"
//                          "  store i32 %6, i32* %2, align 4\n"
//                          "  br label %forcond\n"
//                          "\n"
//                          "endfor:                                           ; preds = %forcond\n"
//                          "  ret i32 1\n"
//                          "}\n");
//    }
//
//    TEST_F(CodeGenTest, CGForCondBlock) {
//        ASTNode *Node = CreateNode();
//
//        // main()
//        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);
//        ASTParam *Param = MainFn->addParam(SourceLoc, IntType, "a");
//
//        ASTForBlock *ForBlock = MainFn->getBody()->AddForBlock(SourceLoc);
//        ASTValueExpr *OneCost = new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 1));
//
//        //Cond
//        ASTVarRefExpr *InitVarRef = new ASTVarRefExpr(new ASTVarRef(Param));
//        ASTGroupExpr *Cond = new ASTBinaryGroupExpr(SourceLoc, COMP_LTE,
//                                                    InitVarRef, OneCost);
//        ForBlock->setCondition(Cond);
//
//        ASTVarAssign *A2 = new ASTVarAssign(SourceLoc, ForBlock->getLoop(), Param);
//        A2->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 1)));
//        ForBlock->getLoop()->AddVarAssign(A2);
//        ForBlock->getLoop()->AddContinue(SourceLoc);
//
//        MainFn->getBody()->AddReturn(SourceLoc, OneCost);
//
//        // Add to Node
//        Builder->AddFunction(Node, MainFn);
//
//        // Generate Code
//        CodeGenModule *CGM = Node->getCodeGen();
//        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
//        CGF->GenBody();
//        Function *F = CGF->getFunction();
//
//        EXPECT_FALSE(Diags.hasErrorOccurred());
//        testing::internal::CaptureStdout();
//        F->print(llvm::outs());
//        std::string output = testing::internal::GetCapturedStdout();
//
//        EXPECT_EQ(output, "define internal i32 @main(i32 %0) {\n"
//                          "entry:\n"
//                          "  %1 = alloca i32, align 4\n"
//                          "  store i32 %0, i32* %1, align 4\n"
//                          "  br label %forcond\n"
//                          "\n"
//                          "forcond:                                          ; preds = %forloop, %entry\n"
//                          "  %2 = load i32, i32* %1, align 4\n"
//                          "  %3 = icmp sle i32 %2, 1\n"
//                          "  br i1 %3, label %forloop, label %endfor\n"
//                          "\n"
//                          "forloop:                                          ; preds = %forcond\n"
//                          "  store i32 1, i32* %1, align 4\n"
//                          "  br label %forcond\n"
//                          "\n"
//                          "endfor:                                           ; preds = %forcond\n"
//                          "  ret i32 1\n"
//                          "}\n");
//    }
//
//    TEST_F(CodeGenTest, CGForPostBlock) {
//        ASTNode *Node = CreateNode();
//
//        // main()
//        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", VisibilityKind::V_DEFAULT);
//        ASTParam *Param = MainFn->addParam(SourceLoc, IntType, "a");
//
//        ASTForBlock *ForBlock = MainFn->getBody()->AddForBlock(SourceLoc);
//        ASTValueExpr *OneCost = new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 1));
//
//        // Post
//        ASTBlock *PostBlock = ForBlock->getPost();
//        ASTExprStmt *ExprStmt = new ASTExprStmt(SourceLoc, PostBlock);
//        ExprStmt->setExpr(new ASTUnaryGroupExpr(SourceLoc, ARITH_INCR, UNARY_PRE,
//                                                new ASTVarRefExpr(new ASTVarRef(Param))));
//        PostBlock->AddExprStmt(ExprStmt);
//
//        ASTVarAssign *A2 = new ASTVarAssign(SourceLoc, ForBlock->getLoop(), Param);
//        A2->setExpr(new ASTValueExpr(new ASTIntegerValue(SourceLoc, IntType, 1)));
//        ForBlock->getLoop()->AddVarAssign(A2);
//        ForBlock->getLoop()->AddContinue(SourceLoc);
//
//        MainFn->getBody()->AddReturn(SourceLoc, OneCost);
//
//        // Add to Node
//        Builder->AddFunction(Node, MainFn);
//
//        // Generate Code
//        CodeGenModule *CGM = Node->getCodeGen();
//        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
//        CGF->GenBody();
//        Function *F = CGF->getFunction();
//
//        EXPECT_FALSE(Diags.hasErrorOccurred());
//        testing::internal::CaptureStdout();
//        F->print(llvm::outs());
//        std::string output = testing::internal::GetCapturedStdout();
//
//        EXPECT_EQ(output, "define internal i32 @main(i32 %0) {\n"
//                          "entry:\n"
//                          "  %1 = alloca i32, align 4\n"
//                          "  store i32 %0, i32* %1, align 4\n"
//                          "  br label %forloop\n"
//                          "\n"
//                          "forloop:                                          ; preds = %forpost, %entry\n"
//                          "  store i32 1, i32* %1, align 4\n"
//                          "  br label %forpost\n"
//                          "\n"
//                          "forpost:                                          ; preds = %forloop\n"
//                          "  %2 = load i32, i32* %1, align 4\n"
//                          "  %3 = add nsw i32 %2, 1\n"
//                          "  store i32 %3, i32* %1, align 4\n"
//                          "  br label %forloop\n"
//                          "\n"
//                          "endfor:                                           ; No predecessors!\n"
//                          "  ret i32 1\n"
//                          "}\n");
//    }

} // anonymous namespace
