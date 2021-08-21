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
#include "AST/ASTNode.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunc.h"
#include "AST/ASTVar.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTForBlock.h"
#include "AST/ASTOperatorExpr.h"
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

    const char* testFile = "test.fly";

    // The test fixture.
    class CodeGenTest : public ::testing::Test {

    public:
        const CompilerInstance CI;
        ASTContext *Context;
        CodeGen *CG;
        DiagnosticsEngine &Diags;
        SourceLocation SourceLoc;

        CodeGenTest() : CI(*TestUtils::CreateCompilerInstance()),
                   Context(new ASTContext(CI.getDiagnostics())),
                   CG(TestUtils::CreateCodeGen(CI)),
                   Diags(CI.getDiagnostics()) {

        }

        ASTNode *CreateAST(const llvm::StringRef Name, const StringRef NameSpace = "default") {
            auto *Node = new ASTNode(Name, Context, CG->CreateModule(Name));
            Node->setNameSpace(NameSpace);
            return Node;
        }
    };

    bool createTestFile(const char* testFile) {
        std::fstream my_file;
        my_file.open(testFile, std::ios::out);
        if (my_file) {
            std::cout << "File " << testFile << " created successfully!\n";
            my_file.close();
        } else {
            std::cout << "Error File " << testFile << " not created!\n";
        }
        return (bool) my_file;
    }

    void read() {
        std::ifstream reader(testFile) ;

        ASSERT_TRUE(reader && "Error opening input file");

        std::cout << testFile << " contains: \n";
        char letter ;
        for(int i = 0; ! reader.eof() ; i++ ) {
            reader.get( letter ) ;
            std::cout << letter ;
        }

        reader.close() ;
    }

    void deleteTestFile(const char* testFile) {
        remove(testFile);
        std::cout << "File " << testFile << " deleted successfully!\n";
    }

    TEST_F(CodeGenTest, EmitNothing) {

        EXPECT_TRUE(createTestFile(testFile));

        ASTNode *Node = CreateAST(testFile);

        CodeGenOptions codeGenOpts;
        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        CodeGen CG(Diags, codeGenOpts, TargetOpts, Backend_EmitNothing);
        CI.getTargetOptions()->CodeModel = "default";
        bool Success = CG.Emit(nullptr);

        ASSERT_TRUE(Success);
        delete Node;
        deleteTestFile(testFile);
    }
//
//    TEST_F(CodeGenTest, EmitLL) {
//
//        EXPECT_TRUE(createTestFile(testFile));
//
//        ASTContext *Ctx = new ASTContext(Diags);
//        ASTNode *Node = createAST(testFile);
//
//        CodeGenOptions CodeGenOpts;
//        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
//        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
//        TargetOpts->CodeModel = "default";
//        LLVMContext LLVMCtx;
//        CodeGen CG(Diags, CodeGenOpts, TargetOpts, *Ctx, Backend_EmitLL);
//        ASSERT_TRUE(CG.GenerateModule(Node));
//
//        CG.getModule()->print(llvm::outs(), nullptr);
//        read();
//        deleteTestFile(testFile);
//        delete Node;
//    }

