//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

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
#include "AST/ASTBlock.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTOperatorExpr.h"
#include "Basic/Diagnostic.h"
#include "Basic/DiagnosticOptions.h"
#include "Basic/FileManager.h"
#include "Basic/SourceLocation.h"
#include "Basic/SourceManager.h"
#include "Basic/TargetOptions.h"
#include "Basic/Builtins.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "gtest/gtest.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <AST/ASTOperatorExpr.h>


namespace {
    using namespace fly;

    const char* testFile = "test.fly";

    // The test fixture.
    class CodeGenTest : public ::testing::Test {

    public:

        CodeGenTest() :
                FileMgr(FileMgrOpts),
                DiagID(new DiagnosticIDs()),
                Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
                SourceMgr(Diags, FileMgr)
                {}

        FileSystemOptions FileMgrOpts;
        FileManager FileMgr;
        IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
        DiagnosticsEngine Diags;
        SourceManager SourceMgr;
        SourceLocation SourceLoc;

        ASTNode *createAST(const llvm::StringRef Name, ASTContext *Ctx, const StringRef NameSpace = "default") {
            ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> result = FileMgr.getBufferForFile(Name);
            std::unique_ptr<MemoryBuffer> &Buf = result.get();
            FileID FID = SourceMgr.createFileID(std::move(Buf));
            SourceLoc = SourceMgr.getLocForStartOfFile(FID);
            auto *Node = new ASTNode(Name, FID, Ctx);
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

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = createAST(testFile, Ctx);

        CodeGenOptions codeGenOpts;
        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        CodeGen CG(Diags, codeGenOpts, TargetOpts, *Ctx, Backend_EmitNothing);
        bool Success = CG.Execute();

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
//        ASTNode *Node = createAST(testFile, Ctx);
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
//        ASTNode *Node = createAST(testFile, Ctx);
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
//        ASTNode *Node = createAST(testFile, Ctx);
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
//        ASTNode *Node = createAST(testFile, Ctx);
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

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = createAST(testFile, Ctx);
        ASTGlobalVar *Var = new ASTGlobalVar(Node, SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->addGlobalVar(Var);

        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();

        CodeGenOptions CodeGenOpts;
        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        TargetOpts->CodeModel = "default";
        LLVMContext LLVMCtx;
        CodeGenModule CGM(Diags, *Node, LLVMCtx, *CodeGen::CreateTargetInfo(Diags, TargetOpts), CodeGenOpts);

        GlobalVariable *GVar = CGM.GenGlobalVar(Var)->getGlobalVar();

        testing::internal::CaptureStdout();
        GVar->print(llvm::outs(), true);
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "@a = external global i32");
        delete Node;
        deleteTestFile(testFile);
    }

    TEST_F(CodeGenTest, CGFunc) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = createAST(testFile, Ctx);
        
        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc),
                                      "main");
        MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "P1");
        MainFn->addParam(SourceLoc, new ASTFloatType(SourceLoc), "P2");
        MainFn->addParam(SourceLoc, new ASTBoolType(SourceLoc), "P3");
        Node->addFunction(MainFn);

        ASTValueExpr *Expr = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        MainFn->getBody()->addReturn(SourceLoc, Expr);

        CodeGenOptions CodeGenOpts;
        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        TargetOpts->CodeModel = "default";
        TargetInfo *Target = CodeGen::CreateTargetInfo(Diags, TargetOpts);
        LLVMContext LLVMCtx;
        CodeGenModule CGM(Diags, *Node, LLVMCtx, *Target, CodeGenOpts);

        Function *F = CGM.GenFunction(MainFn)->getFunction();

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

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = createAST(testFile, Ctx);

        ASTGlobalVar *GVar = new ASTGlobalVar(Node, SourceLoc, new ASTFloatType(SourceLoc), "G");
        Node->addGlobalVar(GVar);

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        Node->addFunction(MainFn);

        // int A
        ASTLocalVar *VarA = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTIntType(SourceLoc), "A");
        MainFn->getBody()->addVar(VarA);

        // A = 1
        ASTLocalVarStmt * VStmt = new ASTLocalVarStmt(SourceLoc, MainFn->getBody(), VarA->getName());
        ASTExpr *Expr = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        VStmt->setExpr(Expr);
        MainFn->getBody()->addVar(VStmt);

        // GlobalVar
        // G = 1
        ASTLocalVarStmt * GStmt = new ASTLocalVarStmt(SourceLoc, MainFn->getBody(), GVar->getName(), "default");
        GStmt->setExpr(Expr);
        MainFn->getBody()->addVar(GStmt);

        // return A
        MainFn->getBody()->addReturn(SourceLoc, new ASTVarRefExpr(SourceLoc,
                                                                  new ASTVarRef(SourceLoc, VarA->getName())));

        Node->Finalize();

        CodeGenOptions CodeGenOpts;
        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        TargetOpts->CodeModel = "default";
        TargetInfo *Target = CodeGen::CreateTargetInfo(Diags, TargetOpts);
        LLVMContext LLVMCtx;
        CodeGenModule CGM(Diags, *Node, LLVMCtx, *Target, CodeGenOpts);

        CodeGenGlobalVar *G = CGM.GenGlobalVar(GVar);
        Function *F = CGM.GenFunction(MainFn)->getFunction();

        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca i32, align 4\n"
                          "  store i32 1, i32* %0, align 4\n"
                          "  store i32 1, float* @G, align 4\n"
                          "  %1 = load i32, i32* %0, align 4\n"
                          "  ret i32 %1\n"
                          "}\n");
        delete Node;
        deleteTestFile(testFile);
    }

    TEST_F(CodeGenTest, CGFuncRetFn) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = createAST(testFile, Ctx);

        // main()
        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        Node->addFunction(MainFn);

        // test()
        ASTFunc *TestFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "test");
        TestFn->setVisibility(V_PRIVATE);
        Node->addFunction(TestFn);

        ASTFuncCall *TestCall = new ASTFuncCall(SourceLoc, Ctx->getDefaultNameSpace()->getNameSpace(), TestFn->getName());
