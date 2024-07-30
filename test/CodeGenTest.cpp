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
#include "Sema/Sema.h"
#include "Sema/SemaBuilder.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
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
#include "AST/ASTEnumType.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTExpr.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTGroupExpr.h"
#include "AST/ASTOperatorExpr.h"
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
        llvm::SmallVector<ASTScope *, 8> DefaultScopes;

        CodeGenTest() : CI(*TestUtils::CreateCompilerInstance()),
                        CG(TestUtils::CreateCodeGen(CI)),
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
                                                       Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 0)))),
                        DefaultScopes(Builder.CreateScopes(ASTVisibilityKind::V_PUBLIC, false)) {
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmPrinters();
        }

        ASTModule *CreateModule(std::string Name = "test") {
            Diags.getClient()->BeginSourceFile();
            auto *Module = Builder.CreateModule(Name);
            Diags.getClient()->EndSourceFile();
            return Module;
        }

        virtual ~CodeGenTest() {
            llvm::outs().flush();
        }

        std::string getOutput(llvm::Module *Module) {
            testing::internal::CaptureStdout();
            Module->print(llvm::outs(), nullptr);
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
        llvm::SmallVector<ASTScope *, 8> TopScopes = Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false);

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

        llvm::SmallVector<ASTScope *, 8> TopScopes = Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false);

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
        ASTArrayValue *ArrayValEmpty = Builder.CreateArrayValue(SourceLoc);
        ASTGlobalVar *kVar = Builder.CreateGlobalVar(Module, SourceLoc, ArrayInt0Type, "k", TopScopes, Builder.CreateExpr(ArrayValEmpty));

        // l (array with 2 val)
        ASTArrayValue *ArrayVal = Builder.CreateArrayValue(SourceLoc);
        Builder.AddArrayValue(ArrayVal, Builder.CreateIntegerValue(SourceLoc, 1)); // ArrayVal = {1}
        Builder.AddArrayValue(ArrayVal, Builder.CreateIntegerValue(SourceLoc, 2)); // ArrayVal = {1, 1}
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

        llvm::SmallVector<ASTScope *, 8> TopScopes = Builder.CreateScopes(ASTVisibilityKind::V_DEFAULT, false);
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", TopScopes);
        // func(int P1, float P2, bool P3, long P4, double P5, byte P6, short P7, ushort P8, uint P9, ulong P10) {
        // }

        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, IntType, "P1")));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, FloatType, "P2")));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, BoolType, "P3")));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, LongType, "P4")));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, DoubleType, "P5")));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, ByteType, "P6")));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, ShortType, "P7")));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, UShortType, "P8")));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, UIntType, "P9")));
        EXPECT_TRUE(Builder.AddParam(Func, Builder.CreateParam(SourceLoc, ULongType, "P10")));
        ASTBlockStmt *Body = Builder.CreateBody(Func);

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
        ASTGlobalVar *GVar = Builder.CreateGlobalVar(Module, SourceLoc, FloatType, "G", DefaultScopes,
                                                     Builder.CreateExpr(FloatingVal));

        // func()
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, IntType, "func", DefaultScopes);
        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // int A = 1
        ASTLocalVar *VarA = Builder.CreateLocalVar(SourceLoc, IntType, "A");
        EXPECT_TRUE(Builder.AddLocalVar(Body, VarA));
        ASTVarStmt *VarAStmt = Builder.CreateVarStmt(VarA);
        ASTExpr *ExprA = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder.AddExpr(VarAStmt, ExprA));
        EXPECT_TRUE(Builder.AddStmt(Body, VarAStmt));

        // GlobalVar
        // G = 1
        ASTVarRef *VarRefG = Builder.CreateVarRef(GVar);
        ASTVarStmt * GVarStmt = Builder.CreateVarStmt(VarRefG);
        ASTExpr *ExprG = Builder.CreateExpr(Builder.CreateFloatingValue(SourceLoc, 1));
        EXPECT_TRUE(Builder.AddExpr(GVarStmt, ExprG));
        EXPECT_TRUE(Builder.AddStmt(Body, GVarStmt));

        // return A
        ASTReturnStmt *Return = Builder.CreateReturnStmt(SourceLoc);
        ASTExpr *ExprRA = Builder.CreateExpr(Builder.CreateVarRef(VarA));
        EXPECT_TRUE(Builder.AddExpr(Return, ExprRA));
        EXPECT_TRUE(Builder.AddStmt(Body, Return));

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
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        // int a = 1
        ASTLocalVar *LocalVar = Builder.CreateLocalVar(SourceLoc, Builder.CreateIntType(SourceLoc), "a");
        EXPECT_TRUE(Builder.AddLocalVar(Body, LocalVar));
        ASTVarStmt *VarStmt = Builder.CreateVarStmt(LocalVar);
        ASTValueExpr *ValueExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder.AddExpr(VarStmt, ValueExpr));
        EXPECT_TRUE(Builder.AddStmt(Body, VarStmt));

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
        ASTFunction *Test = Builder.CreateFunction(Module, SourceLoc, IntType, "test", DefaultScopes);
        Builder.CreateBody(Test);

        // func()
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, IntType, "func", DefaultScopes);
        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // call test()
        ASTExprStmt *ExprStmt = Builder.CreateExprStmt(SourceLoc);
        ASTCall *TestCall = Builder.CreateCall(Test);
        ASTCallExpr *Expr = Builder.CreateExpr(TestCall);
        EXPECT_TRUE(Builder.AddExpr(ExprStmt, Expr));
        EXPECT_TRUE(Builder.AddStmt(Body, ExprStmt));

        //return test()
        ASTReturnStmt *Return = Builder.CreateReturnStmt(SourceLoc);
        ASTCallExpr *ReturnExpr = Builder.CreateExpr(TestCall);
        EXPECT_TRUE(Builder.AddExpr(Return, ReturnExpr));
        EXPECT_TRUE(Builder.AddStmt(Body, Return));

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
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, IntType, "func", DefaultScopes);
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

        ASTBinaryGroupExpr *G2 = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_MUL), E2, E3);
        ASTBinaryGroupExpr *G3 = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_SUB), E4, E5);
        ASTBinaryGroupExpr *G1 = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_DIV), G2, G3);
        ASTBinaryGroupExpr *Group = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_ADD), E1, G1);

        Builder.AddExpr(Return, Group);
        EXPECT_TRUE(Builder.AddStmt(Body, Return));

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
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
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
        AssignExpr = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_ADD),
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cAddVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cAddVarStmt));

        // c = a - b
        ASTVarStmt * cSubVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_SUB),
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cSubVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cSubVarStmt));

        // c = a * b
        ASTVarStmt * cMulVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_MUL),
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cMulVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cMulVarStmt));

        // c = a / b
        ASTVarStmt * cDivVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_DIV),
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cDivVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cDivVarStmt));

        // c = a % b
        ASTVarStmt * cModVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_MOD),
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cModVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cModVarStmt));

        // c = a & b
        ASTVarStmt * cAndVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_AND),
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cAndVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cAndVarStmt));

        // c = a | b
        ASTVarStmt * cOrVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_OR),
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cOrVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cOrVarStmt));

        // c = a xor b
        ASTVarStmt * cXorVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_XOR),
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cXorVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cXorVarStmt));

        // c = a << b
        ASTVarStmt * cShlVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_SHIFT_L),
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cShlVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cShlVarStmt));

        // c = a >> b
        ASTVarStmt * cShrVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cParam));
        AssignExpr = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_ARITH_SHIFT_R),
                                               Builder.CreateExpr(Builder.CreateVarRef(aParam)),
                                               Builder.CreateExpr(Builder.CreateVarRef(bParam)));
        Builder.AddExpr(cShrVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cShrVarStmt));

        // ++c
        ASTExprStmt *cPreIncVarStmt = Builder.CreateExprStmt(SourceLoc);
        AssignExpr = Builder.CreateUnaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTUnaryOperatorKind::UNARY_ARITH_PRE_INCR),
                                              Builder.CreateExpr(Builder.CreateVarRef(cParam)));
        Builder.AddExpr(cPreIncVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cPreIncVarStmt));

        // c++
        ASTExprStmt *cPostIncVarStmt = Builder.CreateExprStmt(SourceLoc);
        AssignExpr = Builder.CreateUnaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTUnaryOperatorKind::UNARY_ARITH_POST_INCR),
                                              Builder.CreateExpr(Builder.CreateVarRef(cParam)));
        Builder.AddExpr(cPostIncVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cPostIncVarStmt));

        // ++c
        ASTExprStmt *cPreDecVarStmt = Builder.CreateExprStmt(SourceLoc);
        AssignExpr = Builder.CreateUnaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTUnaryOperatorKind::UNARY_ARITH_PRE_DECR),
                                              Builder.CreateExpr(Builder.CreateVarRef(cParam)));
        Builder.AddExpr(cPreDecVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cPreDecVarStmt));

        // c++
        ASTExprStmt *cPostDecVarStmt = Builder.CreateExprStmt(SourceLoc);
        AssignExpr = Builder.CreateUnaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTUnaryOperatorKind::UNARY_ARITH_PRE_DECR),
                                              Builder.CreateExpr(Builder.CreateVarRef(cParam)));
        Builder.AddExpr(cPostDecVarStmt, AssignExpr);
        EXPECT_TRUE(Builder.AddStmt(Body, cPostDecVarStmt));

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
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, IntType, "a");
        ASTLocalVar *bVar = Builder.CreateLocalVar(SourceLoc, IntType, "b");
        ASTLocalVar *cVar = Builder.CreateLocalVar(SourceLoc, BoolType, "c");
        EXPECT_TRUE(Builder.AddLocalVar(Body, aVar));
        EXPECT_TRUE(Builder.AddLocalVar(Body, bVar));
        EXPECT_TRUE(Builder.AddLocalVar(Body, cVar));

        // a = 0
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aVar));
        ASTExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 0));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt, Expr1));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt));

        // b = 0
        ASTVarStmt *bVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(bVar));
        ASTExpr *Expr2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 0));
        EXPECT_TRUE(Builder.AddExpr(bVarStmt, Expr2));
        EXPECT_TRUE(Builder.AddStmt(Body, bVarStmt));

        // c = a == b
        ASTVarStmt * cEqVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        ASTExpr *Expr3 = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddExpr(cEqVarStmt, Expr3));
        EXPECT_TRUE(Builder.AddStmt(Body, cEqVarStmt));

        // c = a != b
        ASTVarStmt * cNeqVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        ASTExpr *Expr4 = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_NE),
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddExpr(cNeqVarStmt, Expr4));
        EXPECT_TRUE(Builder.AddStmt(Body, cNeqVarStmt));

        // c = a > b
        ASTVarStmt * cGtVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        ASTExpr *Expr5 = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_GT),
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddExpr(cGtVarStmt, Expr5));
        EXPECT_TRUE(Builder.AddStmt(Body, cGtVarStmt));

        // c = a >= b
        ASTVarStmt * cGteVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        ASTExpr *Expr6 = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_GTE),
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddExpr(cGteVarStmt, Expr6));
        EXPECT_TRUE(Builder.AddStmt(Body, cGteVarStmt));

        // c = a < b
        ASTVarStmt * cLtVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        ASTExpr *Expr7 = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_LT),
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddExpr(cLtVarStmt, Expr7));
        EXPECT_TRUE(Builder.AddStmt(Body, cLtVarStmt));

        // c = a <= b
        ASTVarStmt * cLteVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        ASTExpr *Expr8 = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_LTE),
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddExpr(cLteVarStmt, Expr8));
        EXPECT_TRUE(Builder.AddStmt(Body, cLteVarStmt));

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
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, BoolType, "a");
        ASTLocalVar *bVar = Builder.CreateLocalVar(SourceLoc, BoolType, "b");
        ASTLocalVar *cVar = Builder.CreateLocalVar(SourceLoc, BoolType, "c");
        EXPECT_TRUE(Builder.AddLocalVar(Body, aVar));
        EXPECT_TRUE(Builder.AddLocalVar(Body, bVar));
        EXPECT_TRUE(Builder.AddLocalVar(Body, cVar));

        // a = false
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aVar));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt, Expr1));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt));

        // b = false
        ASTVarStmt *bVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(bVar));
        ASTValueExpr *Expr2 = Builder.CreateExpr(Builder.CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder.AddExpr(bVarStmt, Expr2));
        EXPECT_TRUE(Builder.AddStmt(Body, bVarStmt));

        // c = a and b
        ASTVarStmt * cAndVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        ASTExpr *Expr3 = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_LOGIC_AND),
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddExpr(cAndVarStmt, Expr3));
        EXPECT_TRUE(Builder.AddStmt(Body, cAndVarStmt));

        // c = a or b
        ASTVarStmt * cOrVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        ASTExpr *Expr4 = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_LOGIC_OR),
                                  Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                  Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddExpr(cOrVarStmt, Expr4));
        EXPECT_TRUE(Builder.AddStmt(Body, cOrVarStmt));

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
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
        ASTBlockStmt *Body = Builder.CreateBody(Func);
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, BoolType, "a");
        ASTLocalVar *bVar = Builder.CreateLocalVar(SourceLoc, BoolType, "b");
        ASTLocalVar *cVar = Builder.CreateLocalVar(SourceLoc, BoolType, "c");
        EXPECT_TRUE(Builder.AddLocalVar(Body, aVar));
        EXPECT_TRUE(Builder.AddLocalVar(Body, bVar));
        EXPECT_TRUE(Builder.AddLocalVar(Body, cVar));

        // a = false
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aVar));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt, Expr1));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt));

        // b = false
        ASTVarStmt *bVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(bVar));
        ASTValueExpr *Expr2 = Builder.CreateExpr(Builder.CreateBoolValue(SourceLoc, false));
        EXPECT_TRUE(Builder.AddExpr(bVarStmt, Expr2));
        EXPECT_TRUE(Builder.AddStmt(Body, bVarStmt));

        // c = a == b ? a : b
        ASTVarStmt * cVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(cVar));
        ASTBinaryGroupExpr *Cond = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                                                             Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                                             Builder.CreateExpr(Builder.CreateVarRef(bVar)));

        ASTTernaryGroupExpr *TernaryExpr = Builder.CreateTernaryExpr(Cond,
                                                                     Builder.CreateOperatorExpr(SourceLoc, ASTTernaryOperatorKind::TERNARY_IF),
                                                                     Builder.CreateExpr(Builder.CreateVarRef(aVar)),
                                                                     Builder.CreateOperatorExpr(SourceLoc, ASTTernaryOperatorKind::TERNARY_ELSE),
                                                                     Builder.CreateExpr(Builder.CreateVarRef(bVar)));
        EXPECT_TRUE(Builder.AddExpr(cVarStmt, TernaryExpr));
        EXPECT_TRUE(Builder.AddStmt(Body, cVarStmt));

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
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // int a = 0
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddLocalVar(Body, aVar));
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aVar));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 0));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt, Expr1));
        EXPECT_TRUE(Builder.AddStmt(Body, aVarStmt));

        // if (a == 1)
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(Builder.CreateVarRef(aVar));
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTBinaryGroupExpr *IfCond = Builder.CreateBinaryExpr(
                Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ), aVarRef, Value1);

        // Create/Add if block
        ASTIfStmt *IfStmt = Builder.CreateIfStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddExpr(IfStmt, IfCond));
        EXPECT_TRUE(Builder.AddStmt(Body, IfStmt));

        // { a = 2 }
        ASTBlockStmt *IfBlock = Builder.CreateBlockStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(IfStmt, IfBlock));
        ASTVarStmt *a2VarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aVar));
        EXPECT_TRUE(Builder.AddStmt(IfBlock, a2VarStmt));
        ASTValueExpr *Expr2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        EXPECT_TRUE(Builder.AddExpr(a2VarStmt, Expr2));

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
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(Func, aParam));

        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // if (a == 1)
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(Builder.CreateVarRef(aParam));
        ASTBinaryGroupExpr *IfCond = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                                                              aVarRef, Value1);

        // Create/Add if block
        ASTIfStmt *IfStmt = Builder.CreateIfStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddExpr(IfStmt, IfCond));
        EXPECT_TRUE(Builder.AddStmt(Body, IfStmt));

        // { a = 1 }
        ASTBlockStmt *IfBlock = Builder.CreateBlockStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(IfStmt, IfBlock));
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        EXPECT_TRUE(Builder.AddStmt(IfBlock, aVarStmt));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt, Expr1));

        // else {a = 2}
        ASTBlockStmt *ElseBlock = Builder.CreateBlockStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddElse(IfStmt, ElseBlock));
        ASTVarStmt *aVarStmt2 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        EXPECT_TRUE(Builder.AddStmt(ElseBlock, aVarStmt2));
        ASTValueExpr *Expr2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt2, Expr2));

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
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(Func, aParam));

        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // if (a == 1)
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(Builder.CreateVarRef(aParam));
        ASTBinaryGroupExpr *IfCond = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                                                              aVarRef, Value1);

        // Create/Add if block
        ASTIfStmt *IfStmt = Builder.CreateIfStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddExpr(IfStmt, IfCond));
        EXPECT_TRUE(Builder.AddStmt(Body, IfStmt));
        
        // { a = 11 }
        ASTBlockStmt *IfBlock = Builder.CreateBlockStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(IfStmt, IfBlock));
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        EXPECT_TRUE(Builder.AddStmt(IfBlock, aVarStmt));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 11));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt, Expr1));

        // elsif (a == 2)
        ASTBlockStmt *ElsifBlock = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        ASTBinaryGroupExpr *ElsifCond = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                                                                 aVarRef, Value2);
        EXPECT_TRUE(Builder.AddElsif(IfStmt, ElsifCond, ElsifBlock));
        // { a = 22 }
        ASTVarStmt *aVarStmt2 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        EXPECT_TRUE(Builder.AddStmt(ElsifBlock, aVarStmt2));
        ASTValueExpr *Expr2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 22));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt2, Expr2));

        // elsif (a == 3) { a = 33 }
        ASTBlockStmt *ElsifBlock2 = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value3 = Builder.CreateExpr( Builder.CreateIntegerValue(SourceLoc, 3));
        ASTBinaryGroupExpr *ElsifCond2 = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                                                                  aVarRef, Value3);
        EXPECT_TRUE(Builder.AddElsif(IfStmt, ElsifCond2, ElsifBlock2));
        ASTVarStmt *aVarStmt3 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        EXPECT_TRUE(Builder.AddStmt(ElsifBlock2, aVarStmt3));
        ASTValueExpr *Expr3 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 33));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt3, Expr3));

        // else {a == 44}
        ASTBlockStmt *ElseBlock = Builder.CreateBlockStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddElse(IfStmt, ElseBlock));
        ASTVarStmt *aVarStmt4 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        EXPECT_TRUE(Builder.AddStmt(ElseBlock, aVarStmt4));
        ASTValueExpr *Expr4 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 44));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt4, Expr4));

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
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(Func, aParam));

        ASTBlockStmt *Body = Builder.CreateBody(Func);
        
        // if a == 1
        ASTIfStmt *IfStmt = Builder.CreateIfStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(Body, IfStmt));
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(Builder.CreateVarRef(aParam));
        ASTBinaryGroupExpr *IfCond = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ), aVarRef, Value1);
        EXPECT_TRUE(Builder.AddExpr(IfStmt, IfCond));

        // { a = 11 }
        ASTBlockStmt *IfBlock = Builder.CreateBlockStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(IfStmt, IfBlock));
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        EXPECT_TRUE(Builder.AddStmt(IfBlock, aVarStmt));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 11));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt, Expr1));

        // elsif (a == 2)
        ASTBlockStmt *ElsifBlock = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        ASTBinaryGroupExpr *ElsifCond = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                                                                 aVarRef, Value2);
        EXPECT_TRUE(Builder.AddElsif(IfStmt, ElsifCond, ElsifBlock));
        // { a = 22 }
        ASTVarStmt *aVarStmt2 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        EXPECT_TRUE(Builder.AddStmt(ElsifBlock, aVarStmt2));
        ASTValueExpr *Expr2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 22));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt2, Expr2));

        // elsif (a == 3) { a = 33 }
        ASTBlockStmt *ElsifBlock2 = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Value3 = Builder.CreateExpr( Builder.CreateIntegerValue(SourceLoc, 3));
        ASTBinaryGroupExpr *ElsifCond2 = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ),
                                                                  aVarRef, Value3);
        EXPECT_TRUE(Builder.AddElsif(IfStmt, ElsifCond2, ElsifBlock2));
        ASTVarStmt *aVarStmt3 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        EXPECT_TRUE(Builder.AddStmt(ElsifBlock2, aVarStmt3));
        ASTValueExpr *Expr3 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 33));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt3, Expr3));

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
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(Func, aParam));

        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // switch a
        ASTSwitchStmt *SwitchStmt = Builder.CreateSwitchStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(Body, SwitchStmt));
        ASTVarRefExpr *aVarRefExpr = Builder.CreateExpr(Builder.CreateVarRef(aParam));
        EXPECT_TRUE(Builder.AddExpr(SwitchStmt, aVarRefExpr));

        // case 1: a = 1 break
        ASTBlockStmt *Case1Block = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Case1Value = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder.AddSwitchCase(SwitchStmt, Case1Value, Case1Block));
        ASTVarStmt *aVarStmt1 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        EXPECT_TRUE(Builder.AddStmt(Case1Block, aVarStmt1));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt1, Expr1));

        // case 2: a = 2 break
        ASTBlockStmt *Case2Block = Builder.CreateBlockStmt(SourceLoc);
        ASTValueExpr *Case2Value = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        EXPECT_TRUE(Builder.AddSwitchCase(SwitchStmt, Case2Value, Case2Block));
        ASTVarStmt *aVarStmt2 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        EXPECT_TRUE(Builder.AddStmt(Case2Block, aVarStmt2));
        ASTValueExpr *Expr2 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt2, Expr2));

        // default: a = 3
        ASTBlockStmt *DefaultBlock = Builder.CreateBlockStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddSwitchDefault(SwitchStmt, DefaultBlock));
        ASTVarStmt *aVarStmt3 = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        EXPECT_TRUE(Builder.AddStmt(DefaultBlock, aVarStmt3));
        ASTValueExpr *Expr3 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 3));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt3, Expr3));
        
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
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(Func, aParam));

        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // while a == 1
        ASTLoopStmt *LoopStmt = Builder.CreateLoopStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(Body, LoopStmt));
        ASTValueExpr *Value1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        ASTVarRefExpr *aVarRef = Builder.CreateExpr(Builder.CreateVarRef(aParam));
        ASTBinaryGroupExpr *Cond = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_EQ), aVarRef, Value1);
        EXPECT_TRUE(Builder.AddExpr(LoopStmt, Cond));
        
        // { a = 1 }
        ASTBlockStmt *BlockStmt = Builder.CreateBlockStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(LoopStmt, BlockStmt));
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        EXPECT_TRUE(Builder.AddStmt(BlockStmt, aVarStmt));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt, Expr1));

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
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
        ASTParam *aParam = Builder.CreateParam(SourceLoc, IntType, "a");
        EXPECT_TRUE(Builder.AddParam(Func, aParam));

        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // for int i = 1; i < 1; ++i
        ASTLoopStmt *ForStmt = Builder.CreateLoopStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(Body, ForStmt));

        // int i = 1
        ASTBlockStmt *InitBlock = Builder.CreateBlockStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddLoopInit(ForStmt, InitBlock));
        ASTLocalVar *iVar = Builder.CreateLocalVar(SourceLoc, IntType, "i");
        EXPECT_TRUE(Builder.AddLocalVar(InitBlock, iVar));
        ASTVarRef *iVarRef = Builder.CreateVarRef(iVar);
        ASTVarStmt *iVarStmt = Builder.CreateVarStmt(iVarRef);
        ASTValueExpr *Value1Expr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder.AddExpr(iVarStmt, Value1Expr));
        EXPECT_TRUE(Builder.AddStmt(InitBlock, iVarStmt));

        // i < 1
        ASTBinaryGroupExpr *Cond = Builder.CreateBinaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTBinaryOperatorKind::BINARY_COMP_LTE),
                                                            Builder.CreateExpr(iVarRef), Value1Expr);
        EXPECT_TRUE(Builder.AddExpr(ForStmt, Cond));
        
        // ++i
        ASTBlockStmt *PostBlock = Builder.CreateBlockStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddLoopPost(ForStmt, PostBlock));
        ASTUnaryGroupExpr *IncExpr = Builder.CreateUnaryExpr(Builder.CreateOperatorExpr(SourceLoc, ASTUnaryOperatorKind::UNARY_ARITH_PRE_INCR),
                                                             Builder.CreateExpr(Builder.CreateVarRef(iVar)));
        ASTExprStmt *iVarIncStmt = Builder.CreateExprStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddExpr(iVarIncStmt, IncExpr));
        EXPECT_TRUE(Builder.AddStmt(PostBlock, iVarIncStmt));

        // { a = 1}
        ASTBlockStmt *LoopBlock = Builder.CreateBlockStmt(SourceLoc);
        EXPECT_TRUE(Builder.AddStmt(ForStmt, LoopBlock));
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(aParam));
        ASTValueExpr *Expr1 = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 1));
        EXPECT_TRUE(Builder.AddExpr(aVarStmt, Expr1));
        EXPECT_TRUE(Builder.AddStmt(LoopBlock, aVarStmt));

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
                                                  DefaultScopes, SuperClasses);

        // int a() { return 1 }
        ASTClassMethod *aFunc = Builder.CreateClassMethod(SourceLoc, *TestClass, IntType,
                                                          "a",
                                                          DefaultScopes);
        ASTBlockStmt *aFuncBody = Builder.CreateBody(aFunc);
        ASTReturnStmt *aFuncReturn = Builder.CreateReturnStmt(SourceLoc);
        ASTValueExpr *aFuncExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLocation(), 1));
        Builder.AddExpr(aFuncReturn, aFuncExpr);
        Builder.AddStmt(aFuncBody, aFuncReturn);


        // public int b() { return 1 }
        ASTClassMethod *bFunc = Builder.CreateClassMethod(SourceLoc, *TestClass, IntType,
                                                          "b", DefaultScopes);
        ASTBlockStmt *bFuncBody = Builder.CreateBody(bFunc);
        ASTReturnStmt *bFuncReturn = Builder.CreateReturnStmt(SourceLoc);
        ASTValueExpr *bFuncExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLocation(), 1));
        Builder.AddExpr(bFuncReturn, bFuncExpr);
        Builder.AddStmt(bFuncBody, bFuncReturn);

        // private const int c { return 1 }
        ASTClassMethod *cFunc = Builder.CreateClassMethod(SourceLoc, *TestClass, IntType,
                                                          "c", DefaultScopes);
        ASTBlockStmt *cFuncBody = Builder.CreateBody(cFunc);
        ASTReturnStmt *cFuncReturn = Builder.CreateReturnStmt(SourceLoc);
        ASTValueExpr *cFuncExpr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLocation(), 1));
        Builder.AddExpr(cFuncReturn, cFuncExpr);
        Builder.AddStmt(cFuncBody, cFuncReturn);

        // int main() {
        //  TestClass test = new TestClass()
        //  int a = test.a()
        //  int b = test.b()
        //  int c = test.c()
        //  delete test
        // }
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // TestClass test = new TestClass()
        ASTType *TestClassType = Builder.CreateClassType(TestClass);
        ASTLocalVar *TestVar = Builder.CreateLocalVar(SourceLoc, TestClassType, "test");
        Builder.AddLocalVar(Body, TestVar);
        ASTClassMethod *DefaultConstructor = TestClass->getDefaultConstructor();
        ASTVarRef *Instance = Builder.CreateVarRef(TestVar);
        ASTCall *ConstructorCall = Builder.CreateCall(DefaultConstructor);
        ASTCallExpr *NewExpr = Builder.CreateNewExpr(ConstructorCall);
        ASTVarStmt *testNewStmt = Builder.CreateVarStmt(TestVar);
        Builder.AddExpr(testNewStmt, NewExpr);
        Builder.AddStmt(Body, testNewStmt);

        // int a = test.a()
        ASTType *aType = aFunc->getReturnType();
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, aType, "a");
        Builder.AddLocalVar(Body, aVar);
        ASTCallExpr *aCallExpr = Builder.CreateExpr(Builder.CreateCall(Instance, aFunc));
        ASTVarStmt *aStmt = Builder.CreateVarStmt(aVar);
        Builder.AddExpr(aStmt, aCallExpr);
        Builder.AddStmt(Body, aStmt);

        // int b = test.b()
        ASTType *bType = bFunc->getReturnType();
        ASTLocalVar *bVar = Builder.CreateLocalVar(SourceLoc, bType, "b");
        Builder.AddLocalVar(Body, bVar);
        ASTCallExpr *bCallExpr = Builder.CreateExpr(Builder.CreateCall(Instance, bFunc));
        ASTVarStmt *bStmt = Builder.CreateVarStmt(bVar);
        Builder.AddExpr(bStmt, bCallExpr);
        Builder.AddStmt(Body, bStmt);

        // int c = test.c()
        ASTType *cType = cFunc->getReturnType();
        ASTLocalVar *cVar = Builder.CreateLocalVar(SourceLoc, cType, "c");
        Builder.AddLocalVar(Body, cVar);
        ASTCallExpr *cCallExpr = Builder.CreateExpr(Builder.CreateCall(Instance, cFunc));
        ASTVarStmt *cStmt = Builder.CreateVarStmt(cVar);
        Builder.AddExpr(cStmt, cCallExpr);
        Builder.AddStmt(Body, cStmt);

        // delete test
        ASTDeleteStmt *DeleteStmt = Builder.CreateDeleteStmt(SourceLoc, (ASTVarRef *) Instance);
        Builder.AddStmt(Body, DeleteStmt);

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
        //   int a() { return a }
        // }
        llvm::SmallVector<ASTClassType *, 4> SuperClasses;
        ASTClass *TestClass = Builder.CreateClass(Module, SourceLoc, ASTClassKind::CLASS, "TestClass", DefaultScopes,
                                                  SuperClasses);

        // int a
        ASTClassAttribute *aAttribute = Builder.CreateClassAttribute(SourceLoc, *TestClass,
                                                                     Builder.CreateIntType(SourceLoc),
                                                                     "a", DefaultScopes);

        // int a() { return a }
        ASTClassMethod *aFunc = Builder.CreateClassMethod(SourceLoc, *TestClass, IntType,
                                                          "a", DefaultScopes);
        ASTBlockStmt *aFuncBody = Builder.CreateBody(aFunc);
        ASTReturnStmt *aFuncReturn = Builder.CreateReturnStmt(SourceLoc);
        Builder.AddExpr(aFuncReturn, Builder.CreateExpr(Builder.CreateVarRef(aAttribute)));
        Builder.AddStmt(aFuncBody, aFuncReturn);

        // int main() {
        //  TestClass test = new TestClass()
        //  int a = test.a()
        //  test.a = 2
        //  delete test
        // }
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // TestClass test = new TestClass()
        ASTType *TestClassType = Builder.CreateClassType(TestClass);
        ASTLocalVar *TestVar = Builder.CreateLocalVar(SourceLoc, TestClassType, "test");
        Builder.AddLocalVar(Body, TestVar);
        ASTClassMethod *DefaultConstructor = TestClass->getDefaultConstructor();
        ASTCall *ConstructorCall = Builder.CreateCall(DefaultConstructor);
        ASTCallExpr *NewExpr = Builder.CreateNewExpr(ConstructorCall);
        ASTVarStmt *testNewStmt = Builder.CreateVarStmt(TestVar);
        Builder.AddExpr(testNewStmt, NewExpr);
        Builder.AddStmt(Body, testNewStmt);

        // int a = test.a()
        ASTVarRef *Instance = Builder.CreateVarRef(TestVar);
        ASTType *aType = aFunc->getReturnType();
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, aType, "a");
        Builder.AddLocalVar(Body, aVar);
        ASTCallExpr *aCallExpr = Builder.CreateExpr(Builder.CreateCall(Instance, aFunc));
        ASTVarStmt *aStmt = Builder.CreateVarStmt(aVar);
        Builder.AddExpr(aStmt, aCallExpr);
        Builder.AddStmt(Body, aStmt);

        //  test.a = 2
        ASTVarStmt *attrStmt = Builder.CreateVarStmt(Builder.CreateVarRef(Instance, aAttribute));
        ASTValueExpr *value2Expr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLocation(), 2));
        Builder.AddExpr(attrStmt, value2Expr);
        Builder.AddStmt(Body, attrStmt);

        // delete test
        ASTDeleteStmt *DeleteStmt = Builder.CreateDeleteStmt(SourceLoc, (ASTVarRef *) Instance);
        Builder.AddStmt(Body, DeleteStmt);

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
                              "define i32 @TestClass_a(%error* %0, %TestClass* %1) {\n"
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
                              "  %7 = call i32 @TestClass_a(%error* %4, %TestClass* %6)\n"
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
                                                   DefaultScopes, SuperClasses);
        ASTClassAttribute *aField = Builder.CreateClassAttribute(SourceLoc, *TestStruct, Builder.CreateIntType(SourceLoc),
                                                                 "a", DefaultScopes);
        ASTClassAttribute *bField = Builder.CreateClassAttribute(SourceLoc, *TestStruct, Builder.CreateIntType(SourceLoc),
                                                                 "b", DefaultScopes);
        ASTClassAttribute *cField = Builder.CreateClassAttribute(SourceLoc, *TestStruct, Builder.CreateIntType(SourceLoc),
                                                                 "c", DefaultScopes);

        // int func() {
        //  TestStruct test = new TestStruct();
        //  int a = test.a
        //  test.b = 2
        //  return 1
        // }
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
        ASTBlockStmt *Body = Builder.CreateBody(Func);

        // TestStruct test = new TestStruct()
        ASTType *TestClassType = Builder.CreateClassType(TestStruct);
        ASTLocalVar *TestVar = Builder.CreateLocalVar(SourceLoc, TestClassType, "test");
        Builder.AddLocalVar(Body, TestVar);
        ASTClassMethod *DefaultConstructor = TestStruct->getDefaultConstructor();
        ASTCall *ConstructorCall = Builder.CreateCall(DefaultConstructor);
        ASTCallExpr *NewExpr = Builder.CreateNewExpr(ConstructorCall);
        ASTVarStmt *testNewStmt = Builder.CreateVarStmt(TestVar);
        Builder.AddExpr(testNewStmt, NewExpr);
        Builder.AddStmt(Body, testNewStmt);

        // int a = test.a
        ASTVarRef *Instance = Builder.CreateVarRef(TestVar);
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, IntType, "a");
        Builder.AddLocalVar(Body, aVar);
        ASTVarRefExpr *test_aRefExpr = Builder.CreateExpr(Builder.CreateVarRef(Instance, aField));
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(aVar);
        Builder.AddExpr(aVarStmt, test_aRefExpr);
        Builder.AddStmt(Body, aVarStmt);

        // test.b = 2
        ASTVarStmt *test_bVarStmt = Builder.CreateVarStmt(Builder.CreateVarRef(Instance, bField));
        ASTExpr *Expr = Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, 2));
        Builder.AddExpr(test_bVarStmt, Expr);
        Builder.AddStmt(Body, test_bVarStmt);

        // delete test
        ASTDeleteStmt *Delete = Builder.CreateDeleteStmt(SourceLoc, (ASTVarRef *) Instance);
        Builder.AddStmt(Body, Delete);

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
        ASTEnum *TestEnum = Builder.CreateEnum(Module, SourceLoc, "TestEnum", DefaultScopes, SuperEnums);
        ASTEnumEntry *A = Builder.CreateEnumEntry(SourceLoc, TestEnum, "A");
        ASTEnumEntry *B = Builder.CreateEnumEntry(SourceLoc, TestEnum, "B");
        ASTEnumEntry *C = Builder.CreateEnumEntry(SourceLoc, TestEnum, "C");

        // int main() {
        //  TestEnum a = TestEnum.A;
        //  TestEnum b = a
        //  return 1
        // }
        ASTFunction *Func = Builder.CreateFunction(Module, SourceLoc, VoidType, "func", DefaultScopes);
        ASTBlockStmt *Body = Builder.CreateBody(Func);

        ASTType *TestEnumType = Builder.CreateEnumType(TestEnum);

        //  TestEnum a = TestEnum.A;
        ASTLocalVar *aVar = Builder.CreateLocalVar(SourceLoc, TestEnumType, "a");
        Builder.AddLocalVar(Body, aVar);
        ASTVarRefExpr *Enum_ARefExpr = Builder.CreateExpr(Builder.CreateVarRef(A));
        ASTVarStmt *aVarStmt = Builder.CreateVarStmt(aVar);
        Builder.AddExpr(aVarStmt, Enum_ARefExpr);
        Builder.AddStmt(Body, aVarStmt);

        //  TestEnum b = a
        ASTLocalVar *bVar = Builder.CreateLocalVar(SourceLoc, TestEnumType, "b");
        Builder.AddLocalVar(Body, bVar);
        ASTVarStmt *bVarStmt = Builder.CreateVarStmt(bVar);
        ASTVarRefExpr *aRefExpr = Builder.CreateExpr(Builder.CreateVarRef(aVar));
        Builder.AddExpr(bVarStmt, aRefExpr);
        Builder.AddStmt(Body, bVarStmt);

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

    TEST_F(CodeGenTest, CGError) {
        ASTModule *Module = CreateModule();

        // int testFail0() {
        //   fail()
        // }
        ASTFunction *TestFail0 = Builder.CreateFunction(Module, SourceLoc, IntType, "testFail0", DefaultScopes);
        ASTBlockStmt *Body0 = Builder.CreateBody(TestFail0);
        ASTFailStmt *Fail0Stmt = Builder.CreateFailStmt(SourceLoc, TestFail0->getErrorHandler());
        Builder.AddExpr(Fail0Stmt, Builder.CreateExpr());
        EXPECT_TRUE(Builder.AddStmt(Body0, Fail0Stmt));

        // int testFail1() {
        //   fail(true)
        // }
        ASTFunction *TestFail1 = Builder.CreateFunction(Module, SourceLoc, IntType, "testFail1", DefaultScopes);
        ASTBlockStmt *Body1 = Builder.CreateBody(TestFail1);
        ASTBoolValue *BoolVal = Builder.CreateBoolValue(SourceLoc, true);
        ASTFailStmt *Fail1Stmt = Builder.CreateFailStmt(SourceLoc, TestFail1->getErrorHandler());
        Builder.AddExpr(Fail1Stmt, Builder.CreateExpr(BoolVal));
        EXPECT_TRUE(Builder.AddStmt(Body1, Fail1Stmt));

        // int testFail2() {
        //   fail(10)
        // }
        ASTFunction *TestFail2 = Builder.CreateFunction(Module, SourceLoc, IntType, "testFail2", DefaultScopes);
        ASTBlockStmt *Body2 = Builder.CreateBody(TestFail2);
        ASTIntegerValue *IntVal = Builder.CreateIntegerValue(SourceLoc, 10);
        ASTFailStmt *Fail2Stmt = Builder.CreateFailStmt(SourceLoc, TestFail2->getErrorHandler());
        Builder.AddExpr(Fail2Stmt, Builder.CreateExpr(IntVal));
        EXPECT_TRUE(Builder.AddStmt(Body2, Fail2Stmt));

        // int testFail3() {
        //  fail("Error")
        // }
        ASTFunction *TestFail3 = Builder.CreateFunction(Module, SourceLoc, IntType, "testFail3", DefaultScopes);
        ASTBlockStmt *Body3 = Builder.CreateBody(TestFail3);
        ASTStringValue *StrVal = Builder.CreateStringValue(SourceLoc, "Error");
        ASTFailStmt *Fail3Stmt = Builder.CreateFailStmt(SourceLoc, TestFail3->getErrorHandler());
        Builder.AddExpr(Fail3Stmt, Builder.CreateExpr(StrVal));
        EXPECT_TRUE(Builder.AddStmt(Body3, Fail3Stmt));

        // int testFail4() {
        //  fail(new TestStruct())
        // }
        llvm::SmallVector<ASTClassType *, 4> SuperClasses;
        ASTClass *TestStruct = Builder.CreateClass(Module, SourceLoc, ASTClassKind::STRUCT, "TestStruct", DefaultScopes, SuperClasses);
        ASTClassAttribute *aField = Builder.CreateClassAttribute(SourceLoc, *TestStruct, Builder.CreateIntType(SourceLoc), "a", DefaultScopes);

        ASTFunction *TestFail4 = Builder.CreateFunction(Module, SourceLoc, IntType, "testFail4", DefaultScopes);
        ASTBlockStmt *Body4 = Builder.CreateBody(TestFail4);
        // TestStruct test = new TestStruct()
        ASTType *TestClassType = Builder.CreateClassType(TestStruct);
        ASTClassMethod *DefaultConstructor = TestStruct->getDefaultConstructor();
        ASTCall *ConstructorCall = Builder.CreateCall(DefaultConstructor);
        // fail new TestStruct()
        ASTFailStmt *Fail4Stmt = Builder.CreateFailStmt(SourceLoc, TestFail4->getErrorHandler());
        Builder.AddExpr(Fail4Stmt, Builder.CreateNewExpr(ConstructorCall));
        EXPECT_TRUE(Builder.AddStmt(Body4, Fail4Stmt));

        // main() {
        //   testFail0()
        //   testFail1()
        //   testFail2()
        //   testFail3()
        //   testFail4()
        // }
        ASTFunction *Main = Builder.CreateFunction(Module, SourceLoc, VoidType, "main", DefaultScopes);
        ASTBlockStmt *MainBody = Builder.CreateBody(Main);

        // call testFail0()
        ASTExprStmt *CallTestFail0 = Builder.CreateExprStmt(SourceLoc);
        ASTCallExpr *CallExpr0 = Builder.CreateExpr(Builder.CreateCall(TestFail0));
        Builder.AddExpr(CallTestFail0, CallExpr0);
        EXPECT_TRUE(Builder.AddStmt(MainBody, CallTestFail0));

        // call testFail1()
        ASTExprStmt *CallTestFail1 = Builder.CreateExprStmt(SourceLoc);
        ASTCallExpr *CallExpr1 = Builder.CreateExpr(Builder.CreateCall(TestFail1));
        Builder.AddExpr(CallTestFail1, CallExpr1);
        EXPECT_TRUE(Builder.AddStmt(MainBody, CallTestFail1));

        // call testFail2()
        ASTExprStmt *CallTestFail2 = Builder.CreateExprStmt(SourceLoc);
        ASTCallExpr *CallExpr2 = Builder.CreateExpr(Builder.CreateCall(TestFail2));
        Builder.AddExpr(CallTestFail2, CallExpr2);
        EXPECT_TRUE(Builder.AddStmt(MainBody, CallTestFail2));

        // call testFail3()
        ASTExprStmt *CallTestFail3 = Builder.CreateExprStmt(SourceLoc);
        ASTCallExpr *CallExpr3 = Builder.CreateExpr(Builder.CreateCall(TestFail3));
        Builder.AddExpr(CallTestFail3, CallExpr3);
        EXPECT_TRUE(Builder.AddStmt(MainBody, CallTestFail3));

        // call testFail4()
        ASTExprStmt *CallTestFail4 = Builder.CreateExprStmt(SourceLoc);
        ASTCallExpr *CallExpr4 = Builder.CreateExpr(Builder.CreateCall(TestFail4));
        Builder.AddExpr(CallTestFail4, CallExpr4);
        EXPECT_TRUE(Builder.AddStmt(MainBody, CallTestFail4));

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
} // anonymous namespace