//    TEST_F(CodeGenTest, EmitBC) {
//
//        EXPECT_TRUE(createTestFile(testFile));
//
//        ASTContext *Ctx = new ASTContext(Diags);
//        ASTNode *Node = createAST(testFile);
//
//        CodeGenOptions CodeGenOpts;
//        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
//        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
//        TargetOpts->CodeModel = "default";
//        LLVMContext LLVMCtx;
//        CodeGen CG(Diags, CodeGenOpts, TargetOpts, *Ctx, Backend_EmitLL);
//        ASSERT_TRUE(CG.GenerateModule(Node));
//
//        CG.getModule()->print(llvm::outs(), nullptr);
//
//        read();
//        deleteTestFile(testFile);
//        delete Node;
//    }
//
//    TEST_F(CodeGenTest, EmitAS) {
//
//        EXPECT_TRUE(createTestFile(testFile));
//
//        ASTContext *Ctx = new ASTContext(Diags);
//        ASTNode *Node = createAST(testFile);
//
//        CodeGenOptions CodeGenOpts;
//        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
//        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
//        TargetOpts->CodeModel = "default";
//        LLVMContext LLVMCtx;
//        CodeGen CG(Diags, CodeGenOpts, TargetOpts, *Ctx, Backend_EmitAssembly);
//        ASSERT_TRUE(CG.GenerateModule(Node));
//
//        CG.getModule()->print(llvm::outs(), nullptr);
//
//        deleteTestFile(testFile);
//        delete Node;
//    }
//
//    TEST_F(CodeGenTest, EmitOBJ) {
//
//        EXPECT_TRUE(createTestFile(testFile));
//
//        ASTContext *Ctx = new ASTContext(Diags);
//        ASTNode *Node = createAST(testFile);
//
//        CodeGenOptions CodeGenOpts;
//        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
//        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
//        TargetOpts->CodeModel = "default";
//        LLVMContext LLVMCtx;
//        CodeGen CG(Diags, CodeGenOpts, TargetOpts, *Ctx, Backend_EmitObj);
//        ASSERT_TRUE(CG.GenerateModule(Node));
//
//        CG.getModule()->print(llvm::outs(), nullptr);
//
//        deleteTestFile(testFile);
//        delete Node;
//    }

    TEST_F(CodeGenTest, CGGlobalVar) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTNode *Node = CreateAST(testFile);
        ASTGlobalVar *Var = new ASTGlobalVar(Node, SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->addGlobalVar(Var);

        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        GlobalVariable *GVar = (GlobalVariable *)CGM->GenGlobalVar(Var)->getPointer();
        testing::internal::CaptureStdout();
        GVar->print(llvm::outs(), true);
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "@a = external global i32");
        delete Node;
        deleteTestFile(testFile);
    }

    TEST_F(CodeGenTest, CGFunc) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTNode *Node = CreateAST(testFile);
        
        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc),
                                      "main");
        MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "P1");
        MainFn->addParam(SourceLoc, new ASTFloatType(SourceLoc), "P2");
        MainFn->addParam(SourceLoc, new ASTBoolType(SourceLoc), "P3");
        Node->addFunction(MainFn);

        ASTValueExpr *Expr = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        MainFn->getBody()->addReturn(SourceLoc, Expr);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0, float %1, i1 %2) {\n"
                          "entry:\n"
                          "  %3 = alloca i32, align 4\n"
                          "  %4 = alloca float, align 4\n"
                          "  %5 = alloca i1, align 1\n"
                          "  store i32 %0, i32* %3, align 4\n"
                          "  store float %1, float* %4, align 4\n"
                          "  store i1 %2, i1* %5, align 1\n"
                          "  ret i32 1\n"
                          "}\n");
        delete Node;
        deleteTestFile(testFile);
    }

    TEST_F(CodeGenTest, CGFuncRetVar) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTNode *Node = CreateAST(testFile);

        ASTGlobalVar *GVar = new ASTGlobalVar(Node, SourceLoc, new ASTFloatType(SourceLoc), "G");
        Node->addGlobalVar(GVar);

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        Node->addFunction(MainFn);

        // int A
        ASTLocalVar *VarA = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTIntType(SourceLoc), "A");
        MainFn->getBody()->addVar(VarA);

        // A = 1
        ASTLocalVarRef * VStmt = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), VarA->getName());
        ASTExpr *Expr = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        VStmt->setExpr(Expr);
        MainFn->getBody()->addVar(VStmt);

        // GlobalVar
        // G = 1
        ASTLocalVarRef * GStmt = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), GVar->getName(), "default");
        GStmt->setExpr(Expr);
        MainFn->getBody()->addVar(GStmt);

        // return A
        MainFn->getBody()->addReturn(SourceLoc, new ASTVarRefExpr(SourceLoc,
                                                                  new ASTVarRef(SourceLoc, VarA->getName())));

        Node->Resolve();

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        CGM->GenGlobalVar(GVar);
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca i32, align 4\n"
                          "  store i32 0, i32* %0, align 4\n"
                          "  store i32 1, i32* %0, align 4\n"
                          "  store float 1.000000e+00, float* @G, align 4\n"
                          "  %1 = load i32, i32* %0, align 4\n"
                          "  ret i32 %1\n"
                          "}\n");
        delete Node;
        deleteTestFile(testFile);
    }

    TEST_F(CodeGenTest, CGFuncRetFn) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = CreateAST(testFile);

        // main()
        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        Node->addFunction(MainFn);

        // test()
        ASTFunc *TestFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "test");
        TestFn->setVisibility(V_PRIVATE);
        Node->addFunction(TestFn);

        ASTFuncCall *TestCall = new ASTFuncCall(SourceLoc, Ctx->getDefaultNameSpace()->getName(), TestFn->getName());