//        TestCall->addArg(new ValueExpr(SourceLoc, "1"));
        // call test()
        MainFn->getBody()->addCall(TestCall);
        //return test()
        MainFn->getBody()->addReturn(SourceLoc, new ASTFuncCallExpr(SourceLoc, TestCall));
        // Finalize Context for Resolutions of Call and Ref
        Node->Finalize();
        Ctx->Finalize();

        CodeGenOptions CodeGenOpts;
        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        TargetOpts->CodeModel = "default";
        TargetInfo *Target = CodeGen::CreateTargetInfo(Diags, TargetOpts);
        LLVMContext LLVMCtx;
        CodeGenModule CGM(Diags, *Node, LLVMCtx, *Target, CodeGenOpts);

        // CodeGen of test function
        CGM.GenFunction(TestFn);

        Function *F = CGM.GenFunction(MainFn)->getFunction();

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

    TEST_F(CodeGenTest, CGExpr) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = createAST(testFile, Ctx);

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

        CodeGenOptions CodeGenOpts;
        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        TargetOpts->CodeModel = "default";
        TargetInfo *Target = CodeGen::CreateTargetInfo(Diags, TargetOpts);
        LLVMContext LLVMCtx;
        CodeGenModule CGM(Diags, *Node, LLVMCtx, *Target, CodeGenOpts);

        Function *F = CGM.GenFunction(MainFn)->getFunction();

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

    TEST_F(CodeGenTest, CGIfBlock) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode *Node = createAST(testFile, Ctx);

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTVoidType(SourceLoc), "main");
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->addFunction(MainFn);

        ASTValueExpr *OneCost = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        ASTGroupExpr *Group = new ASTGroupExpr(SourceLoc);
        ASTComparisonExpr *Comp = new ASTComparisonExpr(SourceLoc, COMP_EQ);
        Group->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(Param)));
        Group->Add(Comp);
        Group->Add(OneCost);
        ASTIfBlock *IfBlock = new ASTIfBlock(SourceLoc, MainFn->getBody(), Group);
        MainFn->getBody()->addBlock(SourceLoc, IfBlock);
        MainFn->getBody()->addReturn(SourceLoc, nullptr);

        CodeGenOptions CodeGenOpts;
        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        TargetOpts->CodeModel = "default";
        TargetInfo *Target = CodeGen::CreateTargetInfo(Diags, TargetOpts);
        LLVMContext LLVMCtx;
        CodeGenModule CGM(Diags, *Node, LLVMCtx, *Target, CodeGenOpts);

        Function *F = CGM.GenFunction(MainFn)->getFunction();

        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define void @main(i32 %0) {\n"
                          "entry:\n"
                          "  %1 = alloca i32, align 4\n"
                          "  store i32 1, i32* %0, align 4\n"
                          "  ret i32 %12\n"
                          "}\n");
        delete Node;
        deleteTestFile(testFile);
    }

} // anonymous namespace
