//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

// fly
#include "TestUtils.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenGlobalVar.h"
#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGenClassFunction.h"
#include "CodeGen/CodeGenClassVar.h"
#include "Sema/Sema.h"
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
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"
#include "Basic/Diagnostic.h"
#include "Basic/SourceLocation.h"
#include "Basic/TargetOptions.h"
#include "Basic/Builtins.h"

// third party
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "gtest/gtest.h"

// standard
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
                Builder(Sema::CreateBuilder(CI.getDiagnostics())),
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

        ASTTopScopes *TopScopes = SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false);

        // default Bool value
        ASTValue *DefaultBoolVal = SemaBuilder::CreateDefaultValue(BoolType);
        ASTGlobalVar *aVar = Builder->CreateGlobalVar(Node, SourceLoc, BoolType, "a", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, aVar, DefaultBoolVal));

        // default Byte value
        ASTValue *DefaultByteVal = SemaBuilder::CreateDefaultValue(ByteType);
        ASTGlobalVar *bVar = Builder->CreateGlobalVar(Node, SourceLoc, ByteType, "b", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, bVar, DefaultByteVal));

        // default Short value
        ASTValue *DefaultShortVal = SemaBuilder::CreateDefaultValue(ShortType);
        ASTGlobalVar *cVar = Builder->CreateGlobalVar(Node, SourceLoc, ShortType, "c", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, cVar, DefaultShortVal));

        // default UShort value
        ASTValue *DefaultUShortVal = SemaBuilder::CreateDefaultValue(UShortType);
        ASTGlobalVar *dVar = Builder->CreateGlobalVar(Node, SourceLoc, UShortType, "d", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, dVar, DefaultUShortVal));

        // default Int value
        ASTValue *DefaultIntVal = SemaBuilder::CreateDefaultValue(IntType);
        ASTGlobalVar *eVar = Builder->CreateGlobalVar(Node, SourceLoc, IntType, "e", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, eVar, DefaultIntVal));

        // default UInt value
        ASTValue *DefaultUintVal = SemaBuilder::CreateDefaultValue(UIntType);
        ASTGlobalVar *fVar = Builder->CreateGlobalVar(Node, SourceLoc, UIntType, "f", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, fVar, DefaultUintVal));

        // default Long value
        ASTValue *DefaultLongVal = SemaBuilder::CreateDefaultValue(LongType);
        ASTGlobalVar *gVar = Builder->CreateGlobalVar(Node, SourceLoc, LongType, "g", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, gVar, DefaultLongVal));

        // default ULong value
        ASTValue *DefaultULongVal = SemaBuilder::CreateDefaultValue(ULongType);
        ASTGlobalVar *hVar = Builder->CreateGlobalVar(Node, SourceLoc, ULongType, "h", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, hVar, DefaultULongVal));

        // default Float value
        ASTValue *DefaultFloatVal = SemaBuilder::CreateDefaultValue(FloatType);
        ASTGlobalVar *iVar = Builder->CreateGlobalVar(Node, SourceLoc, FloatType, "i", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, iVar, DefaultFloatVal));

        // default Double value
        ASTValue *DefaultDoubleVal = SemaBuilder::CreateDefaultValue(DoubleType);
        ASTGlobalVar *jVar = Builder->CreateGlobalVar(Node, SourceLoc, DoubleType, "j", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node,jVar, DefaultDoubleVal));

        // default Array value
        ASTValue *DefaultArrayVal = SemaBuilder::CreateDefaultValue(ArrayInt0Type);
        ASTGlobalVar *kVar = Builder->CreateGlobalVar(Node, SourceLoc, ArrayInt0Type, "k", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, kVar, DefaultArrayVal));

        // Generate Code
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
        ASTNode *Node = CreateNode();

        ASTTopScopes *TopScopes = SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false);

        // a
        ASTBoolValue *BoolVal = SemaBuilder::CreateBoolValue(SourceLoc, true);
        ASTGlobalVar *aVar = Builder->CreateGlobalVar(Node, SourceLoc, BoolType, "a", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, aVar, BoolVal));

        // b
        ASTIntegerValue *ByteVal = SemaBuilder::CreateIntegerValue(SourceLoc, 1);
        ASTGlobalVar *bVar = Builder->CreateGlobalVar(Node, SourceLoc, ByteType, "b", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, bVar, ByteVal));

        // c
        ASTIntegerValue *ShortVal = SemaBuilder::CreateIntegerValue(SourceLoc, -2);
        ASTGlobalVar *cVar = Builder->CreateGlobalVar(Node, SourceLoc, ShortType, "c", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, cVar, ShortVal));

        // d
        ASTIntegerValue *UShortVal = SemaBuilder::CreateIntegerValue(SourceLoc, 2);
        ASTGlobalVar *dVar = Builder->CreateGlobalVar(Node, SourceLoc, UShortType, "d", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, dVar, UShortVal));

        // e
        ASTIntegerValue *IntVal = SemaBuilder::CreateIntegerValue(SourceLoc, -3);
        ASTGlobalVar *eVar = Builder->CreateGlobalVar(Node, SourceLoc, IntType, "e", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, eVar, IntVal));

        // f
        ASTIntegerValue *UIntVal = SemaBuilder::CreateIntegerValue(SourceLoc, 3);
        ASTGlobalVar *fVar = Builder->CreateGlobalVar(Node, SourceLoc, UIntType, "f", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, fVar, UIntVal));

        // g
        ASTIntegerValue *LongVal = SemaBuilder::CreateIntegerValue(SourceLoc, -4);
        ASTGlobalVar *gVar = Builder->CreateGlobalVar(Node, SourceLoc, LongType, "g", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, gVar, LongVal));

        // h
        ASTIntegerValue *ULongVal = SemaBuilder::CreateIntegerValue(SourceLoc, 4);
        ASTGlobalVar *hVar = Builder->CreateGlobalVar(Node, SourceLoc, ULongType, "h", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, hVar, ULongVal));

        // i
        ASTFloatingValue *FloatVal = SemaBuilder::CreateFloatingValue(SourceLoc, 1.5);
        ASTGlobalVar *iVar = Builder->CreateGlobalVar(Node, SourceLoc, FloatType, "i", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, iVar, FloatVal));

        // j
        ASTFloatingValue *DoubleVal = SemaBuilder::CreateFloatingValue(SourceLoc, 2.5);
        ASTGlobalVar *jVar = Builder->CreateGlobalVar(Node, SourceLoc, DoubleType, "j", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node,jVar, DoubleVal));

        // k (empty array)
        ASTArrayValue *ArrayValEmpty = SemaBuilder::CreateArrayValue(SourceLoc);
        ASTGlobalVar *kVar = Builder->CreateGlobalVar(Node, SourceLoc, ArrayInt0Type, "k", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, kVar, ArrayValEmpty));

        // l (2 val)
        ASTArrayValue *ArrayVal = SemaBuilder::CreateArrayValue(SourceLoc);
        ASTArrayType *ArrayInt2Type = SemaBuilder::CreateArrayType(SourceLoc, IntType,
          Builder->CreateExpr(nullptr, SemaBuilder::CreateIntegerValue(SourceLoc, 2)));
        Builder->AddArrayValue(ArrayVal, SemaBuilder::CreateIntegerValue(SourceLoc, 1)); // ArrayVal = {1}
        Builder->AddArrayValue(ArrayVal, SemaBuilder::CreateIntegerValue(SourceLoc, 2)); // ArrayVal = {1, 1}
        ASTGlobalVar *lVar = Builder->CreateGlobalVar(Node, SourceLoc, ArrayInt2Type, "l", TopScopes);
        EXPECT_TRUE(Builder->AddGlobalVar(Node, lVar, ArrayVal));

        // Generate Code
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
    }

    TEST_F(CodeGenTest, CGFunc) {
        ASTNode *Node = CreateNode();

        ASTTopScopes *TopScopes = SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false);

        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", TopScopes);
        EXPECT_TRUE(Builder->AddParam(Builder->CreateParam(MainFn, SourceLoc, IntType, "P1", false)));
        EXPECT_TRUE(Builder->AddParam(Builder->CreateParam(MainFn, SourceLoc, FloatType, "P2", false)));
        EXPECT_TRUE(Builder->AddParam(Builder->CreateParam(MainFn, SourceLoc, BoolType, "P3", false)));
        EXPECT_TRUE(Builder->AddParam(Builder->CreateParam(MainFn, SourceLoc, LongType, "P4", false)));
        EXPECT_TRUE(Builder->AddParam(Builder->CreateParam(MainFn, SourceLoc, DoubleType, "P5", false)));
        EXPECT_TRUE(Builder->AddParam(Builder->CreateParam(MainFn, SourceLoc, ByteType, "P6", false)));
        EXPECT_TRUE(Builder->AddParam(Builder->CreateParam(MainFn, SourceLoc, ShortType, "P7", false)));
        EXPECT_TRUE(Builder->AddParam(Builder->CreateParam(MainFn, SourceLoc, UShortType, "P8", false)));
        EXPECT_TRUE(Builder->AddParam(Builder->CreateParam(MainFn, SourceLoc, UIntType, "P9", false)));
        EXPECT_TRUE(Builder->AddParam(Builder->CreateParam(MainFn, SourceLoc, ULongType, "P10", false)));
        ASTBlock *Body = Builder->getBlock(MainFn);
        
        ASTIntegerValue *IntVal = SemaBuilder::CreateIntegerValue(SourceLoc, 1);
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, IntVal);
        EXPECT_TRUE(Builder->AddStmt(Return));

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
        ASTGlobalVar *GVar = Builder->CreateGlobalVar(Node, SourceLoc, FloatType, "G",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_PRIVATE, false));
        EXPECT_TRUE(Builder->AddGlobalVar(Node, GVar, FloatingVal));

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);

        // int A
        ASTLocalVar *VarA = Builder->CreateLocalVar(Body, SourceLoc, IntType, "A", false);
        EXPECT_TRUE(Builder->AddStmt(VarA));
        
        // A = 1
        ASTVarAssign * VarAAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(VarA));
        Builder->CreateExpr(VarAAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder->AddStmt(VarAAssign));

        // GlobalVar
        // G = 1
        ASTVarRef *VarRefG = Builder->CreateVarRef(GVar);
        ASTVarAssign * GVarAssign = Builder->CreateVarAssign(Body, VarRefG);
        Builder->CreateExpr(GVarAssign, SemaBuilder::CreateFloatingValue(SourceLoc, 1));
        EXPECT_TRUE(Builder->AddStmt(GVarAssign));

        // return A
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(VarA));
        EXPECT_TRUE(Builder->AddStmt(Return));
        
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
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);

        // test()
        ASTFunction *TestFn = Builder->CreateFunction(Node, SourceLoc, IntType, "test",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));

        // call test()
        ASTExprStmt *ExprStmt = Builder->CreateExprStmt(Body, SourceLoc);
        ASTCall *TestCall = Builder->CreateCall(TestFn);
        Builder->CreateExpr(ExprStmt, TestCall);
        EXPECT_TRUE(Builder->AddStmt(ExprStmt));

        //return test()
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, TestCall);
        EXPECT_TRUE(Builder->AddStmt(Return));

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
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);
        ASTParam *aParam = Builder->CreateParam(MainFn, SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder->AddParam(aParam));
        ASTParam *bParam = Builder->CreateParam(MainFn, SourceLoc, IntType, "b");
        EXPECT_TRUE(Builder->AddParam(bParam));
        ASTParam *cParam = Builder->CreateParam(MainFn, SourceLoc, IntType, "c");
        EXPECT_TRUE(Builder->AddParam(cParam));

        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        // Create this expression: 1 + a * b / (c - 2)
        // E1 + (E2 * E3) / (E4 - E5)
        // E1 + (G2 / G3)
        // E1 + G1
        ASTValueExpr *E1 = Builder->CreateExpr(Return, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *E2 = Builder->CreateExpr(Return, Builder->CreateVarRef(aParam));
        ASTVarRefExpr *E3 = Builder->CreateExpr(Return, Builder->CreateVarRef(bParam));
        ASTVarRefExpr *E4 = Builder->CreateExpr(Return, Builder->CreateVarRef(cParam));
        ASTValueExpr *E5 = Builder->CreateExpr(Return, SemaBuilder::CreateIntegerValue(SourceLoc, 2));

        ASTBinaryGroupExpr *G2 = Builder->CreateBinaryExpr(Return, SourceLoc, ASTBinaryOperatorKind::ARITH_MUL, E2, E3);
        ASTBinaryGroupExpr *G3 = Builder->CreateBinaryExpr(Return, SourceLoc, ASTBinaryOperatorKind::ARITH_SUB, E4, E5);
        ASTBinaryGroupExpr *G1 = Builder->CreateBinaryExpr(Return, SourceLoc, ASTBinaryOperatorKind::ARITH_DIV, G2, G3);
        ASTBinaryGroupExpr *Group = Builder->CreateBinaryExpr(Return, SourceLoc, ASTBinaryOperatorKind::ARITH_ADD, E1, G1);

        EXPECT_TRUE(Builder->AddStmt(Return));

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
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);
        ASTParam *aParam = Builder->CreateParam(MainFn, SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder->AddParam(aParam));
        ASTParam *bParam = Builder->CreateParam(MainFn, SourceLoc, IntType, "b");
        EXPECT_TRUE(Builder->AddParam(bParam));
        ASTParam *cParam = Builder->CreateParam(MainFn, SourceLoc, IntType, "c");
        EXPECT_TRUE(Builder->AddParam(cParam));

        // a = 0
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 0));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign));

        // b = 0
        ASTVarAssign *bVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(bParam));
        Builder->CreateExpr(bVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 0));
        EXPECT_TRUE(Builder->AddStmt(bVarAssign));

        // c = a + b
        ASTVarAssign * cAddVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cAddVarAssign, SourceLoc, ASTBinaryOperatorKind::ARITH_ADD,
                                  Builder->CreateExpr(cAddVarAssign, Builder->CreateVarRef(aParam)),
                                  Builder->CreateExpr(cAddVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(cAddVarAssign));

        // c = a - b
        ASTVarAssign * cSubVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cSubVarAssign, SourceLoc, ASTBinaryOperatorKind::ARITH_SUB,
                                  Builder->CreateExpr(cSubVarAssign, Builder->CreateVarRef(aParam)),
                                  Builder->CreateExpr(cSubVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(cSubVarAssign));

        // c = a * b
        ASTVarAssign * cMulVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cMulVarAssign, SourceLoc, ASTBinaryOperatorKind::ARITH_MUL,
                                  Builder->CreateExpr(cMulVarAssign, Builder->CreateVarRef(aParam)),
                                  Builder->CreateExpr(cMulVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(cMulVarAssign));

        // c = a / b
        ASTVarAssign * cDivVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cDivVarAssign, SourceLoc, ASTBinaryOperatorKind::ARITH_DIV,
                                  Builder->CreateExpr(cDivVarAssign, Builder->CreateVarRef(aParam)),
                                  Builder->CreateExpr(cDivVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(cDivVarAssign));

        // c = a % b
        ASTVarAssign * cModVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cModVarAssign, SourceLoc, ASTBinaryOperatorKind::ARITH_MOD,
                                  Builder->CreateExpr(cModVarAssign, Builder->CreateVarRef(aParam)),
                                  Builder->CreateExpr(cModVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(cModVarAssign));

        // c = a & b
        ASTVarAssign * cAndVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cAndVarAssign, SourceLoc, ASTBinaryOperatorKind::ARITH_AND,
                                  Builder->CreateExpr(cAndVarAssign, Builder->CreateVarRef(aParam)),
                                  Builder->CreateExpr(cAndVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(cAndVarAssign));

        // c = a | b
        ASTVarAssign * cOrVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cOrVarAssign, SourceLoc, ASTBinaryOperatorKind::ARITH_OR,
                                  Builder->CreateExpr(cOrVarAssign, Builder->CreateVarRef(aParam)),
                                  Builder->CreateExpr(cOrVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(cOrVarAssign));

        // c = a xor b
        ASTVarAssign * cXorVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cXorVarAssign, SourceLoc, ASTBinaryOperatorKind::ARITH_XOR,
                                  Builder->CreateExpr(cXorVarAssign, Builder->CreateVarRef(aParam)),
                                  Builder->CreateExpr(cXorVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(cXorVarAssign));

        // c = a << b
        ASTVarAssign * cShlVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cShlVarAssign, SourceLoc, ASTBinaryOperatorKind::ARITH_SHIFT_L,
                                  Builder->CreateExpr(cShlVarAssign, Builder->CreateVarRef(aParam)),
                                  Builder->CreateExpr(cShlVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(cShlVarAssign));

        // c = a >> b
        ASTVarAssign * cShrVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cParam));
        Builder->CreateBinaryExpr(cShrVarAssign, SourceLoc, ASTBinaryOperatorKind::ARITH_SHIFT_R,
                                  Builder->CreateExpr(cShrVarAssign, Builder->CreateVarRef(aParam)),
                                  Builder->CreateExpr(cShrVarAssign, Builder->CreateVarRef(bParam)));
        EXPECT_TRUE(Builder->AddStmt(cShrVarAssign));

        // ++c
        ASTExprStmt *cPreIncVarAssign = Builder->CreateExprStmt(Body, SourceLoc);
        Builder->CreateUnaryExpr(cPreIncVarAssign, SourceLoc, ASTUnaryOperatorKind::ARITH_INCR, ASTUnaryOptionKind::UNARY_PRE,
                                 Builder->CreateExpr(cPreIncVarAssign, Builder->CreateVarRef(cParam)));
        EXPECT_TRUE(Builder->AddStmt(cPreIncVarAssign));

        // c++
        ASTExprStmt *cPostIncVarAssign = Builder->CreateExprStmt(Body, SourceLoc);
        Builder->CreateUnaryExpr(cPostIncVarAssign, SourceLoc, ASTUnaryOperatorKind::ARITH_INCR, ASTUnaryOptionKind::UNARY_POST,
                                 Builder->CreateExpr(cPostIncVarAssign, Builder->CreateVarRef(cParam)));
        EXPECT_TRUE(Builder->AddStmt(cPostIncVarAssign));

        // ++c
        ASTExprStmt *cPreDecVarAssign = Builder->CreateExprStmt(Body, SourceLoc);
        Builder->CreateUnaryExpr(cPreDecVarAssign, SourceLoc, ASTUnaryOperatorKind::ARITH_DECR, ASTUnaryOptionKind::UNARY_PRE,
                                 Builder->CreateExpr(cPreDecVarAssign, Builder->CreateVarRef(cParam)));
        EXPECT_TRUE(Builder->AddStmt(cPreDecVarAssign));

        // c++
        ASTExprStmt *cPostDecVarAssign = Builder->CreateExprStmt(Body, SourceLoc);
        Builder->CreateUnaryExpr(cPostDecVarAssign, SourceLoc, ASTUnaryOperatorKind::ARITH_DECR, ASTUnaryOptionKind::UNARY_POST,
                                 Builder->CreateExpr(cPostDecVarAssign, Builder->CreateVarRef(cParam)));
        EXPECT_TRUE(Builder->AddStmt(cPostDecVarAssign));

        //return c
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(cParam));
        EXPECT_TRUE(Builder->AddStmt(Return));

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
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, BoolType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);
        ASTLocalVar *aVar = Builder->CreateLocalVar(Body, SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder->AddStmt(aVar));
        ASTLocalVar *bVar = Builder->CreateLocalVar(Body, SourceLoc, IntType, "b");
        EXPECT_TRUE(Builder->AddStmt(bVar));
        ASTLocalVar *cVar = Builder->CreateLocalVar(Body, SourceLoc, BoolType, "c");
        EXPECT_TRUE(Builder->AddStmt(cVar));

        // a = 0
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(aVar));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 0));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign));

        // b = 0
        ASTVarAssign *bVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(bVar));
        Builder->CreateExpr(bVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 0));
        EXPECT_TRUE(Builder->AddStmt(bVarAssign));

        // Operation Equal
        ASTVarAssign * cEqVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cEqVarAssign, SourceLoc, ASTBinaryOperatorKind::COMP_EQ,
                                  Builder->CreateExpr(cEqVarAssign, Builder->CreateVarRef(aVar)),
                                  Builder->CreateExpr(cEqVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(cEqVarAssign));

        // Operation Not Equal
        ASTVarAssign * cNeqVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cNeqVarAssign, SourceLoc, ASTBinaryOperatorKind::COMP_NE,
                                  Builder->CreateExpr(cNeqVarAssign, Builder->CreateVarRef(aVar)),
                                  Builder->CreateExpr(cNeqVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(cNeqVarAssign));

        // Operation Greater Than
        ASTVarAssign * cGtVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cGtVarAssign, SourceLoc, ASTBinaryOperatorKind::COMP_GT,
                                  Builder->CreateExpr(cGtVarAssign, Builder->CreateVarRef(aVar)),
                                  Builder->CreateExpr(cGtVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(cGtVarAssign));

        // Operation Greater Than or Equal
        ASTVarAssign * cGteVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cGteVarAssign, SourceLoc, ASTBinaryOperatorKind::COMP_GTE,
                                  Builder->CreateExpr(cGteVarAssign, Builder->CreateVarRef(aVar)),
                                  Builder->CreateExpr(cGteVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(cGteVarAssign));

        // Operation Less Than
        ASTVarAssign * cLtVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cLtVarAssign, SourceLoc, ASTBinaryOperatorKind::COMP_LT,
                                  Builder->CreateExpr(cLtVarAssign, Builder->CreateVarRef(aVar)),
                                  Builder->CreateExpr(cLtVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(cLtVarAssign));

        // Operation Less Than or Equal
        ASTVarAssign * cLteVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cLteVarAssign, SourceLoc, ASTBinaryOperatorKind::COMP_LTE,
                                  Builder->CreateExpr(cLteVarAssign, Builder->CreateVarRef(aVar)),
                                  Builder->CreateExpr(cLteVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(cLteVarAssign));

        //return c
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(cVar));
        EXPECT_TRUE(Builder->AddStmt(Return));

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

        EXPECT_EQ(output, "define i1 @main() {\n"
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
                          "  ret i1 %18\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGLogicOp) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, BoolType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);
        ASTLocalVar *aVar = Builder->CreateLocalVar(Body, SourceLoc, BoolType, "a");
        EXPECT_TRUE(Builder->AddStmt(aVar));
        ASTLocalVar *bVar = Builder->CreateLocalVar(Body, SourceLoc, BoolType, "b");
        EXPECT_TRUE(Builder->AddStmt(bVar));
        ASTLocalVar *cVar = Builder->CreateLocalVar(Body, SourceLoc, BoolType, "c");
        EXPECT_TRUE(Builder->AddStmt(cVar));

        // a = false
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(aVar));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign));

        // b = false
        ASTVarAssign *bVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(bVar));
        Builder->CreateExpr(bVarAssign, SemaBuilder::CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder->AddStmt(bVarAssign));

        // Operation And Logic
        ASTVarAssign * cAndVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cAndVarAssign, SourceLoc, ASTBinaryOperatorKind::LOGIC_AND,
                                  Builder->CreateExpr(cAndVarAssign, Builder->CreateVarRef(aVar)),
                                  Builder->CreateExpr(cAndVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(cAndVarAssign));

        // Operation Or Logic
        ASTVarAssign * cOrVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cVar));
        Builder->CreateBinaryExpr(cOrVarAssign, SourceLoc, ASTBinaryOperatorKind::LOGIC_OR,
                                  Builder->CreateExpr(cOrVarAssign, Builder->CreateVarRef(aVar)),
                                  Builder->CreateExpr(cOrVarAssign, Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(cOrVarAssign));

        //return c
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(cVar));
        EXPECT_TRUE(Builder->AddStmt(Return));

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

        EXPECT_EQ(output, "define i1 @main() {\n"
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
                          "  ret i1 %16\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGTernaryOp) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);
        ASTLocalVar *aVar = Builder->CreateLocalVar(Body, SourceLoc, BoolType, "a");
        EXPECT_TRUE(Builder->AddStmt(aVar));
        ASTLocalVar *bVar = Builder->CreateLocalVar(Body, SourceLoc, BoolType, "b");
        EXPECT_TRUE(Builder->AddStmt(bVar));
        ASTLocalVar *cVar = Builder->CreateLocalVar(Body, SourceLoc, IntType, "c");
        EXPECT_TRUE(Builder->AddStmt(cVar));

        // a = false
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(aVar));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign));

        // b = false
        ASTVarAssign *bVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(bVar));
        Builder->CreateExpr(bVarAssign, SemaBuilder::CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder->AddStmt(bVarAssign));

        // return
        ASTVarAssign * cOrVarAssign = Builder->CreateVarAssign(Body, Builder->CreateVarRef(cVar));
        ASTBinaryGroupExpr *Cond = Builder->CreateBinaryExpr(cOrVarAssign, SourceLoc, ASTBinaryOperatorKind::COMP_EQ,
                                                             Builder->CreateExpr(cOrVarAssign,
                                                                                 Builder->CreateVarRef(aVar)),
                                                             Builder->CreateExpr(cOrVarAssign,
                                                                                 Builder->CreateVarRef(bVar)));
        Builder->CreateTernaryExpr(cOrVarAssign, Cond, SourceLoc,
                                   Builder->CreateExpr(cOrVarAssign,Builder->CreateVarRef(aVar)),
                                   SourceLoc,
                                   Builder->CreateExpr(cOrVarAssign,Builder->CreateVarRef(bVar)));
        EXPECT_TRUE(Builder->AddStmt(cOrVarAssign));

        //return c
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(cVar));
        EXPECT_TRUE(Builder->AddStmt(Return));

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

    TEST_F(CodeGenTest, CGIfBlock) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);
        ASTLocalVar *aVar = Builder->CreateLocalVar(Body, SourceLoc, IntType, "a");
        Builder->CreateExpr(aVar, SemaBuilder::CreateIntegerValue(SourceLoc, 0));
        EXPECT_TRUE(Builder->AddStmt(aVar));

        // Create if block
        ASTIfBlock *IfBlock = Builder->CreateIfBlock(Body, SourceLoc);

        // if (a == 1)
        ASTVarRefExpr *aVarRef = Builder->CreateExpr(IfBlock, Builder->CreateVarRef(aVar));
        ASTValueExpr *Value1 = Builder->CreateExpr(IfBlock, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        ASTBinaryGroupExpr *IfCond = Builder->CreateBinaryExpr(IfBlock, SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value1);

        // { a = 2 }
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(IfBlock, Builder->CreateVarRef(aVar));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 2));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign));
        EXPECT_TRUE(Builder->AddStmt(IfBlock));

        // return a
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(aVar));
        EXPECT_TRUE(Builder->AddStmt(Return));

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
                          "  store i32 0, i32* %0, align 4\n"
                          "  %1 = load i32, i32* %0, align 4\n"
                          "  %2 = icmp eq i32 %1, 1\n"
                          "  br i1 %2, label %ifthen, label %endif\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 2, i32* %0, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "endif:                                            ; preds = %ifthen, %entry\n"
                          "  %3 = load i32, i32* %0, align 4\n"
                          "  ret i32 %3\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGIfElseBlock) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main", 
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);
        ASTParam *aParam = Builder->CreateParam(MainFn, SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder->AddParam(aParam));

        // Create if block
        ASTIfBlock *IfBlock = Builder->CreateIfBlock(Body, SourceLoc);

        // if (a == 1)
        ASTValueExpr *Value1 = Builder->CreateExpr(aParam, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder->CreateExpr(IfBlock, Builder->CreateVarRef(aParam));
        ASTBinaryGroupExpr *IfCond = Builder->CreateBinaryExpr(IfBlock, SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value1);

        // { a = 1 }
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(IfBlock, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign));

        // else {a == 2}
        ASTElseBlock *ElseBlock = Builder->CreateElseBlock(IfBlock, SourceLoc);
        ASTVarAssign *aVarAssign2 = Builder->CreateVarAssign(ElseBlock, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign2, SemaBuilder::CreateIntegerValue(SourceLoc, 2));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign2));
        EXPECT_TRUE(Builder->AddStmt(IfBlock));

        // return a
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(aParam));
        EXPECT_TRUE(Builder->AddStmt(Return));

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
                          "  %3 = icmp eq i32 %2, 1\n"
                          "  br i1 %3, label %ifthen, label %else\n"
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
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  ret i32 %4\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGIfElsifElseBlock) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);
        ASTParam *aParam = Builder->CreateParam(MainFn, SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder->AddParam(aParam));

        // Create if block
        ASTIfBlock *IfBlock = Builder->CreateIfBlock(Body, SourceLoc);

        // if (a == 1) { a = 11 }
        ASTValueExpr *Value1 = Builder->CreateExpr(aParam, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder->CreateExpr(IfBlock, Builder->CreateVarRef(aParam));
        ASTBinaryGroupExpr *IfCond = Builder->CreateBinaryExpr(IfBlock, SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value1);
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(IfBlock, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 11));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign));

        // elsif (a == 2) { a = 22 }
        ASTElsifBlock *ElsifBlock = Builder->CreateElsifBlock(IfBlock, SourceLoc);
        ASTValueExpr *Value2 = Builder->CreateExpr(aParam, SemaBuilder::CreateIntegerValue(SourceLoc, 2));
        ASTBinaryGroupExpr *ElsifCond = Builder->CreateBinaryExpr(ElsifBlock, SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value2);
        ASTVarAssign *aVarAssign2 = Builder->CreateVarAssign(ElsifBlock, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign2, SemaBuilder::CreateIntegerValue(SourceLoc, 22));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign2));

        // elsif (a == 3) { a = 33 }
        ASTElsifBlock *ElsifBlock2 = Builder->CreateElsifBlock(IfBlock, SourceLoc);
        ASTValueExpr *Value3 = Builder->CreateExpr(aParam, SemaBuilder::CreateIntegerValue(SourceLoc, 3));
        ASTBinaryGroupExpr *ElsifCond2 = Builder->CreateBinaryExpr(ElsifBlock2, SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value3);
        ASTVarAssign *aVarAssign3 = Builder->CreateVarAssign(ElsifBlock2, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign3, SemaBuilder::CreateIntegerValue(SourceLoc, 33));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign3));

        // else {a == 44}
        ASTElseBlock *ElseBlock = Builder->CreateElseBlock(IfBlock, SourceLoc);
        ASTVarAssign *aVarAssign4 = Builder->CreateVarAssign(ElseBlock, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign4, SemaBuilder::CreateIntegerValue(SourceLoc, 44));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign4));
        EXPECT_TRUE(Builder->AddStmt(IfBlock));

        // return a
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(aParam));
        EXPECT_TRUE(Builder->AddStmt(Return));

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
                          "  %3 = icmp eq i32 %2, 1\n"
                          "  br i1 %3, label %ifthen, label %elsif\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 11, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif:                                            ; preds = %entry\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  %5 = icmp eq i32 %4, 2\n"
                          "  br i1 %5, label %elsifthen, label %elsif1\n"
                          "\n"
                          "elsifthen:                                        ; preds = %elsif\n"
                          "  store i32 22, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif1:                                           ; preds = %elsif\n"
                          "  %6 = load i32, i32* %1, align 4\n"
                          "  %7 = icmp eq i32 %6, 3\n"
                          "  br i1 %7, label %elsifthen2, label %else\n"
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
                          "  %8 = load i32, i32* %1, align 4\n"
                          "  ret i32 %8\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGIfElsifBlock) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);
        ASTParam *aParam = Builder->CreateParam(MainFn, SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder->AddParam(aParam));

        // Create if block
        ASTIfBlock *IfBlock = Builder->CreateIfBlock(Body, SourceLoc);
        EXPECT_TRUE(Builder->AddStmt(IfBlock));

        // if a == 1 { a = 11 }
        ASTValueExpr *Value1 = Builder->CreateExpr(aParam, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder->CreateExpr(IfBlock, Builder->CreateVarRef(aParam));
        ASTBinaryGroupExpr *IfCond = Builder->CreateBinaryExpr(IfBlock, SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value1);
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(IfBlock, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 11));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign));

        // elsif a == 2 { a = 22 }
        ASTElsifBlock *ElsifBlock = Builder->CreateElsifBlock(IfBlock, SourceLoc);
        ASTValueExpr *Value2 = Builder->CreateExpr(aParam, SemaBuilder::CreateIntegerValue(SourceLoc, 2));
        ASTBinaryGroupExpr *ElsifCond = Builder->CreateBinaryExpr(ElsifBlock, SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value2);
        ASTVarAssign *aVarAssign2 = Builder->CreateVarAssign(ElsifBlock, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign2, SemaBuilder::CreateIntegerValue(SourceLoc, 22));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign2));

        // elsif a == 3 { a = 33 }
        ASTElsifBlock *ElsifBlock2 = Builder->CreateElsifBlock(IfBlock, SourceLoc);
        ASTValueExpr *Value3 = Builder->CreateExpr(aParam, SemaBuilder::CreateIntegerValue(SourceLoc, 3));
        ASTBinaryGroupExpr *ElsifCond2 = Builder->CreateBinaryExpr(ElsifBlock2, SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value3);
        ASTVarAssign *aVarAssign3 = Builder->CreateVarAssign(ElsifBlock2, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign3, SemaBuilder::CreateIntegerValue(SourceLoc, 33));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign3));

        // return a
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(aParam));
        EXPECT_TRUE(Builder->AddStmt(Return));

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
                          "entry:\n  %1 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %1, align 4\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  %3 = icmp eq i32 %2, 1\n"
                          "  br i1 %3, label %ifthen, label %elsif\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 11, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\nelsif:                                            ; preds = %entry\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  %5 = icmp eq i32 %4, 2\n"
                          "  br i1 %5, label %elsifthen, label %elsif1\n"
                          "\n"
                          "elsifthen:                                        ; preds = %elsif\n"
                          "  store i32 22, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif1:                                           ; preds = %elsif\n"
                          "  %6 = load i32, i32* %1, align 4\n"
                          "  %7 = icmp eq i32 %6, 3\n"
                          "  br i1 %7, label %elsifthen2, label %endif\n"
                          "\n"
                          "elsifthen2:                                       ; preds = %elsif1\n"
                          "  store i32 33, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "endif:                                            ; preds = %elsifthen2, %elsif1, %elsifthen, %ifthen\n"
                          "  %8 = load i32, i32* %1, align 4\n"
                          "  ret i32 %8\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGSwitchBlock) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);
        ASTParam *aParam = Builder->CreateParam(MainFn, SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder->AddParam(aParam));

        ASTSwitchBlock *SwitchBlock = Builder->CreateSwitchBlock(Body, SourceLoc);
        EXPECT_TRUE(Builder->AddStmt(SwitchBlock));

        // switch a 
        ASTVarRefExpr *aVarRef = Builder->CreateExpr(SwitchBlock, Builder->CreateVarRef(aParam));
        
        // case 1: a = 1 break
        ASTSwitchCaseBlock *Case1Block = Builder->CreateSwitchCaseBlock(SwitchBlock, SourceLoc);
        Builder->CreateExpr(Case1Block, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        ASTVarAssign *aVarAssign1 = Builder->CreateVarAssign(Case1Block, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign1, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign1));

        // case 2: a = 2 break
        ASTSwitchCaseBlock *Case2Block = Builder->CreateSwitchCaseBlock(SwitchBlock, SourceLoc);
        Builder->CreateExpr(Case2Block, SemaBuilder::CreateIntegerValue(SourceLoc, 2));
        ASTVarAssign *aVarAssign2 = Builder->CreateVarAssign(Case2Block, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign2, SemaBuilder::CreateIntegerValue(SourceLoc, 2));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign2));

        // default: a = 3
        ASTSwitchDefaultBlock *DefaultBlock = Builder->CreateSwitchDefaultBlock(SwitchBlock, SourceLoc);
        ASTVarAssign *aVarAssign3 = Builder->CreateVarAssign(DefaultBlock, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign3, SemaBuilder::CreateIntegerValue(SourceLoc, 3));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign3));

        // return a
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(aParam));
        EXPECT_TRUE(Builder->AddStmt(Return));

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
                          "  switch i32 %2, label %default [\n"
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
                          "  %3 = load i32, i32* %1, align 4\n"
                          "  ret i32 %3\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGWhileBlock) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);
        ASTParam *aParam = Builder->CreateParam(MainFn, SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder->AddParam(aParam));

        // Create while block
        ASTWhileBlock *WhileBlock = Builder->CreateWhileBlock(Body, SourceLoc);
        EXPECT_TRUE(Builder->AddStmt(WhileBlock));

        // while a == 1
        ASTValueExpr *Value1 = Builder->CreateExpr(aParam, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder->CreateExpr(WhileBlock, Builder->CreateVarRef(aParam));
        ASTBinaryGroupExpr *WhileCond = Builder->CreateBinaryExpr(WhileBlock, SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value1);

        // { a = 1 }
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(WhileBlock, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign));

        // return a
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(aParam));
        EXPECT_TRUE(Builder->AddStmt(Return));

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
                          "  br label %whilecond\n"
                          "\n"
                          "whilecond:                                        ; preds = %whileloop, %entry\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  %3 = icmp eq i32 %2, 1\n"
                          "  br i1 %3, label %whileloop, label %whileend\n"
                          "\n"
                          "whileloop:                                        ; preds = %whilecond\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  br label %whilecond\n"
                          "\n"
                          "whileend:                                         ; preds = %whilecond\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  ret i32 %4\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGForInitCondPostBlock) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);
        ASTParam *aParam = Builder->CreateParam(MainFn, SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder->AddParam(aParam));

        // Create for block
        ASTForBlock *ForBlock = Builder->CreateForBlock(Body, SourceLoc);
        ASTForLoopBlock *LoopBlock = Builder->CreateForLoopBlock(ForBlock, SourceLoc);
        ASTForPostBlock *PostBlock = Builder->CreateForPostBlock(ForBlock, SourceLoc);
        EXPECT_TRUE(Builder->AddStmt(ForBlock));

        // for int i = 1; i < 1; ++i
        ASTLocalVar *iVar = Builder->CreateLocalVar(ForBlock, SourceLoc, IntType, "i");
        ASTValueExpr *Value1 = Builder->CreateExpr(iVar, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *iVarRef = Builder->CreateExpr(ForBlock, Builder->CreateVarRef(iVar));
        EXPECT_TRUE(Builder->AddStmt(iVar));
        
        ASTBinaryGroupExpr *ForCond = Builder->CreateBinaryExpr(ForBlock, SourceLoc, ASTBinaryOperatorKind::COMP_LTE, iVarRef, Value1);
        
        ASTExprStmt *iPreInc = Builder->CreateExprStmt(PostBlock, SourceLoc);
        Builder->CreateUnaryExpr(iPreInc, SourceLoc, ASTUnaryOperatorKind::ARITH_INCR, ASTUnaryOptionKind::UNARY_PRE,
                                 Builder->CreateExpr(iPreInc, Builder->CreateVarRef(iVar)));
        EXPECT_TRUE(Builder->AddStmt(iPreInc));

        // { a = 1}
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(LoopBlock, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign));
        
        // return a
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(aParam));
        EXPECT_TRUE(Builder->AddStmt(Return));

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
                          "  %2 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %1, align 4\n"
                          "  store i32 1, i32* %2, align 4\n"
                          "  br label %forcond\n"
                          "\n"
                          "forcond:                                          ; preds = %forpost, %entry\n"
                          "  %3 = load i32, i32* %2, align 4\n"
                          "  %4 = icmp sle i32 %3, 1\n"
                          "  br i1 %4, label %forloop, label %endfor\n"
                          "\n"
                          "forloop:                                          ; preds = %forcond\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  br label %forpost\n"
                          "\n"
                          "forpost:                                          ; preds = %forloop\n"
                          "  %5 = load i32, i32* %2, align 4\n"
                          "  %6 = add nsw i32 %5, 1\n"
                          "  store i32 %6, i32* %2, align 4\n"
                          "  br label %forcond\n"
                          "\n"
                          "endfor:                                           ; preds = %forcond\n"
                          "  %7 = load i32, i32* %1, align 4\n"
                          "  ret i32 %7\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGForCondBlock) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);
        ASTParam *aParam = Builder->CreateParam(MainFn, SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder->AddParam(aParam));

        // Create for block
        ASTForBlock *ForBlock = Builder->CreateForBlock(Body, SourceLoc);
        ASTForLoopBlock *LoopBlock = Builder->CreateForLoopBlock(ForBlock, SourceLoc);
        EXPECT_TRUE(Builder->AddStmt(ForBlock));

        // for a < 1
        ASTVarRefExpr *aVarRef = Builder->CreateExpr(ForBlock, Builder->CreateVarRef(aParam));
        ASTValueExpr *Value1 = Builder->CreateExpr(ForBlock, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        ASTBinaryGroupExpr *ForCond = Builder->CreateBinaryExpr(ForBlock, SourceLoc, ASTBinaryOperatorKind::COMP_LTE, aVarRef, Value1);

        // { a = 1}
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(LoopBlock, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign));

        // return a
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(aParam));
        EXPECT_TRUE(Builder->AddStmt(Return));

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
                          "  br label %forcond\n"
                          "\n"
                          "forcond:                                          ; preds = %forloop, %entry\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  %3 = icmp sle i32 %2, 1\n"
                          "  br i1 %3, label %forloop, label %endfor\n"
                          "\n"
                          "forloop:                                          ; preds = %forcond\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  br label %forcond\n"
                          "\n"
                          "endfor:                                           ; preds = %forcond\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  ret i32 %4\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGForPostBlock) {
        ASTNode *Node = CreateNode();

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);
        ASTParam *aParam = Builder->CreateParam(MainFn, SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder->AddParam(aParam));

        // Create for block
        ASTForBlock *ForBlock = Builder->CreateForBlock(Body, SourceLoc);
        ASTForLoopBlock *LoopBlock = Builder->CreateForLoopBlock(ForBlock, SourceLoc);
        ASTForPostBlock *PostBlock = Builder->CreateForPostBlock(ForBlock, SourceLoc);
        EXPECT_TRUE(Builder->AddStmt(ForBlock));

        // for a < 1; ++a
        ASTVarRefExpr *aVarRef = Builder->CreateExpr(ForBlock, Builder->CreateVarRef(aParam));
        ASTValueExpr *Value1 = Builder->CreateExpr(aParam, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        ASTBinaryGroupExpr *ForCond = Builder->CreateBinaryExpr(ForBlock, SourceLoc, ASTBinaryOperatorKind::COMP_LTE, aVarRef, Value1);

        ASTExprStmt *aPreInc = Builder->CreateExprStmt(PostBlock, SourceLoc);
        Builder->CreateUnaryExpr(aPreInc, SourceLoc, ASTUnaryOperatorKind::ARITH_INCR, ASTUnaryOptionKind::UNARY_PRE,
                                 aVarRef);
        EXPECT_TRUE(Builder->AddStmt(aPreInc));

        // { a = 1}
        ASTVarAssign *aVarAssign = Builder->CreateVarAssign(LoopBlock, Builder->CreateVarRef(aParam));
        Builder->CreateExpr(aVarAssign, SemaBuilder::CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder->AddStmt(aVarAssign));

        // return a
        ASTReturn *Return = Builder->CreateReturn(Body, SourceLoc);
        Builder->CreateExpr(Return, Builder->CreateVarRef(aParam));
        EXPECT_TRUE(Builder->AddStmt(Return));

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
                          "  br label %forcond\n"
                          "\n"
                          "forcond:                                          ; preds = %forpost, %entry\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  %3 = icmp sle i32 %2, 1\n"
                          "  br i1 %3, label %forloop, label %endfor\n"
                          "\n"
                          "forloop:                                          ; preds = %forcond\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  br label %forpost\n"
                          "\n"
                          "forpost:                                          ; preds = %forloop\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  %5 = add nsw i32 %4, 1\n"
                          "  store i32 %5, i32* %1, align 4\n"
                          "  br label %forcond\n"
                          "\n"
                          "endfor:                                           ; preds = %forcond\n"
                          "  %6 = load i32, i32* %1, align 4\n"
                          "  ret i32 %6\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGClassStruct) {
        ASTNode *Node = CreateNode();

        // TestClass {
        //   int a
        //   public int b
        //   private const int c
        // }
        ASTClass *TestClass = Builder->CreateClass(Node, SourceLoc, "TestClass",
                                                   SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTClassVar *aField = Builder->CreateClassVar(TestClass, SourceLoc, SemaBuilder::CreateIntType(SourceLoc),
                                "a",
                                SemaBuilder::CreateClassScopes(
                                        ASTClassVisibilityKind::CLASS_V_DEFAULT, false));
        ASTClassVar *bField = Builder->CreateClassVar(TestClass, SourceLoc, SemaBuilder::CreateIntType(SourceLoc),
                                "b",
                                SemaBuilder::CreateClassScopes(
                                        ASTClassVisibilityKind::CLASS_V_PUBLIC, false));
        ASTClassVar *cField = Builder->CreateClassVar(TestClass, SourceLoc, SemaBuilder::CreateIntType(SourceLoc),
                                "c",
                                SemaBuilder::CreateClassScopes(
                                        ASTClassVisibilityKind::CLASS_V_PRIVATE, true));
        Builder->AddClass(Node, TestClass);

        // main()
        ASTFunction *MainFn = Builder->CreateFunction(Node, SourceLoc, IntType, "main",
                                                      SemaBuilder::CreateTopScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlock *Body = Builder->getBlock(MainFn);

        // TestClass test;
        ASTType *TestClassType = SemaBuilder::CreateClassType(TestClass);
        ASTLocalVar *TestVar = Builder->CreateLocalVar(Body, SourceLoc, TestClassType, "test");
        Builder->AddStmt(TestVar);

        // int a = test.a
        ASTType *aType = aField->getType();
        ASTLocalVar *aVar = Builder->CreateLocalVar(Body, SourceLoc, aType, "a");
        ASTVarRefExpr *aRefExpr = Builder->CreateExpr(aVar, Builder->CreateVarRef(aField));
        Builder->AddStmt(aVar);

        // int b = test.b
        ASTType *bType = SemaBuilder::CreateIntType(SourceLoc);
        ASTLocalVar *bVar = Builder->CreateLocalVar(Body, SourceLoc, bType, "b");
        ASTVarRefExpr *bRefExpr = Builder->CreateExpr(bVar, Builder->CreateVarRef(bField));
        Builder->AddStmt(bVar);

        // int c = test.c
        ASTType *cType = SemaBuilder::CreateIntType(SourceLoc);
        ASTLocalVar *cVar = Builder->CreateLocalVar(Body, SourceLoc, cType, "c");
        ASTVarRefExpr *cRefExpr = Builder->CreateExpr(cVar, Builder->CreateVarRef(cField));
        Builder->AddStmt(cVar);

        // Add to Node
        EXPECT_TRUE(Builder->AddFunction(Node, MainFn));
        EXPECT_TRUE(Builder->AddNode(Node));
        EXPECT_TRUE(Builder->Build());

        // Generate Code
        CodeGenClass *CGC = CGM->GenClass(TestClass);
        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
//        CGM->Module->print(llvm::outs(), nullptr);
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                            "entry:\n"
                            "  %0 = alloca %TestClass, align 8\n"
                            "  %1 = alloca i32, align 4\n"
                            "  %2 = alloca i32, align 4\n"
                            "  %3 = alloca i32, align 4\n"
                            "  %4 = getelementptr inbounds %TestClass, %TestClass* %0, i32 0, i32 0\n"
                            "  %5 = load i32, i32* %4, align 4\n"
                            "  store i32 %5, i32* %1, align 4\n"
                            "  %6 = getelementptr inbounds %TestClass, %TestClass* %0, i32 0, i32 1\n"
                            "  %7 = load i32, i32* %6, align 4\n"
                            "  store i32 %7, i32* %2, align 4\n"
                            "  %8 = getelementptr inbounds %TestClass, %TestClass* %0, i32 0, i32 2\n"
                            "  %9 = load i32, i32* %8, align 4\n"
                            "  store i32 %9, i32* %3, align 4\n"
                            "}\n");
    }

} // anonymous namespace