//        TestCall->addArg(new ValueExpr(SourceLoc, "1"));
        // call test()
        MainFn->getBody()->addCall(TestCall);
        //return test()
        MainFn->getBody()->addReturn(SourceLoc, new ASTFuncCallExpr(SourceLoc, TestCall));
        // Resolve Context for Resolutions of Call and Ref
        Node->Resolve();
        Ctx->Resolve();

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = call i32 @test()\n"
                          "  %1 = call i32 @test()\n"
                          "  ret i32 %1\n"
                          "}\n");
        delete Node;
    }

    TEST_F(CodeGenTest, CGGroupExpr) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = CreateAST(testFile);

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTVoidType(SourceLoc), "main");
        MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "b");
        MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "c");
        Node->addFunction(MainFn);

        // Create this expression: 1 + a * b / (c - 2)
        ASTGroupExpr *Group = new ASTGroupExpr(SourceLoc);
        Group->Add(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1",
                                                            new ASTIntType(SourceLoc))));
        Group->Add(new ASTArithExpr(SourceLoc, ARITH_ADD));
        Group->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(SourceLoc, "a")));
        Group->Add(new ASTArithExpr(SourceLoc, ARITH_MUL));
        Group->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(SourceLoc, "b")));
        Group->Add(new ASTArithExpr(SourceLoc, ARITH_DIV));

        ASTGroupExpr *SubGroup = new ASTGroupExpr(SourceLoc);
        SubGroup->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(SourceLoc, "c")));
        SubGroup->Add(new ASTArithExpr(SourceLoc, ARITH_SUB));
        SubGroup->Add(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "2",
                                                               new ASTIntType(SourceLoc))));
        Group->Add(SubGroup);

        MainFn->getBody()->addReturn(SourceLoc, Group);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @main(i32 %0, i32 %1, i32 %2) {\n"
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
        delete Node;
        deleteTestFile(testFile);
    }

    TEST_F(CodeGenTest, CGArithOp) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = CreateAST(testFile);

        // main()
        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        Node->addFunction(MainFn);

        ASTLocalVar *A = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTIntType(SourceLoc), "A");
        ASTLocalVar *B = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTIntType(SourceLoc), "B");
        ASTLocalVar *C = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTIntType(SourceLoc), "C");
        MainFn->getBody()->addVar(A);
        MainFn->getBody()->addVar(B);

        // Operation Add
        ASTGroupExpr *GroupAdd = new ASTGroupExpr(SourceLoc);
        GroupAdd->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupAdd->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_ADD));
        GroupAdd->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        C->setExpr(GroupAdd);
        MainFn->getBody()->addVar(C);

        // Operation Sub
        ASTLocalVarRef *Csub = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupSub = new ASTGroupExpr(SourceLoc);
        GroupSub->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupSub->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_SUB));
        GroupSub->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Csub->setExpr(GroupSub);
        MainFn->getBody()->addVar(Csub);

        // Operation Mul
        ASTLocalVarRef *Cmul = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupMul = new ASTGroupExpr(SourceLoc);
        GroupMul->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupMul->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_MUL));
        GroupMul->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cmul->setExpr(GroupMul);
        MainFn->getBody()->addVar(Cmul);

        // Operation Div
        ASTLocalVarRef *Cdiv = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupDiv = new ASTGroupExpr(SourceLoc);
        GroupDiv->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupDiv->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_DIV));
        GroupDiv->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cdiv->setExpr(GroupDiv);
        MainFn->getBody()->addVar(Cdiv);

        // Operation Mod
        ASTLocalVarRef *Cmod = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupMod = new ASTGroupExpr(SourceLoc);
        GroupMod->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupMod->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_MOD));
        GroupMod->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cmod->setExpr(GroupMod);
        MainFn->getBody()->addVar(Cmod);

        // Operation And
        ASTLocalVarRef *Cand = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupAnd = new ASTGroupExpr(SourceLoc);
        GroupAnd->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupAnd->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_AND));
        GroupAnd->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cand->setExpr(GroupAnd);
        MainFn->getBody()->addVar(Cand);

        // Operation Or
        ASTLocalVarRef *Cor = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupOr = new ASTGroupExpr(SourceLoc);
        GroupOr->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupOr->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_OR));
        GroupOr->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cor->setExpr(GroupOr);
        MainFn->getBody()->addVar(Cor);

        // Operation Xor
        ASTLocalVarRef *Cxor = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupXor = new ASTGroupExpr(SourceLoc);
        GroupXor->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupXor->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_XOR));
        GroupXor->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cxor->setExpr(GroupXor);
        MainFn->getBody()->addVar(Cxor);

        // Operation Shl
        ASTLocalVarRef *Cshl = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupShl = new ASTGroupExpr(SourceLoc);
        GroupShl->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupShl->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_SHIFT_L));
        GroupShl->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cshl->setExpr(GroupShl);
        MainFn->getBody()->addVar(Cshl);

        // Operation Shr
        ASTLocalVarRef *Cshr = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupShr = new ASTGroupExpr(SourceLoc);
        GroupShr->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupShr->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_SHIFT_R));
        GroupShr->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cshr->setExpr(GroupShr);
        MainFn->getBody()->addVar(Cshr);

        //return test()
        MainFn->getBody()->addReturn(SourceLoc, new ASTVarRefExpr(SourceLoc, new ASTVarRef(C)));
        // Resolve Context for Resolutions of Call and Ref
        Node->Resolve();
        Ctx->Resolve();

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca i32, align 4\n"
                          "  %1 = alloca i32, align 4\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store i32 0, i32* %0, align 4\n"
                          "  store i32 0, i32* %1, align 4\n"
                          "  %3 = load i32, i32* %0, align 4\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  %5 = add i32 %3, %4\n"
                          "  store i32 %5, i32* %2, align 4\n"
                          "  %6 = sub i32 %3, %4\n"
                          "  store i32 %6, i32* %2, align 4\n"
                          "  %7 = mul i32 %3, %4\n"
                          "  store i32 %7, i32* %2, align 4\n"
                          "  %8 = sdiv i32 %3, %4\n"
                          "  store i32 %8, i32* %2, align 4\n"
                          "  %9 = srem i32 %3, %4\n"
                          "  store i32 %9, i32* %2, align 4\n"
                          "  %10 = and i32 %3, %4\n"
                          "  store i32 %10, i32* %2, align 4\n"
                          "  %11 = or i32 %3, %4\n"
                          "  store i32 %11, i32* %2, align 4\n"
                          "  %12 = xor i32 %3, %4\n"
                          "  store i32 %12, i32* %2, align 4\n"
                          "  %13 = shl i32 %3, %4\n"
                          "  store i32 %13, i32* %2, align 4\n"
                          "  %14 = ashr i32 %3, %4\n"
                          "  store i32 %14, i32* %2, align 4\n"
                          "  %15 = load i32, i32* %2, align 4\n"
                          "  ret i32 %15\n"
                          "}\n");
        delete Node;
    }

    TEST_F(CodeGenTest, CGComparatorOp) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = CreateAST(testFile);

        // main()
        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        Node->addFunction(MainFn);

        ASTLocalVar *A = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTIntType(SourceLoc), "A");
        ASTLocalVar *B = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTIntType(SourceLoc), "B");
        ASTLocalVar *C = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTBoolType(SourceLoc), "C");
        MainFn->getBody()->addVar(A);
        MainFn->getBody()->addVar(B);

        // Operation Equal
        ASTGroupExpr *GroupEq = new ASTGroupExpr(SourceLoc);
        GroupEq->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupEq->Add(new ASTComparisonExpr(SourceLoc, ComparisonOpKind::COMP_EQ));
        GroupEq->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        C->setExpr(GroupEq);
        MainFn->getBody()->addVar(C);

        // Operation Not Equal
        ASTLocalVarRef *Cneq = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupNeq = new ASTGroupExpr(SourceLoc);
        GroupNeq->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupNeq->Add(new ASTComparisonExpr(SourceLoc, ComparisonOpKind::COMP_NE));
        GroupNeq->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cneq->setExpr(GroupNeq);
        MainFn->getBody()->addVar(Cneq);

        // Operation Greater Than
        ASTLocalVarRef *Cgt = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupGt = new ASTGroupExpr(SourceLoc);
        GroupGt->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupGt->Add(new ASTComparisonExpr(SourceLoc, ComparisonOpKind::COMP_GT));
        GroupGt->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cgt->setExpr(GroupGt);
        MainFn->getBody()->addVar(Cgt);

        // Operation Greater Than or Equal
        ASTLocalVarRef *Cgte = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupGte = new ASTGroupExpr(SourceLoc);
        GroupGte->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupGte->Add(new ASTComparisonExpr(SourceLoc, ComparisonOpKind::COMP_GTE));
        GroupGte->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cgte->setExpr(GroupGte);
        MainFn->getBody()->addVar(Cgte);

        // Operation Less Than
        ASTLocalVarRef *Clt = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupLt = new ASTGroupExpr(SourceLoc);
        GroupLt->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupLt->Add(new ASTComparisonExpr(SourceLoc, ComparisonOpKind::COMP_LT));
        GroupLt->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Clt->setExpr(GroupLt);
        MainFn->getBody()->addVar(Clt);

        // Operation Less Than or Equal
        ASTLocalVarRef *Clte = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupLte = new ASTGroupExpr(SourceLoc);
        GroupLte->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupLte->Add(new ASTComparisonExpr(SourceLoc, ComparisonOpKind::COMP_LTE));
        GroupLte->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Clte->setExpr(GroupLte);
        MainFn->getBody()->addVar(Clte);

        //return test()
        MainFn->getBody()->addReturn(SourceLoc, new ASTVarRefExpr(SourceLoc, new ASTVarRef(C)));
        // Resolve Context for Resolutions of Call and Ref
        Node->Resolve();
        Ctx->Resolve();

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca i32, align 4\n"
                          "  %1 = alloca i32, align 4\n"
                          "  %2 = alloca i1, align 1\n"
                          "  store i32 0, i32* %0, align 4\n"
                          "  store i32 0, i32* %1, align 4\n"
                          "  %3 = load i32, i32* %0, align 4\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  %5 = icmp eq i32 %3, %4\n"
                          "  store i1 %5, i1* %2, align 1\n"
                          "  %6 = icmp ne i32 %3, %4\n"
                          "  store i1 %6, i1* %2, align 1\n"
                          "  %7 = icmp sgt i32 %3, %4\n"
                          "  store i1 %7, i1* %2, align 1\n"
                          "  %8 = icmp sge i32 %3, %4\n"
                          "  store i1 %8, i1* %2, align 1\n"
                          "  %9 = icmp slt i32 %3, %4\n"
                          "  store i1 %9, i1* %2, align 1\n"
                          "  %10 = icmp sle i32 %3, %4\n"
                          "  store i1 %10, i1* %2, align 1\n"
                          "  %11 = load i1, i1* %2, align 1\n"
                          "  ret i1 %11\n"
                          "}\n");
        delete Node;
    }

    TEST_F(CodeGenTest, CGLogicOp) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = CreateAST(testFile);

        // main()
        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        Node->addFunction(MainFn);

        ASTLocalVar *A = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTBoolType(SourceLoc), "A");
        ASTLocalVar *B = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTBoolType(SourceLoc), "B");
        ASTLocalVar *C = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTBoolType(SourceLoc), "C");
        MainFn->getBody()->addVar(A);
        MainFn->getBody()->addVar(B);
        MainFn->getBody()->addVar(C);

        // Operation And Logic
        ASTGroupExpr *GroupAnd = new ASTGroupExpr(SourceLoc);
        GroupAnd->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupAnd->Add(new ASTLogicExpr(SourceLoc, LogicOpKind::LOGIC_AND));
        GroupAnd->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        C->setExpr(GroupAnd);

        // Operation Or Logic
        ASTLocalVarRef *Cor = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupOr = new ASTGroupExpr(SourceLoc);
        GroupOr->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupOr->Add(new ASTLogicExpr(SourceLoc, LogicOpKind::LOGIC_OR));
        GroupOr->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cor->setExpr(GroupOr);
        MainFn->getBody()->addVar(Cor);

        //return test()
        MainFn->getBody()->addReturn(SourceLoc, new ASTVarRefExpr(SourceLoc, new ASTVarRef(C)));
        // Resolve Context for Resolutions of Call and Ref
        Node->Resolve();
        Ctx->Resolve();

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca i1, align 1\n"
                          "  %1 = alloca i1, align 1\n"
                          "  %2 = alloca i1, align 1\n"
                          "  store i1 false, i1* %0, align 1\n"
                          "  store i1 false, i1* %1, align 1\n"
                          "  %3 = load i1, i1* %0, align 1\n"
                          "  br i1 %3, label %and, label %and1\n"
                          "\n"
                          "and:                                              ; preds = %entry\n"
                          "  %4 = load i1, i1* %1, align 1\n"
                          "  br label %and1\n"
                          "\n"
                          "and1:                                             ; preds = %and, %entry\n"
                          "  %5 = phi i1 [ false, %entry ], [ %4, %and ]\n"
                          "  %6 = zext i1 %5 to i8\n"
                          "  store i8 %6, i1* %2, align 1\n"
                          "  %7 = load i1, i1* %0, align 1\n"
                          "  br i1 %7, label %or2, label %or\n"
                          "\n"
                          "or:                                               ; preds = %and1\n"
                          "  %8 = load i1, i1* %1, align 1\n"
                          "  br label %or2\n"
                          "\n"
                          "or2:                                              ; preds = %or, %and1\n"
                          "  %9 = phi i1 [ true, %and1 ], [ %8, %or ]\n"
                          "  %10 = zext i1 %9 to i8\n"
                          "  store i8 %10, i1* %2, align 1\n"
                          "  %11 = load i1, i1* %2, align 1\n"
                          "  ret i1 %11\n"
                          "}\n");
        delete Node;
    }

    TEST_F(CodeGenTest, CGIfBlock) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = CreateAST(testFile);

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->addFunction(MainFn);

        // Compare
        ASTValueExpr *OneCost = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        ASTGroupExpr *Group = new ASTGroupExpr(SourceLoc);
        ASTComparisonExpr *Comp = new ASTComparisonExpr(SourceLoc, COMP_EQ);
        ASTVarRefExpr *ARef = new ASTVarRefExpr(SourceLoc, new ASTVarRef(Param));
        Group->Add(ARef);
        Group->Add(Comp);
        Group->Add(OneCost);

        // First If
        ASTIfBlock *IfBlock = new ASTIfBlock(SourceLoc, MainFn->getBody(), Group);
        ASTLocalVarRef *A2 = new ASTLocalVarRef(SourceLoc, IfBlock, Param);
        A2->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc))));
        IfBlock->addVar(A2);
        MainFn->getBody()->addBlock(SourceLoc, IfBlock);
        MainFn->getBody()->addReturn(SourceLoc, ARef);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
                          "entry:\n"
                          "  %1 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %1, align 4\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  %3 = icmp eq i32 %2, 1\n"
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
        delete Node;
        deleteTestFile(testFile);
    }

    TEST_F(CodeGenTest, CGIfElseBlock) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = CreateAST(testFile);

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->addFunction(MainFn);

        ASTValueExpr *OneCost = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        ASTGroupExpr *Group = new ASTGroupExpr(SourceLoc);
        ASTComparisonExpr *Comp = new ASTComparisonExpr(SourceLoc, COMP_EQ);
        ASTVarRefExpr *ARef = new ASTVarRefExpr(SourceLoc, new ASTVarRef(Param));
        Group->Add(ARef);
        Group->Add(Comp);
        Group->Add(OneCost);

        ASTIfBlock *IfBlock = new ASTIfBlock(SourceLoc, MainFn->getBody(), Group);
        ASTLocalVarRef *A2 = new ASTLocalVarRef(SourceLoc, IfBlock, Param);
        A2->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc))));
        IfBlock->addVar(A2);
        MainFn->getBody()->addBlock(SourceLoc, IfBlock);

        ASTElseBlock *ElseBlock = new ASTElseBlock(SourceLoc, MainFn->getBody());
        ASTLocalVarRef *A3 = new ASTLocalVarRef(SourceLoc, ElseBlock, Param);
        A3->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "2", new ASTIntType(SourceLoc))));
        ElseBlock->addVar(A3);
        IfBlock->AddBranch(MainFn->getBody(), ElseBlock);

        MainFn->getBody()->addReturn(SourceLoc, ARef);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
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
        delete Node;
        deleteTestFile(testFile);
    }

    TEST_F(CodeGenTest, CGIfElsifElseBlock) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = CreateAST(testFile);

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->addFunction(MainFn);

        ASTValueExpr *OneCost = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        ASTGroupExpr *Group = new ASTGroupExpr(SourceLoc);
        ASTComparisonExpr *Comp = new ASTComparisonExpr(SourceLoc, COMP_EQ);
        ASTVarRefExpr *ARef = new ASTVarRefExpr(SourceLoc, new ASTVarRef(Param));
        Group->Add(ARef);
        Group->Add(Comp);
        Group->Add(OneCost);

        ASTIfBlock *IfBlock = new ASTIfBlock(SourceLoc, MainFn->getBody(), Group);
        ASTLocalVarRef *A2 = new ASTLocalVarRef(SourceLoc, IfBlock, Param);
        A2->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc))));
        IfBlock->addVar(A2);
        MainFn->getBody()->addBlock(SourceLoc, IfBlock);

        ASTElsifBlock *ElsifBlock = new ASTElsifBlock(SourceLoc, MainFn->getBody(), Group);
        ASTLocalVarRef *A3 = new ASTLocalVarRef(SourceLoc, ElsifBlock, Param);
        A3->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "2", new ASTIntType(SourceLoc))));
        ElsifBlock->addVar(A3);
        IfBlock->AddBranch(MainFn->getBody(), ElsifBlock);

        ASTElsifBlock *Elsif2Block = new ASTElsifBlock(SourceLoc, MainFn->getBody(), Group);
        ASTLocalVarRef *A4 = new ASTLocalVarRef(SourceLoc, Elsif2Block, Param);
        A4->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "3", new ASTIntType(SourceLoc))));
        Elsif2Block->addVar(A4);
        IfBlock->AddBranch(MainFn->getBody(), Elsif2Block);

        ASTElseBlock *ElseBlock = new ASTElseBlock(SourceLoc, MainFn->getBody());
        ASTLocalVarRef *A5 = new ASTLocalVarRef(SourceLoc, ElseBlock, Param);
        A5->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "4", new ASTIntType(SourceLoc))));
        ElseBlock->addVar(A5);
        IfBlock->AddBranch(MainFn->getBody(), ElseBlock);

        MainFn->getBody()->addReturn(SourceLoc, ARef);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
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
        delete Node;
        deleteTestFile(testFile);
    }

    TEST_F(CodeGenTest, CGSwitchBlock) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = CreateAST(testFile);

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->addFunction(MainFn);

        ASTExpr *Cost1 = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        ASTExpr *Cost2 = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "2", new ASTIntType(SourceLoc)));

        ASTVarRefExpr *SwitchExpr = new ASTVarRefExpr(SourceLoc, new ASTVarRef(Param));
        ASTSwitchBlock *SwitchBlock = new ASTSwitchBlock(SourceLoc, MainFn->getBody(), SwitchExpr);
        
        ASTSwitchCaseBlock *Case1Block = SwitchBlock->AddCase(SourceLoc, Cost1);
        ASTLocalVarRef *A2 = new ASTLocalVarRef(SourceLoc, Case1Block, Param);
        A2->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc))));
        Case1Block->addVar(A2);

        ASTSwitchCaseBlock *Case2Block = SwitchBlock->AddCase(SourceLoc, Cost2);
        ASTLocalVarRef *A3 = new ASTLocalVarRef(SourceLoc, Case2Block, Param);
        A3->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "2", new ASTIntType(SourceLoc))));
        Case2Block->addVar(A3);
        Case2Block->addBreak(SourceLoc);

        ASTBlock *DefaultBlock = SwitchBlock->setDefault(SourceLoc);
        ASTLocalVarRef *A4 = new ASTLocalVarRef(SourceLoc, DefaultBlock, Param);
        A4->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "3", new ASTIntType(SourceLoc))));
        DefaultBlock->addVar(A4);
        DefaultBlock->addBreak(SourceLoc);

        MainFn->getBody()->addBlock(SourceLoc, SwitchBlock);
        
        MainFn->getBody()->addReturn(SourceLoc, Cost1);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
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
                          "  ret i32 1\n"
                          "}\n");
        delete Node;
        deleteTestFile(testFile);
    }

    TEST_F(CodeGenTest, CGWhileBlock) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTNode *Node = CreateAST(testFile);

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->addFunction(MainFn);

        ASTValueExpr *OneCost = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        ASTGroupExpr *Group = new ASTGroupExpr(SourceLoc);
        ASTComparisonExpr *Comp = new ASTComparisonExpr(SourceLoc, COMP_EQ);
        ASTVarRefExpr *ARef = new ASTVarRefExpr(SourceLoc, new ASTVarRef(Param));
        Group->Add(ARef);
        Group->Add(Comp);
        Group->Add(OneCost);
        ASTWhileBlock *WhileBlock = new ASTWhileBlock(SourceLoc, MainFn->getBody(), Group);

        ASTLocalVarRef *A2 = new ASTLocalVarRef(SourceLoc, WhileBlock, Param);
        A2->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc))));
        WhileBlock->addVar(A2);
        WhileBlock->addContinue(SourceLoc);
        MainFn->getBody()->addBlock(SourceLoc, WhileBlock);

        MainFn->getBody()->addReturn(SourceLoc, OneCost);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
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
                          "  ret i32 1\n"
                          "}\n");
        delete Node;
        deleteTestFile(testFile);
    }

    TEST_F(CodeGenTest, CGForBlock) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = CreateAST(testFile);

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        MainFn->setVisibility(V_PRIVATE);
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->addFunction(MainFn);
        
        ASTForBlock *ForBlock = new ASTForBlock(SourceLoc, MainFn->getBody());
        ASTValueExpr *OneCost = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        
        // Init
        ASTBlock *InitBlock = ForBlock->getInit();
        ASTLocalVar *InitVar = new ASTLocalVar(SourceLoc, InitBlock, new ASTIntType(SourceLoc), "i");
        InitVar->setExpr(OneCost);
        InitBlock->addVar(InitVar);

        //Cond
        ASTGroupExpr *Cond = new ASTGroupExpr(SourceLoc);
        ASTComparisonExpr *Comp = new ASTComparisonExpr(SourceLoc, COMP_LTE);
        ASTVarRefExpr *InitVarRef = new ASTVarRefExpr(SourceLoc, new ASTVarRef(InitVar));
        Cond->Add(InitVarRef);
        Cond->Add(Comp);
        Cond->Add(OneCost);
        ForBlock->setCond(Cond);
        
        // Post
        ASTBlock *PostBlock = ForBlock->getPost();
        ASTArithExpr *Incr = new ASTArithExpr(SourceLoc, ARITH_INCR);
        ASTUnaryExpr *IncrExpr = new ASTUnaryExpr(SourceLoc, Incr, new ASTVarRef(InitVar), UnaryOpKind::UNARY_PRE);
        ASTExprStmt *ExprStmt = new ASTExprStmt(SourceLoc, PostBlock);
        ExprStmt->setExpr(IncrExpr);
        PostBlock->addExprStmt(ExprStmt);

        ASTLocalVarRef *A2 = new ASTLocalVarRef(SourceLoc, ForBlock->getLoop(), Param);
        A2->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc))));
        ForBlock->getLoop()->addVar(A2);
        ForBlock->getLoop()->addContinue(SourceLoc);
        MainFn->getBody()->addBlock(SourceLoc, ForBlock);

        MainFn->getBody()->addReturn(SourceLoc, OneCost);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
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
                          "  ret i32 1\n"
                          "}\n");
        delete Node;
        deleteTestFile(testFile);
    }

} // anonymous namespace
