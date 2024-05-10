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
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTVar.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTVarStmt.h"
#include "AST/ASTValue.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTParam.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTEnum.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTEnumEntry.h"
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
        Sema *S;
        SourceLocation SourceLoc;
        SemaBuilder &Builder;

        ASTType *VoidType;
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
                S(Sema::CreateSema(CI.getDiagnostics())),
                Builder(S->getBuilder()),
                VoidType(Builder.CreateVoidType(SourceLoc)),
                BoolType(Builder.CreateBoolType(SourceLoc)),
                ByteType(Builder.CreateByteType(SourceLoc)),
                ShortType(Builder.CreateShortType(SourceLoc)),
                UShortType(Builder.CreateUShortType(SourceLoc)),
                IntType(Builder.CreateIntType(SourceLoc)),
                UIntType(Builder.CreateUIntType(SourceLoc)),
                LongType(Builder.CreateLongType(SourceLoc)),
                ULongType(Builder.CreateULongType(SourceLoc)),
                FloatType(Builder.CreateFloatType(SourceLoc)),
                DoubleType(Builder.CreateDoubleType(SourceLoc)),
                ArrayInt0Type(Builder.CreateArrayType(SourceLoc, IntType,
                                                       Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 0))))
                {
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmPrinters();
        }

        ASTModule *CreateModule() {
            Diags.getClient()->BeginSourceFile();
            const std::string Name = "CodeGenTest";
            auto *Module = Builder.CreateModule(Name);
            ASTNameSpace *NameSpace = Builder.CreateDefaultNameSpace();
            Builder.AddNameSpace(Module, NameSpace);
            Builder.AddModule(Module);
            Diags.getClient()->EndSourceFile();
            return Module;
        }

        virtual ~CodeGenTest() {
            llvm::outs().flush();
        }

        std::string getOutput() {
            testing::internal::CaptureStdout();
            CGM->Module->print(llvm::outs(), nullptr);
            std::string output = testing::internal::GetCapturedStdout();
            output.erase(0, output.find("\n") + 1);
            output.erase(0, output.find("\n") + 1);
            output.erase(0, output.find("\n") + 1);
            output.erase(0, output.find("\n") + 2);
            return output;
        }
    };

    TEST_F(CodeGenTest, CGDefaultValueGlobalVar) {
        ASTModule *Module = CreateModule();

        ASTScopes *TopScopes = Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false);

        // default Bool value
        ASTValue *DefaultBoolVal = Builder.CreateDefaultValue(BoolType);
        ASTGlobalVar *aVar = Builder.CreateGlobalVar(SourceLoc, BoolType, "a", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, aVar, DefaultBoolVal));

        // default Byte value
        ASTValue *DefaultByteVal = Builder.CreateDefaultValue(ByteType);
        ASTGlobalVar *bVar = Builder.CreateGlobalVar(SourceLoc, ByteType, "b", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, bVar, DefaultByteVal));

        // default Short value
        ASTValue *DefaultShortVal = Builder.CreateDefaultValue(ShortType);
        ASTGlobalVar *cVar = Builder.CreateGlobalVar(SourceLoc, ShortType, "c", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, cVar, DefaultShortVal));

        // default UShort value
        ASTValue *DefaultUShortVal = Builder.CreateDefaultValue(UShortType);
        ASTGlobalVar *dVar = Builder.CreateGlobalVar(SourceLoc, UShortType, "d", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, dVar, DefaultUShortVal));

        // default Int value
        ASTValue *DefaultIntVal = Builder.CreateDefaultValue(IntType);
        ASTGlobalVar *eVar = Builder.CreateGlobalVar(SourceLoc, IntType, "e", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, eVar, DefaultIntVal));

        // default UInt value
        ASTValue *DefaultUintVal = Builder.CreateDefaultValue(UIntType);
        ASTGlobalVar *fVar = Builder.CreateGlobalVar(SourceLoc, UIntType, "f", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, fVar, DefaultUintVal));

        // default Long value
        ASTValue *DefaultLongVal = Builder.CreateDefaultValue(LongType);
        ASTGlobalVar *gVar = Builder.CreateGlobalVar(SourceLoc, LongType, "g", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, gVar, DefaultLongVal));

        // default ULong value
        ASTValue *DefaultULongVal = Builder.CreateDefaultValue(ULongType);
        ASTGlobalVar *hVar = Builder.CreateGlobalVar(SourceLoc, ULongType, "h", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, hVar, DefaultULongVal));

        // default Float value
        ASTValue *DefaultFloatVal = Builder.CreateDefaultValue(FloatType);
        ASTGlobalVar *iVar = Builder.CreateGlobalVar(SourceLoc, FloatType, "i", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, iVar, DefaultFloatVal));

        // default Double value
        ASTValue *DefaultDoubleVal = Builder.CreateDefaultValue(DoubleType);
        ASTGlobalVar *jVar = Builder.CreateGlobalVar(SourceLoc, DoubleType, "j", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, jVar, DefaultDoubleVal));

        // default Array value
        ASTValue *DefaultArrayVal = Builder.CreateDefaultValue(ArrayInt0Type);
        ASTGlobalVar *kVar = Builder.CreateGlobalVar(SourceLoc, ArrayInt0Type, "k", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, kVar, DefaultArrayVal));

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
        ASTModule *Module = CreateModule();

        ASTScopes *TopScopes = Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false);

        // a
        ASTBoolValue *BoolVal = Builder.CreateBoolValue(SourceLoc, true);
        ASTGlobalVar *aVar = Builder.CreateGlobalVar(SourceLoc, BoolType, "a", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, aVar, BoolVal));

        // b
        ASTIntegerValue *ByteVal = Builder.CreateIntegerValue(SourceLoc, 1);
        ASTGlobalVar *bVar = Builder.CreateGlobalVar(SourceLoc, ByteType, "b", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, bVar, ByteVal));

        // c
        ASTIntegerValue *ShortVal = Builder.CreateIntegerValue(SourceLoc, -2);
        ASTGlobalVar *cVar = Builder.CreateGlobalVar(SourceLoc, ShortType, "c", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, cVar, ShortVal));

        // d
        ASTIntegerValue *UShortVal = Builder.CreateIntegerValue(SourceLoc, 2);
        ASTGlobalVar *dVar = Builder.CreateGlobalVar(SourceLoc, UShortType, "d", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, dVar, UShortVal));

        // e
        ASTIntegerValue *IntVal = Builder.CreateIntegerValue(SourceLoc, -3);
        ASTGlobalVar *eVar = Builder.CreateGlobalVar(SourceLoc, IntType, "e", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, eVar, IntVal));

        // f
        ASTIntegerValue *UIntVal = Builder.CreateIntegerValue(SourceLoc, 3);
        ASTGlobalVar *fVar = Builder.CreateGlobalVar(SourceLoc, UIntType, "f", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, fVar, UIntVal));

        // g
        ASTIntegerValue *LongVal = Builder.CreateIntegerValue(SourceLoc, -4);
        ASTGlobalVar *gVar = Builder.CreateGlobalVar(SourceLoc, LongType, "g", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, gVar, LongVal));

        // h
        ASTIntegerValue *ULongVal = Builder.CreateIntegerValue(SourceLoc, 4);
        ASTGlobalVar *hVar = Builder.CreateGlobalVar(SourceLoc, ULongType, "h", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, hVar, ULongVal));

        // i
        ASTFloatingValue *FloatVal = Builder.CreateFloatingValue(SourceLoc, 1.5);
        ASTGlobalVar *iVar = Builder.CreateGlobalVar(SourceLoc, FloatType, "i", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, iVar, FloatVal));

        // j
        ASTFloatingValue *DoubleVal = Builder.CreateFloatingValue(SourceLoc, 2.5);
        ASTGlobalVar *jVar = Builder.CreateGlobalVar(SourceLoc, DoubleType, "j", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, jVar, DoubleVal));

        // k (empty array)
        ASTArrayValue *ArrayValEmpty = Builder.CreateArrayValue(SourceLoc);
        ASTGlobalVar *kVar = Builder.CreateGlobalVar(SourceLoc, ArrayInt0Type, "k", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, kVar, ArrayValEmpty));

        // l (array with 2 val)
        ASTArrayValue *ArrayVal = Builder.CreateArrayValue(SourceLoc);
        Builder.AddArrayValue(ArrayVal, Builder.CreateIntegerValue(SourceLoc, 1)); // ArrayVal = {1}
        Builder.AddArrayValue(ArrayVal, Builder.CreateIntegerValue(SourceLoc, 2)); // ArrayVal = {1, 1}
        ASTValueExpr *SizeExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, ArrayVal->size()));
        ASTArrayType *ArrayInt2Type = Builder.CreateArrayType(SourceLoc, IntType, SizeExpr);
        ASTGlobalVar *lVar = Builder.CreateGlobalVar(SourceLoc, ArrayInt2Type, "l", TopScopes);
        EXPECT_TRUE(Builder.AddGlobalVar(Module, lVar, ArrayVal));

        // m (string)
        ASTStringType *StringType = Builder.CreateStringType(SourceLoc);
        ASTGlobalVar *mVar = Builder.CreateGlobalVar(SourceLoc, StringType, "m", TopScopes);
        ASTStringValue *StringVal = Builder.CreateStringValue(SourceLoc, "hello");
        EXPECT_TRUE(Builder.AddGlobalVar(Module, mVar, StringVal));

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

        GlobalVariable *mGVar = (GlobalVariable *) CGM->GenGlobalVar(mVar)->getPointer();
        testing::internal::CaptureStdout();
        mGVar->print(llvm::outs());
        output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output, "@m = global [6 x i8] c\"hello\\00\"");
    }

    TEST_F(CodeGenTest, CGFunc) {
        ASTModule *Module = CreateModule();

        ASTScopes *TopScopes = Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false);
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func", TopScopes);

        ASTScopes *Scopes = Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false);
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, IntType, "P1", Scopes)));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, FloatType, "P2", Scopes)));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, BoolType, "P3", Scopes)));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, LongType, "P4", Scopes)));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, DoubleType, "P5", Scopes)));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, ByteType, "P6", Scopes)));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, ShortType, "P7", Scopes)));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, UShortType, "P8", Scopes)));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, UIntType, "P9", Scopes)));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, ULongType, "P10", Scopes)));
        ASTBlockStmt *Body = Builder.CreateBody(Func);

        EXPECT_TRUE(Builder.AddFunction(Module, Func));
        
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1, float %2, i1 %3, i64 %4, double %5, i8 %6, i16 %7, i16 %8, i32 %9, i64 %10) {\n"
                          "entry:\n"
                          "  %11 = alloca i32, align 4\n"
                          "  %12 = alloca float, align 4\n"
                          "  %13 = alloca i8, align 1\n"
                          "  %14 = alloca i64, align 8\n"
                          "  %15 = alloca double, align 8\n"
                          "  %16 = alloca i8, align 1\n"
                          "  %17 = alloca i16, align 2\n"
                          "  %18 = alloca i16, align 2\n"
                          "  %19 = alloca i32, align 4\n"
                          "  %20 = alloca i64, align 8\n"
                          "  store i32 %1, i32* %11, align 4\n"
                          "  store float %2, float* %12, align 4\n"
                          "  %21 = zext i1 %3 to i8\n"
                          "  store i8 %21, i8* %13, align 1\n"
                          "  store i64 %4, i64* %14, align 8\n"
                          "  store double %5, double* %15, align 8\n"
                          "  store i8 %6, i8* %16, align 1\n"
                          "  store i16 %7, i16* %17, align 2\n"
                          "  store i16 %8, i16* %18, align 2\n"
                          "  store i32 %9, i32* %19, align 4\n"
                          "  store i64 %10, i64* %20, align 8\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGFuncUseGlobalVar) {
        ASTModule *Module = CreateModule();

        // float G = 2.0
        ASTFloatingValue *FloatingVal = Builder.CreateFloatingValue(SourceLoc, 2.0);
        ASTGlobalVar *GVar = Builder.CreateGlobalVar(SourceLoc, FloatType, "G",
                                                      Builder.CreateScopes(ASTVisibilityKind::V_PRIVATE, false));
        EXPECT_TRUE(Builder.AddGlobalVar(Module, GVar, FloatingVal));

        // func()
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, IntType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // int A
        ASTLocalVar *VarA = Builder.CreateLocalVar(SourceLoc, IntType, "A");
        ASTVarStmt *VarADecl = Builder.CreateVarStmt(VarA);
        EXPECT_TRUE(Builder.AddStmt(Body, VarADecl));
        
        // A = 1
        ASTVarStmt * VarAAssign = Builder.CreateVarStmt(Builder.CreateVarRef(VarA));
        ASTExpr *AssignExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        Builder.AddExpr(VarAAssign, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, VarAAssign));

        // GlobalVar
        // G = 1
        ASTVarRef *VarRefG = Builder.CreateVarRef(GVar);
        ASTVarStmt * GVarStmt = Builder.CreateVarStmt(VarRefG);
        AssignExpr = Builder.CreateExpr(Builder.CreateFloatingValue(SourceLoc, 1));
        Builder.AddExpr(GVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, GVarStmt));

        // return A
        ASTReturnStmt *Return = Builder.CreateReturnStmt(SourceLoc);
        AssignExpr = Builder.CreateExpr(Builder.CreateVarRef(VarA));
        Builder.AddExpr(Return, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, Return));

        // add to Module
        EXPECT_TRUE(Builder.AddFunction(Module, Func));
        
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenGlobalVar *CGGV = CGM->GenGlobalVar(GVar);
        Value *G = CGGV->getPointer();

        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();
        
        EXPECT_FALSE(Diags.hasErrorOccurred());

        testing::internal::CaptureStdout();
        G->print(llvm::outs());
        std::string output1 = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output1, "@G = internal global float 2.000000e+00");

        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output2 = testing::internal::GetCapturedStdout();
        EXPECT_EQ(output2, "define i32 @func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca i32, align 4\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  store float 1.000000e+00, float* @G, align 4\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  ret i32 %2\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGValue) {
        ASTModule *Module = CreateModule();

        // func()
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                        Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        // int a = 1
        ASTLocalVar *LocalVar = Builder.CreateLocalVar(SourceLoc, Builder.CreateIntType(SourceLoc), "a");
        ASTVarStmt *VarStmt = Builder.CreateVarStmt(LocalVar);
        ASTValueExpr *ValueExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        Builder.AddExpr(VarStmt, ValueExpr);
        Builder.AddStmt(Body, VarStmt);

        // add to Module
        EXPECT_TRUE(Builder.AddFunction(Module, Func));
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
                          "  %1 = alloca i32, align 4\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGFuncCall) {
        ASTModule *Module = CreateModule();

        // func()
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, IntType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // test()
        ASTFunction *Test = Builder.CreateFunction(SourceLoc, IntType, "test",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        Builder.CreateBody(Test);

        // call test()
        ASTExprStmt *ExprStmt = Builder.CreateExprStmt(SourceLoc);
        ASTCall *TestCall = Builder.CreateCall(Test);
        ASTCallExpr *Expr = Builder.CreateExpr(TestCall);
        Builder.AddExpr(ExprStmt, Expr);
        EXPECT_TRUE(Builder.AddStmt(Body, ExprStmt));

        //return test()
        ASTReturnStmt *Return = Builder.CreateReturnStmt(SourceLoc);
        ASTCallExpr *ReturnExpr = Builder.CreateExpr(TestCall);
        Builder.AddExpr(Return, ReturnExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, Return));

        // add to Module
        EXPECT_TRUE(Builder.AddFunction(Module, Func));
        EXPECT_TRUE(Builder.AddFunction(Module, Test));
        
        EXPECT_TRUE(S->Resolve());


        // Generate Code
        CGM->GenFunction(Test);
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = call i32 @test(%error* %0)\n"
                          "  %2 = call i32 @test(%error* %0)\n"
                          "  ret i32 %2\n"
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
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, IntType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(Func, aParam));
        ASTParam *bParam = Builder.CreateParam(SourceLoc, IntType, "b");
        EXPECT_TRUE(Builder.AddParam(Func, bParam));
        ASTParam *cParam = Builder.CreateParam(SourceLoc, IntType, "c");
        EXPECT_TRUE(Builder.AddParam(Func, cParam));

        ASTReturnStmt *Return = Builder.CreateReturnStmt(SourceLoc);
        // Create this expression: 1 + a * b / (c - 2)
        // E1 + (E2 * E3) / (E4 - E5)
        // E1 + (G2 / G3)
        // E1 + G1
        ASTValueExpr *E1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *E2 = Builder.CreateExpr(Builder.CreateVarRef(aParam));
        ASTVarRefExpr *E3 = Builder.CreateExpr(Builder.CreateVarRef(bParam));
        ASTVarRefExpr *E4 = Builder.CreateExpr(Builder.CreateVarRef(cParam));
        ASTValueExpr *E5 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));

        ASTBinaryGroupExpr *G2 = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::ARITH_MUL, E2, E3);
        ASTBinaryGroupExpr *G3 = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::ARITH_SUB, E4, E5);
        ASTBinaryGroupExpr *G1 = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::ARITH_DIV, G2, G3);
        ASTBinaryGroupExpr *Group = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::ARITH_ADD, E1, G1);

        Builder.AddExpr(Return, Group);
        EXPECT_TRUE(Builder.AddStmt(Body, Return));

        // Add to Module
        EXPECT_TRUE(Builder.AddFunction(Module, Func));
        
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @func(%error* %0, i32 %1, i32 %2, i32 %3) {\n"
                          "entry:\n"
                          "  %4 = alloca i32, align 4\n"
                          "  %5 = alloca i32, align 4\n"
                          "  %6 = alloca i32, align 4\n"
                          "  store i32 %1, i32* %4, align 4\n"
                          "  store i32 %2, i32* %5, align 4\n"
                          "  store i32 %3, i32* %6, align 4\n"
                          "  %7 = load i32, i32* %4, align 4\n"
                          "  %8 = load i32, i32* %5, align 4\n"
                          "  %9 = mul i32 %7, %8\n"
                          "  %10 = load i32, i32* %6, align 4\n"
                          "  %11 = sub i32 %10, 2\n"
                          "  %12 = sdiv i32 %9, %11\n"
                          "  %13 = add i32 1, %12\n"
                          "  ret i32 %13\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGArithOp) {
        ASTModule *Module = CreateModule();

        // func()
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(Func, aParam));
        ASTParam *bParam = Builder.CreateParam(SourceLoc, IntType, "b");
        EXPECT_TRUE(Builder.AddParam(Func, bParam));
        ASTParam *cParam = Builder.CreateParam(SourceLoc, IntType, "c");
        EXPECT_TRUE(Builder.AddParam(Func, cParam));

        // a = 0
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        ASTExpr *AssignExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 0));
        Builder.AddExpr(aVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt));

        // b = 0
        ASTVarStmt *bVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(bParam));
        AssignExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 0));
        Builder.AddExpr(bVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, bVarStmt));

        // c = a + b
        ASTVarStmt * cAddVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::ARITH_ADD,
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cAddVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cAddVarStmt));

        // c = a - b
        ASTVarStmt * cSubVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::ARITH_SUB,
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cSubVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cSubVarStmt));

        // c = a * b
        ASTVarStmt * cMulVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::ARITH_MUL,
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cMulVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cMulVarStmt));

        // c = a / b
        ASTVarStmt * cDivVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::ARITH_DIV,
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cDivVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cDivVarStmt));

        // c = a % b
        ASTVarStmt * cModVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::ARITH_MOD,
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cModVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cModVarStmt));

        // c = a & b
        ASTVarStmt * cAndVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::ARITH_AND,
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cAndVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cAndVarStmt));

        // c = a | b
        ASTVarStmt * cOrVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::ARITH_OR,
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cOrVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cOrVarStmt));

        // c = a xor b
        ASTVarStmt * cXorVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::ARITH_XOR,
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cXorVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cXorVarStmt));

        // c = a << b
        ASTVarStmt * cShlVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::ARITH_SHIFT_L,
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cShlVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cShlVarStmt));

        // c = a >> b
        ASTVarStmt * cShrVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::ARITH_SHIFT_R,
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cShrVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cShrVarStmt));

        // ++c
        ASTExprStmt *cPreIncVarStmt = Builder.CreateExprStmt(SourceLoc);
        AssignExpr = Builder.CreateUnaryExpr(SourceLoc, ASTUnaryOperatorKind::ARITH_INCR,
                                              ASTUnaryOptionKind::UNARY_PRE,
                                              Builder.CreateExpr(Builder.CreateVarRef(cParam)));
        Builder.AddExpr(cPreIncVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cPreIncVarStmt));

        // c++
        ASTExprStmt *cPostIncVarStmt = Builder.CreateExprStmt(SourceLoc);
        AssignExpr = Builder.CreateUnaryExpr(SourceLoc, ASTUnaryOperatorKind::ARITH_INCR,
                                              ASTUnaryOptionKind::UNARY_POST,
                                              Builder.CreateExpr(Builder.CreateVarRef(cParam)));
        Builder.AddExpr(cPostIncVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cPostIncVarStmt));

        // ++c
        ASTExprStmt *cPreDecVarStmt = Builder.CreateExprStmt(SourceLoc);
        AssignExpr = Builder.CreateUnaryExpr(SourceLoc, ASTUnaryOperatorKind::ARITH_DECR,
                                              ASTUnaryOptionKind::UNARY_PRE,
                                              Builder.CreateExpr(Builder.CreateVarRef(cParam)));
        Builder.AddExpr(cPreDecVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cPreDecVarStmt));

        // c++
        ASTExprStmt *cPostDecVarStmt = Builder.CreateExprStmt(SourceLoc);
        AssignExpr = Builder.CreateUnaryExpr(SourceLoc, ASTUnaryOperatorKind::ARITH_DECR,
                                              ASTUnaryOptionKind::UNARY_POST,
                                              Builder.CreateExpr(Builder.CreateVarRef(cParam)));
        Builder.AddExpr(cPostDecVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cPostDecVarStmt));

        // Add to Module
        EXPECT_TRUE(Builder.AddFunction(Module, Func));
        
        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1, i32 %2, i32 %3) {\n"
                          "entry:\n"
                          "  %4 = alloca i32, align 4\n"
                          "  %5 = alloca i32, align 4\n"
                          "  %6 = alloca i32, align 4\n"
                          "  store i32 %1, i32* %4, align 4\n"
                          "  store i32 %2, i32* %5, align 4\n"
                          "  store i32 %3, i32* %6, align 4\n"
                          "  store i32 0, i32* %4, align 4\n"
                          "  store i32 0, i32* %5, align 4\n"
                          "  %7 = load i32, i32* %4, align 4\n"
                          "  %8 = load i32, i32* %5, align 4\n"
                          "  %9 = add i32 %7, %8\n"
                          "  store i32 %9, i32* %6, align 4\n"
                          "  %10 = sub i32 %7, %8\n"
                          "  store i32 %10, i32* %6, align 4\n"
                          "  %11 = mul i32 %7, %8\n"
                          "  store i32 %11, i32* %6, align 4\n"
                          "  %12 = sdiv i32 %7, %8\n"
                          "  store i32 %12, i32* %6, align 4\n"
                          "  %13 = srem i32 %7, %8\n"
                          "  store i32 %13, i32* %6, align 4\n"
                          "  %14 = and i32 %7, %8\n"
                          "  store i32 %14, i32* %6, align 4\n"
                          "  %15 = or i32 %7, %8\n"
                          "  store i32 %15, i32* %6, align 4\n"
                          "  %16 = xor i32 %7, %8\n"
                          "  store i32 %16, i32* %6, align 4\n"
                          "  %17 = shl i32 %7, %8\n"
                          "  store i32 %17, i32* %6, align 4\n"
                          "  %18 = ashr i32 %7, %8\n"
                          "  store i32 %18, i32* %6, align 4\n"
                          // Unary Operations
                          "  %19 = load i32, i32* %6, align 4\n"
                          "  %20 = add nsw i32 %19, 1\n"
                          "  store i32 %20, i32* %6, align 4\n"
                          "  %21 = load i32, i32* %6, align 4\n"
                          "  %22 = add nsw i32 %21, 1\n"
                          "  store i32 %22, i32* %6, align 4\n"
                          "  %23 = load i32, i32* %6, align 4\n"
                          "  %24 = add nsw i32 %23, -1\n"
                          "  store i32 %24, i32* %6, align 4\n"
                          "  %25 = load i32, i32* %6, align 4\n"
                          "  %26 = add nsw i32 %25, -1\n"
                          "  store i32 %26, i32* %6, align 4\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGComparatorOp) {
        ASTModule *Module = CreateModule();

        // func()
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, IntType, "a");
        ASTLocalVar *bVar = Builder.CreateLocalVar(SourceLoc, IntType, "b");
        ASTLocalVar *cVar = Builder.CreateLocalVar(SourceLoc, BoolType, "c");

        // a = 0
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aVar));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 0));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt));

        // b = 0
        ASTVarStmt *bVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(bVar));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 0));
        EXPECT_TRUE(Builder.AddStmt(Body, bVarStmt));

        // c = a == b
        ASTVarStmt * cEqVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_EQ,
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddStmt(Body, cEqVarStmt));

        // c = a != b
        ASTVarStmt * cNeqVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_NE,
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddStmt(Body, cNeqVarStmt));

        // c = a > b
        ASTVarStmt * cGtVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_GT,
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddStmt(Body, cGtVarStmt));

        // c = a >= b
        ASTVarStmt * cGteVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_GTE,
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddStmt(Body, cGteVarStmt));

        // c = a < b
        ASTVarStmt * cLtVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_LT,
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddStmt(Body, cLtVarStmt));

        // c = a <= b
        ASTVarStmt * cLteVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_LTE,
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddStmt(Body, cLteVarStmt));

        // Add to Module
        EXPECT_TRUE(Builder.AddFunction(Module, Func));

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
                          "  %1 = alloca i32, align 4\n"
                          "  %2 = alloca i32, align 4\n"
                          "  %3 = alloca i8, align 1\n"
                          "  store i32 0, i32* %1, align 4\n"
                          "  store i32 0, i32* %2, align 4\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  %5 = load i32, i32* %2, align 4\n"
                          "  %6 = icmp eq i32 %4, %5\n"
                          "  %7 = zext i1 %6 to i8\n"
                          "  store i8 %7, i8* %3, align 1\n"
                          "  %8 = icmp ne i32 %4, %5\n"
                          "  %9 = zext i1 %8 to i8\n"
                          "  store i8 %9, i8* %3, align 1\n"
                          "  %10 = icmp sgt i32 %4, %5\n"
                          "  %11 = zext i1 %10 to i8\n"
                          "  store i8 %11, i8* %3, align 1\n"
                          "  %12 = icmp sge i32 %4, %5\n"
                          "  %13 = zext i1 %12 to i8\n"
                          "  store i8 %13, i8* %3, align 1\n"
                          "  %14 = icmp slt i32 %4, %5\n"
                          "  %15 = zext i1 %14 to i8\n"
                          "  store i8 %15, i8* %3, align 1\n"
                          "  %16 = icmp sle i32 %4, %5\n"
                          "  %17 = zext i1 %16 to i8\n"
                          "  store i8 %17, i8* %3, align 1\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGLogicOp) {
        ASTModule *Module = CreateModule();

        // func()
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, BoolType, "a");
        ASTLocalVar *bVar = Builder.CreateLocalVar(SourceLoc, BoolType, "b");
        ASTLocalVar *cVar = Builder.CreateLocalVar(SourceLoc, BoolType, "c");

        // a = false
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aVar));
        Builder.CreateExpr(Builder.CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt));

        // b = false
        ASTVarStmt *bVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(bVar));
        Builder.CreateExpr(Builder.CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder.AddStmt(Body, bVarStmt));

        // c = a and b
        ASTVarStmt * cAndVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::LOGIC_AND,
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddStmt(Body, cAndVarStmt));

        // c = a or b
        ASTVarStmt * cOrVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::LOGIC_OR,
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddStmt(Body, cOrVarStmt));

        // Add to Module
        EXPECT_TRUE(Builder.AddFunction(Module, Func));

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
                          "  %1 = alloca i8, align 1\n"
                          "  %2 = alloca i8, align 1\n"
                          "  %3 = alloca i8, align 1\n"
                          "  store i8 0, i8* %1, align 1\n"
                          "  store i8 0, i8* %2, align 1\n"
                          "  %4 = load i8, i8* %1, align 1\n"
                          "  %5 = trunc i8 %4 to i1\n"
                          "  br i1 %5, label %and, label %and1\n"
                          "\n"
                          "and:                                              ; preds = %entry\n"
                          "  %6 = load i8, i8* %2, align 1\n"
                          "  %7 = trunc i8 %6 to i1\n"
                          "  br label %and1\n"
                          "\n"
                          "and1:                                             ; preds = %and, %entry\n"
                          "  %8 = phi i1 [ false, %entry ], [ %7, %and ]\n"
                          "  %9 = zext i1 %8 to i8\n"
                          "  store i8 %9, i8* %3, align 1\n"
                          "  %10 = load i8, i8* %1, align 1\n"
                          "  %11 = trunc i8 %10 to i1\n"
                          "  br i1 %11, label %or2, label %or\n"
                          "\n"
                          "or:                                               ; preds = %and1\n"
                          "  %12 = load i8, i8* %2, align 1\n"
                          "  %13 = trunc i8 %12 to i1\n"
                          "  br label %or2\n"
                          "\n"
                          "or2:                                              ; preds = %or, %and1\n"
                          "  %14 = phi i1 [ true, %and1 ], [ %13, %or ]\n"
                          "  %15 = zext i1 %14 to i8\n"
                          "  store i8 %15, i8* %3, align 1\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGTernaryOp) {
        ASTModule *Module = CreateModule();

        // func()
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, BoolType, "a");
        ASTLocalVar *bVar = Builder.CreateLocalVar(SourceLoc, BoolType, "b");
        ASTLocalVar *cVar = Builder.CreateLocalVar(SourceLoc, BoolType, "c");

        // a = false
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aVar));
        Builder.CreateExpr(Builder.CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt));

        // b = false
        ASTVarStmt *bVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(bVar));
        Builder.CreateExpr(Builder.CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder.AddStmt(Body, bVarStmt));

        // c = a == b ? a : b
        ASTVarStmt * cVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        ASTBinaryGroupExpr *Cond = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_EQ,
                                                             Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                                             Builder.CreateExpr(Builder.CreateVarRef(bVar)));

        ASTTernaryGroupExpr *TernaryExpr = Builder.CreateTernaryExpr(Cond, 
                                                                     SourceLoc,
                                                                     Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                                                     SourceLoc,
                                                                      Builder.CreateExpr(Builder.CreateVarRef(bVar)));

        EXPECT_TRUE(Builder.AddStmt(Body, cVarStmt));

        // Add to Module
        EXPECT_TRUE(Builder.AddFunction(Module, Func));

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
                          "  %1 = alloca i8, align 1\n"
                          "  %2 = alloca i8, align 1\n"
                          "  %3 = alloca i8, align 1\n"
                          "  store i8 0, i8* %1, align 1\n"
                          "  store i8 0, i8* %2, align 1\n"
                          "  %4 = load i8, i8* %1, align 1\n"
                          "  %5 = load i8, i8* %2, align 1\n"
                          "  %6 = icmp eq i8 %4, %5\n"
                          "  br i1 %6, label %terntrue, label %ternfalse\n"
                          "\n"
                          "terntrue:                                         ; preds = %entry\n"
                          "  %7 = load i8, i8* %1, align 1\n"
                          "  %8 = trunc i8 %7 to i1\n"
                          "  br label %ternend\n"
                          "\n"
                          "ternfalse:                                        ; preds = %entry\n"
                          "  %9 = load i8, i8* %2, align 1\n"
                          "  %10 = trunc i8 %9 to i1\n"
                          "  br label %ternend\n"
                          "\n"
                          "ternend:                                          ; preds = %ternfalse, %terntrue\n"
                          "  %11 = phi i1 [ %8, %terntrue ], [ %10, %ternfalse ]\n"
                          "  %12 = zext i1 %11 to i8\n"
                          "  store i8 %12, i8* %3, align 1\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGIfBlock) {
        ASTModule *Module = CreateModule();

        // func()
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // int a = 0
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, IntType, "a");
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 0));

        // if (a == 1)
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(Builder.CreateVarRef(aVar));
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTBinaryGroupExpr *IfCond = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value1);

        // { a = 2 }
        ASTBlockStmt *IfBlock = Builder.CreateBlockStmt(SourceLoc);
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aVar));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        EXPECT_TRUE(Builder.AddStmt(IfBlock, aVarStmt));

        // Create if block
        ASTIfStmt *IfStmt = Builder.CreateIfStmt(SourceLoc, IfCond, IfBlock);
        EXPECT_TRUE(Builder.AddStmt(Body, IfStmt));

        // Add to Module
        EXPECT_TRUE(Builder.AddFunction(Module, Func));

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
                          "  %1 = alloca i32, align 4\n"
                          "  store i32 0, i32* %1, align 4\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  %3 = icmp eq i32 %2, 1\n"
                          "  br i1 %3, label %ifthen, label %endif\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 2, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "endif:                                            ; preds = %ifthen, %entry\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGIfElseBlock) {
        ASTModule *Module = CreateModule();

        // func()
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(Func, aParam));

        // if (a == 1)
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(Builder.CreateVarRef(aParam));
        ASTBinaryGroupExpr *IfCond = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value1);

        // Create if block
        ASTBlockStmt *IfBlock = Builder.CreateBlockStmt(SourceLoc);
        ASTIfStmt *IfStmt = Builder.CreateIfStmt(SourceLoc, IfCond, IfBlock);

        // { a = 1 }
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt));

        // else {a == 2}
        ASTBlockStmt *ElseBlock = Builder.CreateBlockStmt( SourceLoc);
        ASTVarStmt *aVarStmt2 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt2));
        EXPECT_TRUE(Builder.AddStmt(Body, IfBlock));

        // Add to Module
        EXPECT_TRUE(Builder.AddFunction(Module, Func));

        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1) {\n"
                          "entry:\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store i32 %1, i32* %2, align 4\n"
                          "  %3 = load i32, i32* %2, align 4\n"
                          "  %4 = icmp eq i32 %3, 1\n"
                          "  br i1 %4, label %ifthen, label %else\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 1, i32* %2, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "else:                                             ; preds = %entry\n"
                          "  store i32 2, i32* %2, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "endif:                                            ; preds = %else, %ifthen\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGIfElsifElseBlock) {
        ASTModule *Module = CreateModule();

        // func()
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(Func, aParam));

        // if (a == 1) { a = 11 }
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(Builder.CreateVarRef(aParam));
        ASTBinaryGroupExpr *IfCond = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value1);
        ASTBlockStmt *IfBlock = Builder.CreateBlockStmt(SourceLoc);
        // Create if block
        ASTIfStmt *IfStmt = Builder.CreateIfStmt(SourceLoc, IfCond, IfBlock);

        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 11));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt));

        // elsif (a == 2) { a = 22 }
        ASTBlockStmt *ElsifBlock = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        ASTBinaryGroupExpr *ElsifCond = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value2);
        ASTVarStmt *aVarStmt2 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 22));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt2));

        // elsif (a == 3) { a = 33 }
        ASTBlockStmt *ElsifBlock2 = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value3 = Builder.CreateExpr( Builder.CreateIntegerValue(SourceLoc, 3));
        ASTBinaryGroupExpr *ElsifCond2 = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value3);
        ASTVarStmt *aVarStmt3 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 33));
        EXPECT_TRUE(Builder.AddElsif(IfStmt, ElsifCond2, ElsifBlock2));
        Builder.AddStmt(Body, aVarStmt3);

        // else {a == 44}
        ASTBlockStmt *ElseBlock = Builder.CreateBlockStmt(SourceLoc);
        ASTVarStmt *aVarStmt4 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 44));
        
        EXPECT_TRUE(Builder.AddElse(IfStmt, ElseBlock));

        EXPECT_TRUE(Builder.AddStmt(Body, IfStmt));

        // Add to Module
        EXPECT_TRUE(Builder.AddFunction(Module, Func));

        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1) {\n"
                          "entry:\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store i32 %1, i32* %2, align 4\n"
                          "  %3 = load i32, i32* %2, align 4\n"
                          "  %4 = icmp eq i32 %3, 1\n"
                          "  br i1 %4, label %ifthen, label %elsif\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 11, i32* %2, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif:                                            ; preds = %entry\n"
                          "  %5 = load i32, i32* %2, align 4\n"
                          "  %6 = icmp eq i32 %5, 2\n"
                          "  br i1 %6, label %elsifthen, label %elsif1\n"
                          "\n"
                          "elsifthen:                                        ; preds = %elsif\n"
                          "  store i32 22, i32* %2, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif1:                                           ; preds = %elsif\n"
                          "  %7 = load i32, i32* %2, align 4\n"
                          "  %8 = icmp eq i32 %7, 3\n"
                          "  br i1 %8, label %elsifthen2, label %else\n"
                          "\n"
                          "elsifthen2:                                       ; preds = %elsif1\n"
                          "  store i32 33, i32* %2, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "else:                                             ; preds = %elsif1\n"
                          "  store i32 44, i32* %2, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "endif:                                            ; preds = %else, %elsifthen2, %elsifthen, %ifthen\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGIfElsifBlock) {
        ASTModule *Module = CreateModule();

        // main()
        ASTFunction *MainFn = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                      Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(MainFn);
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(MainFn, aParam));

        // Create if block
        ASTIfStmt *IfStmt = Builder.CreateIfStmt(Body, SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(Body, IfStmt));

        // if a == 1 { a = 11 }
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(Builder.CreateVarRef(aParam));
        ASTBinaryGroupExpr *IfCond = Builder.CreateBinaryExpr(IfStmt, SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value1);
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 11));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt));

        // elsif a == 2 { a = 22 }
        ASTElsif *Elsif = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        ASTBinaryGroupExpr *ElsifCond = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value2);
        ASTVarStmt *aVarStmt2 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 22));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt2));

        // elsif a == 3 { a = 33 }
        ASTElsif *ElsifBlock2 = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value3 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 3));
        ASTBinaryGroupExpr *ElsifCond2 = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value3);
        ASTVarStmt *aVarStmt3 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 33));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt3));

        // Add to Module
        EXPECT_TRUE(Builder.AddFunction(Module, MainFn));

        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(MainFn);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1) {\n"
                          "entry:\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store i32 %1, i32* %2, align 4\n"
                          "  %3 = load i32, i32* %2, align 4\n"
                          "  %4 = icmp eq i32 %3, 1\n"
                          "  br i1 %4, label %ifthen, label %elsif\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 11, i32* %2, align 4\n"
                          "  br label %endif\n"
                          "\nelsif:                                            ; preds = %entry\n"
                          "  %5 = load i32, i32* %2, align 4\n"
                          "  %6 = icmp eq i32 %5, 2\n"
                          "  br i1 %6, label %elsifthen, label %elsif1\n"
                          "\n"
                          "elsifthen:                                        ; preds = %elsif\n"
                          "  store i32 22, i32* %2, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif1:                                           ; preds = %elsif\n"
                          "  %7 = load i32, i32* %2, align 4\n"
                          "  %8 = icmp eq i32 %7, 3\n"
                          "  br i1 %8, label %elsifthen2, label %endif\n"
                          "\n"
                          "elsifthen2:                                       ; preds = %elsif1\n"
                          "  store i32 33, i32* %2, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "endif:                                            ; preds = %elsifthen2, %elsif1, %elsifthen, %ifthen\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGSwitchBlock) {
        ASTModule *Module = CreateModule();

        // main()
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(Func, aParam));

        ASTSwitchStmt *SwitchStmt = Builder.CreateSwitchStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(Body, SwitchStmt));

        // switch a
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(Builder.CreateVarRef(aParam));

        // case 1: a = 1 break
        ASTBlockStmt *Case1Block = Builder.CreateBlockStmt(SourceLoc);
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarStmt *aVarStmt1 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt1));

        // case 2: a = 2 break
        ASTBlockStmt *Case2Block = Builder.CreateBlockStmt(SourceLoc);
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        ASTVarStmt *aVarStmt2 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt2));

        // default: a = 3
        ASTBlockStmt *DefaultBlock = Builder.CreateBlockStmt(SourceLoc);
        ASTVarStmt *aVarStmt3 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 3));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt3));

        // Add to Module
        EXPECT_TRUE(Builder.AddFunction(Module, Func));

        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1) {\n"
                          "entry:\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store i32 %1, i32* %2, align 4\n"
                          "  %3 = load i32, i32* %2, align 4\n"
                          "  switch i32 %3, label %default [\n"
                          "    i32 1, label %case\n"
                          "    i32 2, label %case1\n"
                          "  ]\n"
                          "\n"
                          "case:                                             ; preds = %entry\n"
                          "  store i32 1, i32* %2, align 4\n"
                          "  br label %case1\n"
                          "\n"
                          "case1:                                            ; preds = %entry, %case\n"
                          "  store i32 2, i32* %2, align 4\n"
                          "  br label %endswitch\n"
                          "\n"
                          "default:                                          ; preds = %entry\n"
                          "  store i32 3, i32* %2, align 4\n"
                          "  br label %endswitch\n"
                          "\n"
                          "endswitch:                                        ; preds = %default, %case1\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGWhileBlock) {
        ASTModule *Module = CreateModule();

        // main()
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(Func, aParam));

        // Create while block
        ASTLoopStmt *LoopStmt = Builder.CreateLoopStmt(Body, SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(Body, LoopStmt));

        // while a == 1
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(Builder.CreateVarRef(aParam));
        ASTBinaryGroupExpr *WhileCond = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_EQ, aVarRef, Value1);

        // { a = 1 }
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt));

        // Add to Module
        EXPECT_TRUE(Builder.AddFunction(Module, Func));

        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1) {\n"
                          "entry:\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store i32 %1, i32* %2, align 4\n"
                          "  br label %whilecond\n"
                          "\n"
                          "whilecond:                                        ; preds = %whileloop, %entry\n"
                          "  %3 = load i32, i32* %2, align 4\n"
                          "  %4 = icmp eq i32 %3, 1\n"
                          "  br i1 %4, label %whileloop, label %whileend\n"
                          "\n"
                          "whileloop:                                        ; preds = %whilecond\n"
                          "  store i32 1, i32* %2, align 4\n"
                          "  br label %whilecond\n"
                          "\n"
                          "whileend:                                         ; preds = %whilecond\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGForInitCondPostBlock) {
        ASTModule *Module = CreateModule();

        // main()
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        ASTParam *aParam = Builder.CreateParam(Func, SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(aParam));

        // Create for block
        ASTLoopStmt *ForBlock = Builder.CreateLoopStmt(Body, SourceLoc);
        ASTForLoopBlock *LoopBlock = Builder.CreateForLoopBlock(ForBlock, SourceLoc);
        ASTForPostBlock *PostBlock = Builder.CreateForPostBlock(ForBlock, SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(Body, ForBlock));

        // for int i = 1; i < 1; ++i
        ASTLocalVar *iVar = Builder.CreateLocalVar(SourceLoc, IntType, "i");
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *iVarRef = Builder.CreateExpr(Builder.CreateVarRef(iVar));
        EXPECT_TRUE(Builder.AddStmt(Body, iVar));

        ASTBinaryGroupExpr *ForCond = Builder.CreateBinaryExpr(SourceLoc, ASTBinaryOperatorKind::COMP_LTE, iVarRef, Value1);

        ASTExprStmt *iPreInc = Builder.CreateExprStmt(PostBlock, SourceLoc);
        Builder.CreateUnaryExpr(SourceLoc, ASTUnaryOperatorKind::ARITH_INCR, ASTUnaryOptionKind::UNARY_PRE,
                                 Builder.CreateExpr(Builder.CreateVarRef(iVar)));
        EXPECT_TRUE(Builder.AddStmt(Body, iPreInc));

        // { a = 1}
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(LoopBlock, Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt));

        // Add to Module
        EXPECT_TRUE(Builder.AddFunction(Func));

        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1) {\n"
                          "entry:\n"
                          "  %2 = alloca i32, align 4\n"
                          "  %3 = alloca i32, align 4\n"
                          "  store i32 %1, i32* %2, align 4\n"
                          "  store i32 1, i32* %3, align 4\n"
                          "  br label %forcond\n"
                          "\n"
                          "forcond:                                          ; preds = %forpost, %entry\n"
                          "  %4 = load i32, i32* %3, align 4\n"
                          "  %5 = icmp sle i32 %4, 1\n"
                          "  br i1 %5, label %forloop, label %endfor\n"
                          "\n"
                          "forloop:                                          ; preds = %forcond\n"
                          "  store i32 1, i32* %2, align 4\n"
                          "  br label %forpost\n"
                          "\n"
                          "forpost:                                          ; preds = %forloop\n"
                          "  %6 = load i32, i32* %3, align 4\n"
                          "  %7 = add nsw i32 %6, 1\n"
                          "  store i32 %7, i32* %3, align 4\n"
                          "  br label %forcond\n"
                          "\n"
                          "endfor:                                           ; preds = %forcond\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGForCondBlock) {
        ASTModule *Module = CreateModule();

        // main()
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        ASTParam *aParam = Builder.CreateParam(Func, SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(aParam));

        // Create for block
        ASTForBlock *ForBlock = Builder.CreateForBlock(Body, SourceLoc);
        ASTForLoopBlock *LoopBlock = Builder.CreateForLoopBlock(ForBlock, SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(Body, ForBlock));

        // for a < 1
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(ForBlock, Builder.CreateVarRef(aParam));
        ASTValueExpr *Value1 = Builder.CreateExpr(ForBlock, Builder.CreateIntegerValue(SourceLoc, 1));
        ASTBinaryGroupExpr *ForCond = Builder.CreateBinaryExpr(ForBlock, SourceLoc, ASTBinaryOperatorKind::COMP_LTE, aVarRef, Value1);

        // { a = 1}
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(LoopBlock, Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt));

        // Add to Module
        EXPECT_TRUE(Builder.AddFunction(Func));

        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1) {\n"
                          "entry:\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store i32 %1, i32* %2, align 4\n"
                          "  br label %forcond\n"
                          "\n"
                          "forcond:                                          ; preds = %forloop, %entry\n"
                          "  %3 = load i32, i32* %2, align 4\n"
                          "  %4 = icmp sle i32 %3, 1\n"
                          "  br i1 %4, label %forloop, label %endfor\n"
                          "\n"
                          "forloop:                                          ; preds = %forcond\n"
                          "  store i32 1, i32* %2, align 4\n"
                          "  br label %forcond\n"
                          "\n"
                          "endfor:                                           ; preds = %forcond\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGForPostBlock) {
        ASTModule *Module = CreateModule();

        // main()
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        ASTParam *aParam = Builder.CreateParam(Func, SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(aParam));

        // Create for block
        ASTForBlock *ForBlock = Builder.CreateForBlock(Body, SourceLoc);
        ASTForLoopBlock *LoopBlock = Builder.CreateForLoopBlock(ForBlock, SourceLoc);
        ASTForPostBlock *PostBlock = Builder.CreateForPostBlock(ForBlock, SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(Body, ForBlock));

        // for a < 1; ++a
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(ForBlock, Builder.CreateVarRef(aParam));
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTBinaryGroupExpr *ForCond = Builder.CreateBinaryExpr(ForBlock, SourceLoc, ASTBinaryOperatorKind::COMP_LTE, aVarRef, Value1);

        ASTExprStmt *aPreInc = Builder.CreateExprStmt(PostBlock, SourceLoc);
        Builder.CreateUnaryExpr(SourceLoc, ASTUnaryOperatorKind::ARITH_INCR, ASTUnaryOptionKind::UNARY_PRE, aVarRef);
        EXPECT_TRUE(Builder.AddStmt(Body, aPreInc));

        // { a = 1}
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(LoopBlock, Builder.CreateVarRef(aParam));
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt));

        // Add to Module
        EXPECT_TRUE(Builder.AddFunction(Module, Func));

        EXPECT_TRUE(S->Resolve());

        // Generate Code
        CodeGenFunction *CGF = CGM->GenFunction(Func);
        CGF->GenBody();
        Function *F = CGF->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @func(%error* %0, i32 %1) {\n"
                          "entry:\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store i32 %1, i32* %2, align 4\n"
                          "  br label %forcond\n"
                          "\n"
                          "forcond:                                          ; preds = %forpost, %entry\n"
                          "  %3 = load i32, i32* %2, align 4\n"
                          "  %4 = icmp sle i32 %3, 1\n"
                          "  br i1 %4, label %forloop, label %endfor\n"
                          "\n"
                          "forloop:                                          ; preds = %forcond\n"
                          "  store i32 1, i32* %2, align 4\n"
                          "  br label %forpost\n"
                          "\n"
                          "forpost:                                          ; preds = %forloop\n"
                          "  %5 = load i32, i32* %2, align 4\n"
                          "  %6 = add nsw i32 %5, 1\n"
                          "  store i32 %6, i32* %2, align 4\n"
                          "  br label %forcond\n"
                          "\n"
                          "endfor:                                           ; preds = %forcond\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGClassOnlyMethods) {
        ASTModule *Module = CreateModule();

        // TestClass {
        //   int a() { return 1 }
        //   public int b() { return 1 }
        //   private const int c { return 1 }
        // }
        ASTClass *TestClass = Builder.CreateClass(ASTClassKind::CLASS,
                                                   Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false),
                                                   SourceLoc, "TestClass");

        // int a() { return 1 }
        ASTClassMethod *aFunc = Builder.CreateClassMethod(SourceLoc, IntType,
                                                             "a",
                                                             Builder.CreateScopes(
                                                                     ASTVisibilityKind::V_DEFAULT, false, false));
        ASTBlockStmt *aFuncBody = Builder.CreateBody(aFunc);
        ASTReturnStmt *aFuncReturn = Builder.CreateReturnStmt(SourceLoc);
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLocation(), 1));
        Builder.AddStmt(Body, aFuncReturn);
        Builder.AddClassMethod(aFunc);

        // public int b() { return 1 }
        ASTClassMethod *bFunc = Builder.CreateClassMethod(SourceLoc, IntType,
                                                             "b",
                                                             Builder.CreateScopes(
                                                                     ASTVisibilityKind::V_PUBLIC, false, false));
        ASTBlockStmt *bFuncBody = Builder.CreateBody(bFunc);
        ASTReturnStmt *bFuncReturn = Builder.CreateReturnStmt(SourceLoc);
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLocation(), 1));
        Builder.AddStmt(Body, bFuncReturn);
        Builder.AddClassMethod(bFunc);

        // private const int c { return 1 }
        ASTClassMethod *cFunc = Builder.CreateClassMethod(SourceLoc, IntType,
                                                             "c",
                                                             Builder.CreateScopes(
                                                                     ASTVisibilityKind::V_PRIVATE, true, false));
        ASTBlockStmt *cFuncBody = Builder.CreateBody(cFunc);
        ASTReturnStmt *cFuncReturn = Builder.CreateReturnStmt(SourceLoc);
        Builder.CreateExpr(Builder.CreateIntegerValue(SourceLocation(), 1));
        Builder.AddStmt(Body, cFuncReturn);
        Builder.AddClassMethod(cFunc);

        // int main() {
        //  TestClass test = new TestClass()
        //  int a = test.a()
        //  int b = test.b()
        //  int c = test.c()
        //  delete test
        // }
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // TestClass test = new TestClass()
        ASTType *TestClassType = Builder.CreateClassType(TestClass);
        ASTLocalVar *TestVar = Builder.CreateLocalVar(SourceLoc, TestClassType, "test");
        ASTClassMethod *DefaultConstructor = TestClass->getConstructors().find(0)->second.front();
        ASTVarRef *Instance = Builder.CreateVarRef(TestVar);
        ASTCall *ConstructorCall = Builder.CreateCall(Instance, DefaultConstructor);
        ASTCallExpr *NewExpr = Builder.CreateNewExpr(ConstructorCall);
        Builder.AddExpr(NewExpr);
        Builder.AddStmt(Body, TestVar);

        // int a = test.a()
        ASTType *aType = aFunc->getType();
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, aType, "a");
        ASTCallExpr *aCallExpr = Builder.CreateExpr(Builder.CreateCall(Instance, aFunc));
        Builder.AddExpr(aVar, aCallExpr);
        Builder.AddStmt(Body, aVar);

        // int b = test.b()
        ASTType *bType = bFunc->getType();
        ASTLocalVar *bVar = Builder.CreateLocalVar(SourceLoc, bType, "b");
        ASTCallExpr *bCallExpr = Builder.CreateExpr(Builder.CreateCall(Instance, bFunc));
        Builder.AddExpr(bCallExpr);
        Builder.AddStmt(Body, bVar);

        // int c = test.c()
        ASTType *cType = cFunc->getType();
        ASTLocalVar *cVar = Builder.CreateLocalVar(SourceLoc, cType, "c");
        ASTCallExpr *cCallExpr = Builder.CreateExpr(cVar, Builder.CreateCall(Instance, cFunc));
        Builder.AddExpr(cVar, cCallExpr);
        Builder.AddStmt(Body, cVar);

        // delete test
        ASTDeleteStmt *Delete = Builder.CreateDeleteStmt(Body, SourceLoc, (ASTVarRef *) Instance);
        Builder.AddStmt(Body, Delete);

        // Add to Module
        EXPECT_TRUE(Builder.AddIdentity(TestClass));
        EXPECT_TRUE(Builder.AddFunction(Func));

        bool Success = S->Resolve();
        EXPECT_TRUE(Success);

        if (Success) {

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
            std::string output = getOutput();

            EXPECT_EQ(output, "%error = type { i8, i32, i8* }\n"
                              "%TestClass = type { %TestClass_vtable* }\n"
                              "%TestClass_vtable = type { i32 (%error*, %TestClass*), i32 (%error*, %TestClass*), i32 (%error*, %TestClass*) }\n"
                              "\n"
                              "define void @TestClass_TestClass(%error* %0, %TestClass* %1) {\n"
                              "entry:\n"
                              "  %2 = alloca %TestClass*, align 8\n"
                              "  store %TestClass* %1, %TestClass** %2, align 8\n"
                              "  %3 = load %TestClass*, %TestClass** %2, align 8\n"
                              "  ret void\n"
                              "}\n"
                              "\n"
                              "define i32 @TestClass_a(%error* %0, %TestClass* %1) {\n"
                              "entry:\n"
                              "  %2 = alloca %TestClass*, align 8\n"
                              "  store %TestClass* %1, %TestClass** %2, align 8\n"
                              "  %3 = load %TestClass*, %TestClass** %2, align 8\n"
                              "  ret i32 1\n"
                              "}\n"
                              "\n"
                              "define i32 @TestClass_b(%error* %0, %TestClass* %1) {\n"
                              "entry:\n"
                              "  %2 = alloca %TestClass*, align 8\n"
                              "  store %TestClass* %1, %TestClass** %2, align 8\n"
                              "  %3 = load %TestClass*, %TestClass** %2, align 8\n"
                              "  ret i32 1\n"
                              "}\n"
                              "\n"
                              "define i32 @TestClass_c(%error* %0, %TestClass* %1) {\n"
                              "entry:\n"
                              "  %2 = alloca %TestClass*, align 8\n"
                              "  store %TestClass* %1, %TestClass** %2, align 8\n"
                              "  %3 = load %TestClass*, %TestClass** %2, align 8\n"
                              "  ret i32 1\n"
                              "}\n"
                              "\n"
                              "define void @func(%error* %0) {\n"
                              "entry:\n"
                              "  %1 = alloca %TestClass, align 8\n"
                              "  %2 = alloca i32, align 4\n"
                              "  %3 = alloca i32, align 4\n"
                              "  %4 = alloca i32, align 4\n"
                              "  %5 = alloca %TestClass, align 8\n"
                              "  call void @TestClass_TestClass(%error* %0, %TestClass* %5)\n"
                              "  %6 = call i32 @TestClass_a(%error* %0, %TestClass* %5)\n"
                              "  store i32 %6, i32* %2, align 4\n"
                              "  %7 = call i32 @TestClass_b(%error* %0, %TestClass* %5)\n"
                              "  store i32 %7, i32* %3, align 4\n"
                              "  %8 = call i32 @TestClass_c(%error* %0, %TestClass* %5)\n"
                              "  store i32 %8, i32* %4, align 4\n"
                              "  ret void\n"
                              "}\n");
        }
    }

    TEST_F(CodeGenTest, CGClass) {
        ASTModule *Module = CreateModule();

        // TestClass {
        //   int a
        //   int getA() { return a }
        // }
        ASTClass *TestClass = Builder.CreateClass(ASTClassKind::CLASS,
                                                   Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false),
                                                   SourceLoc, "TestClass");
        ASTClassAttribute *aField = Builder.CreateClassAttribute(SourceLoc, Builder.CreateIntType(SourceLoc),
                                                      "a",
                                                      Builder.CreateScopes(
                                                              ASTVisibilityKind::V_DEFAULT, false, false));
        Builder.AddClassAttribute(TestClass, aField);


        // int getA() { return a }
        ASTClassMethod *getA = Builder.CreateClassMethod(SourceLoc, IntType,
                                                            "getA",
                                                            Builder.CreateScopes(
                                                                     ASTVisibilityKind::V_DEFAULT, false, false));
        ASTBlockStmt *aFuncBody = Builder.CreateBody(getA);
        ASTReturnStmt *aFuncReturn = Builder.CreateReturnStmt(SourceLoc);
        Builder.CreateExpr(aFuncReturn, Builder.CreateVarRef(aField));
        Builder.AddStmt(Body, aFuncReturn);
        Builder.AddClassMethod(getA);

        // int main() {
        //  TestClass test = new TestClass()
        //  int a = test.getA()
        //  delete test
        // }
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // TestClass test = new TestClass()
        ASTType *TestClassType = Builder.CreateClassType(TestClass);
        ASTLocalVar *TestVar = Builder.CreateLocalVar(SourceLoc, TestClassType, "test");
        ASTClassMethod *DefaultConstructor = TestClass->getConstructors().find(0)->second.front();
        ASTVarRef *Instance = Builder.CreateVarRef(TestVar);
        ASTCall *ConstructorCall = Builder.CreateCall(Instance, DefaultConstructor);
        ASTCallExpr *NewExpr = Builder.CreateNewExpr(TestVar, ConstructorCall);
        Builder.AddExpr(TestVar, NewExpr);
        Builder.AddStmt(Body, TestVar);

        // int a = test.a()
        ASTType *aType = getA->getType();
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, aType, "a");
        ASTCallExpr *aCallExpr = Builder.CreateExpr(aVar, Builder.CreateCall(Instance, getA));
        Builder.AddExpr(aVar, aCallExpr);
        Builder.AddStmt(Body, aVar);

        // delete test
        ASTDeleteStmt *Delete = Builder.CreateDeleteStmt(Body, SourceLoc, (ASTVarRef *) Instance);
        Builder.AddStmt(Body, Delete);

        // Add to Module
        EXPECT_TRUE(Builder.AddIdentity(TestClass));
        EXPECT_TRUE(Builder.AddFunction(Func));

        bool Success = S->Resolve();
        EXPECT_TRUE(Success);

        if (Success) {

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
            std::string output = getOutput();

            EXPECT_EQ(output, "%error = type { i8, i32, i8* }\n"
                              "%TestClass = type { %TestClass_vtable*, i32 }\n"
                              "%TestClass_vtable = type { i32 (%error*, %TestClass*) }\n"
                              "\n"
                              "define void @TestClass_TestClass(%error* %0, %TestClass* %1) {\n"
                              "entry:\n"
                              "  %2 = alloca %TestClass*, align 8\n"
                              "  store %TestClass* %1, %TestClass** %2, align 8\n"
                              "  %3 = load %TestClass*, %TestClass** %2, align 8\n"
                              "  %4 = getelementptr inbounds %TestClass, %TestClass* %3, i32 0, i32 1\n"
                              "  %5 = load i32, i32* %4, align 4\n"
                              "  store i32 0, i32 %5, align 4\n"
                              "  ret void\n"
                              "}\n"
                              "\n"
                              "define i32 @TestClass_getA(%error* %0, %TestClass* %1) {\n"
                              "entry:\n"
                              "  %2 = alloca %TestClass*, align 8\n"
                              "  store %TestClass* %1, %TestClass** %2, align 8\n"
                              "  %3 = load %TestClass*, %TestClass** %2, align 8\n"
                              "  %4 = getelementptr inbounds %TestClass, %TestClass* %3, i32 0, i32 1\n"
                              "  %5 = load i32, i32* %4, align 4\n"
                              "  ret i32 %5\n"
                              "}\n"
                              "\n"
                              "define void @func(%error* %0) {\n"
                              "entry:\n"
                              "  %1 = alloca %TestClass, align 8\n"
                              "  %2 = alloca i32, align 4\n"
                              "  %malloccall = tail call i8* @malloc(i32 ptrtoint (%TestClass* getelementptr (%TestClass, %TestClass* null, i32 1) to i32))\n"
                              "  %TestClassInst = bitcast i8* %malloccall to %TestClass*\n"
                              "  call void @TestClass_TestClass(%error* %0, %TestClass* %TestClassInst)\n"
                              "  %3 = call i32 @TestClass_getA(%error* %0, %TestClass* %TestClassInst)\n"
                              "  store i32 %3, i32* %2, align 4\n"
                              "  %4 = bitcast %TestClass* %TestClassInst to i8*\n"
                              "  tail call void @free(i8* %4)\n"
                              "  ret void\n"
                              "}\n"
                              "\n"
                              "declare noalias i8* @malloc(i32)\n"
                              "\n"
                              "declare void @free(i8*)\n");
        }
    }

    TEST_F(CodeGenTest, CGStruct) {
        ASTModule *Module = CreateModule();

        // struct TestStruct {
        //   int a
        //   int b
        //   const int c
        // }
        ASTClass *TestStruct = Builder.CreateClass(ASTClassKind::STRUCT,
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false),
                                                    SourceLoc, "TestStruct");
        ASTClassAttribute *aField = Builder.CreateClassAttribute(SourceLoc, Builder.CreateIntType(SourceLoc),
                                                      "a",
                                                      Builder.CreateScopes(
                                                              ASTVisibilityKind::V_DEFAULT, false, false));
        Builder.AddClassAttribute(TestStruct, aField);

        ASTClassAttribute *bField = Builder.CreateClassAttribute(SourceLoc, Builder.CreateIntType(SourceLoc),
                                                      "b",
                                                      Builder.CreateScopes(
                                                              ASTVisibilityKind::V_DEFAULT, false, false));
        Builder.AddClassAttribute(TestStruct, bField);

        ASTClassAttribute *cField = Builder.CreateClassAttribute(SourceLoc, Builder.CreateIntType(SourceLoc),
                                                      "c",
                                                      Builder.CreateScopes(
                                                              ASTVisibilityKind::V_DEFAULT, true, false));
        Builder.AddClassAttribute(TestStruct, cField);

        // int main() {
        //  TestStruct test = new TestStruct();
        //  int a = test.a
        //  test.b = 2
        //  int c = test.c
        //  return 1
        // }
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // TestStruct test = new TestStruct()
        ASTType *TestClassType = Builder.CreateClassType(TestStruct);
        ASTLocalVar *TestVar = Builder.CreateLocalVar(SourceLoc, TestClassType, "test");
        ASTClassMethod *DefaultConstructor = TestStruct->getConstructors().find(0)->second.front();
        ASTVarRef *Instance = (ASTVarRef *) Builder.CreateVarRef(TestVar);
        ASTCallExpr *NewExpr = Builder.CreateNewExpr(TestVar, Builder.CreateCall(Instance, DefaultConstructor));
        Builder.AddExpr(TestVar, NewExpr);
        Builder.AddStmt(Body, TestVar);

        // int a = test.a
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, IntType, "a");
        ASTVarRefExpr *aRefExpr = Builder.CreateExpr(aVar, Builder.CreateVarRef(Instance, aField));
        Builder.AddStmt(Body, aVar);

        // test.b = 2
        ASTVarStmt *bFieldAssign = Builder.CreateVarStmt(Builder.CreateVarRef(Instance, bField));
        Builder.CreateExpr(bFieldAssign, Builder.CreateIntegerValue(SourceLoc, 2));
        Builder.AddStmt(Body, bFieldAssign);

        // int c = test.c
        ASTLocalVar *cVar = Builder.CreateLocalVar(SourceLoc, IntType, "c");
        ASTVarRefExpr *cRefExpr = Builder.CreateExpr(cVar, Builder.CreateVarRef(Instance, cField));
        Builder.AddStmt(Body, cVar);

        // delete test
        ASTDeleteStmt *Delete = Builder.CreateDeleteStmt(SourceLoc, (ASTVarRef *) Instance);
        Builder.AddStmt(Body, Delete);

        // Add to Module
        EXPECT_TRUE(Builder.AddIdentity(TestStruct));
        EXPECT_TRUE(Builder.AddFunction(Func));

        bool Success = S->Resolve();
        EXPECT_TRUE(Success);

        if (Success) {
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
            std::string output = getOutput();

            EXPECT_EQ(output, "%TestStruct = type { i32, i32, i32 }\n"
                              "%error = type { i8, i32, i8* }\n"
                              "\n"
                              "define void @TestStruct_TestStruct(%TestStruct* %0) {\n"
                              "entry:\n"
                              "  %1 = alloca %TestStruct*, align 8\n"
                              "  store %TestStruct* %0, %TestStruct** %1, align 8\n"
                              "  %2 = load %TestStruct*, %TestStruct** %1, align 8\n"
                              "  %3 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 0\n"
                              "  %4 = load i32, i32* %3, align 4\n"
                              "  store i32 0, i32 %4, align 4\n"
                              "  %5 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 1\n"
                              "  %6 = load i32, i32* %5, align 4\n"
                              "  store i32 0, i32 %6, align 4\n"
                              "  %7 = getelementptr inbounds %TestStruct, %TestStruct* %2, i32 0, i32 2\n"
                              "  %8 = load i32, i32* %7, align 4\n"
                              "  store i32 0, i32 %8, align 4\n"
                              "  ret void\n"
                              "}\n"
                              "\n"
                              "define void @func(%error* %0) {\n"
                              "entry:\n"
                              "  %1 = alloca %TestStruct, align 8\n"
                              "  %2 = alloca i32, align 4\n"
                              "  %3 = alloca i32, align 4\n"
                              "  %malloccall = tail call i8* @malloc(i32 trunc (i64 mul nuw (i64 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i64), i64 3) to i32))\n"
                              "  %TestStructInst = bitcast i8* %malloccall to %TestStruct*\n"
                              "  call void @TestStruct_TestStruct(%TestStruct* %TestStructInst)\n"
                              "  %4 = getelementptr inbounds %TestStruct, %TestStruct* %TestStructInst, i32 0, i32 0\n"
                              "  %5 = load i32, i32* %4, align 4\n"
                              "  store i32 %5, i32* %2, align 4\n"
                              "  %6 = getelementptr inbounds %TestStruct, %TestStruct* %TestStructInst, i32 0, i32 1\n"
                              "  %7 = load i32, i32* %6, align 4\n"
                              "  store i32 2, i32 %7, align 4\n"
                              "  %8 = getelementptr inbounds %TestStruct, %TestStruct* %TestStructInst, i32 0, i32 2\n"
                              "  %9 = load i32, i32* %8, align 4\n"
                              "  store i32 %9, i32* %3, align 4\n"
                              "  %10 = bitcast %TestStruct* %TestStructInst to i8*\n"
                              "  tail call void @free(i8* %10)\n"
                              "  ret void\n"
                              "}\n"
                              "\n"
                              "declare noalias i8* @malloc(i32)\n"
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
        ASTEnum *TestEnum = Builder.CreateEnum(SourceLoc, "TestEnum", Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTEnumEntry *A = Builder.CreateEnumEntry(SourceLoc, "A");
        Builder.AddEnumEntry(TestEnum, A);
        ASTEnumEntry *B = Builder.CreateEnumEntry(SourceLoc, "B");
        Builder.AddEnumEntry(TestEnum, B);
        ASTEnumEntry *C = Builder.CreateEnumEntry(SourceLoc, "C");
        Builder.AddEnumEntry(TestEnum, C);

        // int main() {
        //  TestEnum a = TestEnum.A;
        //  TestEnum b = a
        //  return 1
        // }
        ASTFunction *Func = Builder.CreateFunction(SourceLoc, VoidType, "func",
                                                    Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false));
        ASTBlockStmt *Body = Builder.CreateBody(Func);

        ASTType *TestEnumType = Builder.CreateEnumType(TestEnum);

        //  TestEnum a = TestEnum.A;
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, TestEnumType, "a");
        ASTVarRefExpr *aRefExpr = Builder.CreateExpr(aVar, Builder.CreateVarRef(A));
        Builder.AddStmt(Body, aVar);

        //  TestEnum b = a
        ASTLocalVar *bVar = Builder.CreateLocalVar(SourceLoc, TestEnumType, "b");
        ASTVarRefExpr *bRefExpr = Builder.CreateExpr(bVar, Builder.CreateVarRef(aVar));
        Builder.AddStmt(Body, bVar);

        // Add to Module
        EXPECT_TRUE(Builder.AddIdentity(TestEnum));
        EXPECT_TRUE(Builder.AddFunction(Func));

        bool Success = S->Resolve();
        EXPECT_TRUE(Success);

        if (Success) {
            // Generate Code
            CodeGenEnum *CGC = CGM->GenEnum(TestEnum);
            CodeGenFunction *CGF = CGM->GenFunction(Func);
            CGF->GenBody();

            EXPECT_FALSE(Diags.hasErrorOccurred());
            std::string output = getOutput();

            EXPECT_EQ(output, "%error = type { i8, i32, i8* }\n"
                              "\n"
                              "define void @func(%error* %0) {\n"
                              "entry:\n"
                              "  %1 = alloca i32, align 4\n"
                              "  %2 = alloca i32, align 4\n"
                              "  store i32 1, i32* %1, align 4\n"
                              "  %3 = load i32, i32* %1, align 4\n"
                              "  store i32 %3, i32* %2, align 4\n"
                              "  ret void\n"
                              "}\n");
        }
    }

    TEST_F(CodeGenTest, CGError) {
        ASTModule *Module = CreateModule();
        ASTScopes *TopScopes = Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false);

        // int testFail0() {
        //   fail()
        // }
        ASTFunction *TestFail0 = Builder.CreateFunction(SourceLoc, IntType, "testFail0", TopScopes);
        ASTBlockStmt *Body0 = Builder.CreateBody(TestFail0);
        ASTExprStmt *Fail0 = Builder.CreateExprStmt(SourceLoc);
        Builder.CreateFailStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(Body0, Fail0));
        EXPECT_TRUE(Builder.AddFunction(Module, TestFail0));

        // int testFail1() {
        //   fail(true)
        // }
        ASTFunction *TestFail1 = Builder.CreateFunction(SourceLoc, IntType, "testFail1", TopScopes);
        ASTBlockStmt *Body1 = Builder.CreateBody(TestFail1);
        ASTExprStmt *Fail1 = Builder.CreateExprStmt(SourceLoc);
        ASTBoolValue *BoolVal = Builder.CreateBoolValue(SourceLoc, true);
        ASTValueExpr *BoolValExpr = Builder.CreateExpr(BoolVal);
        Builder.CreateFailStmt(Fail1);
        EXPECT_TRUE(Builder.AddStmt(Body1, Fail1));
        EXPECT_TRUE(Builder.AddFunction(Module, TestFail1));

        // int testFail2() {
        //   fail(1)
        // }
        ASTFunction *TestFail2 = Builder.CreateFunction(SourceLoc, IntType, "testFail2", TopScopes);
        ASTBlockStmt *Body2 = Builder.CreateBody(TestFail2);
        ASTExprStmt *Fail2 = Builder.CreateExprStmt(SourceLoc);
        ASTIntegerValue *IntVal = Builder.CreateIntegerValue(SourceLoc, 1);
        ASTValueExpr *IntValExpr = Builder.CreateExpr(IntVal);
        Builder.CreateFailStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(Body2, Fail2));
        EXPECT_TRUE(Builder.AddFunction(Module, TestFail2));

        // int testFail3() {
        //  fail("Error")
        // }
        ASTFunction *TestFail3 = Builder.CreateFunction(SourceLoc, IntType, "testFail3", TopScopes);
        ASTBlockStmt *Body3 = Builder.CreateBody(TestFail3);
        ASTExprStmt *Fail3 = Builder.CreateExprStmt(SourceLoc);
        ASTStringValue *StrVal = Builder.CreateStringValue(SourceLoc, "Error");
        ASTValueExpr *StrValExpr = Builder.CreateExpr(StrVal);
        Builder.CreateFailStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(Body3, Fail3));
        EXPECT_TRUE(Builder.AddFunction(Module, TestFail3));

        ASTFunction *Main = Builder.CreateFunction(SourceLoc, VoidType, "main", TopScopes);
        ASTBlockStmt *MainBody = Builder.CreateBody(Main);

        // main() {
        //   testFail0()
        //   testFail1()
        //   testFail2()
        //   testFail3()
        // }

        // call test1()
        ASTExprStmt *CallTestFail0 = Builder.CreateExprStmt(SourceLoc);
        ASTCallExpr *CallExpr0 = Builder.CreateExpr(Builder.CreateCall(TestFail0));
        Builder.AddExpr(CallTestFail0, CallExpr0);
        EXPECT_TRUE(Builder.AddStmt(MainBody, CallTestFail0));

        // call test1()
        ASTExprStmt *CallTestFail1 = Builder.CreateExprStmt(SourceLoc);
        ASTCallExpr *CallExpr1 = Builder.CreateExpr(Builder.CreateCall(TestFail1));
        Builder.AddExpr(CallTestFail1, CallExpr1);
        EXPECT_TRUE(Builder.AddStmt(MainBody, CallTestFail1));

        // call test2()
        ASTExprStmt *CallTestFail2 = Builder.CreateExprStmt(SourceLoc);
        ASTCallExpr *CallExpr2 = Builder.CreateExpr(Builder.CreateCall(TestFail2));
        Builder.AddExpr(CallTestFail2, CallExpr2);
        EXPECT_TRUE(Builder.AddStmt(MainBody, CallTestFail2));

        // call test2()
        ASTExprStmt *CallTestFail3 = Builder.CreateExprStmt(SourceLoc);
        ASTCallExpr *CallExpr3 = Builder.CreateExpr(Builder.CreateCall(TestFail3));
        Builder.AddExpr(CallTestFail3, CallExpr3);
        EXPECT_TRUE(Builder.AddStmt(MainBody, CallTestFail3));

        EXPECT_TRUE(Builder.AddFunction(Module, Main));
        EXPECT_TRUE(S->Resolve());

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

        CodeGenFunction *CGF_Main = CGM->GenFunction(Main);
        CGF_Main->GenBody();
        llvm::Function *F_Main = CGF_Main->getFunction();

        EXPECT_FALSE(Diags.hasErrorOccurred());
        testing::internal::CaptureStdout();
        F_TestFail0->print(llvm::outs());
        F_TestFail1->print(llvm::outs());
        F_TestFail2->print(llvm::outs());
        F_TestFail3->print(llvm::outs());
        F_Main->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @testFail0(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = getelementptr inbounds %error, %error* %0, i32 0, i32 0\n"
                          "  store i8 1, i8* %1, align 1\n"
                          "  ret i32 0\n"
                          "}\n"
                          "define i32 @testFail1(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = getelementptr inbounds %error, %error* %0, i32 0, i32 0\n"
                          "  store i8 1, i8* %1, align 1\n"
                          "  ret i32 0\n"
                          "}\n"
                          "define i32 @testFail2(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = getelementptr inbounds %error, %error* %0, i32 0, i32 0\n"
                          "  store i8 2, i8* %1, align 1\n"
                          "  %2 = getelementptr inbounds %error, %error* %0, i32 0, i32 1\n"
                          "  store i32 1, i32* %2, align 4\n"
                          "  ret i32 0\n"
                          "}\n"
                          "define i32 @testFail3(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = getelementptr inbounds %error, %error* %0, i32 0, i32 0\n"
                          "  store i8 3, i8* %1, align 1\n"
                          "  %2 = getelementptr inbounds %error, %error* %0, i32 0, i32 2\n"
                          "  store i8* getelementptr inbounds ([6 x i8], [6 x i8]* @0, i32 0, i32 0), i8** %2, align 8\n"
                          "  ret i32 0\n"
                          "}\n"
                          "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca %error, align 8\n"
                          "  %1 = call i32 @testFail0(%error* %0)\n"
                          "  %2 = call i32 @testFail1(%error* %0)\n"
                          "  %3 = call i32 @testFail2(%error* %0)\n"
                          "  %4 = call i32 @testFail3(%error* %0)\n"
                          "  %5 = getelementptr inbounds %error, %error* %0, i32 0, i32 0\n"
                          "  %6 = load i8, i8* %5, align 1\n"
                          "  %7 = icmp ne i8 %6, 0\n"
                          "  %8 = zext i1 %7 to i32\n"
                          "  ret i32 %8\n"
                          "}\n");
    }
} // anonymous namespace
