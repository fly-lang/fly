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
#include "CodeGen/CodeGenFunction.h"
#include "Basic/Diagnostic.h"
#include "Basic/DiagnosticOptions.h"
#include "Basic/FileManager.h"
#include "Basic/SourceLocation.h"
#include "Basic/SourceManager.h"
#include "Basic/TargetOptions.h"
#include "Basic/Builtins.h"
#include "Frontend/CompilerInstance.h"
#include "llvm/Support/Host.h"
#include "gtest/gtest.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <llvm/Support/TargetSelect.h>


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

        ASTNode createAST(const llvm::StringRef Name, ASTContext *Ctx, const StringRef NameSpace = "default") {
            ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> result = FileMgr.getBufferForFile(Name);
            std::unique_ptr<MemoryBuffer> &Buf = result.get();
            FileID FID = SourceMgr.createFileID(std::move(Buf));
            SourceLoc = SourceMgr.getLocForStartOfFile(FID);
            auto Node = ASTNode(Name, FID, Ctx);
            Node.setNameSpace(NameSpace);
            Node.Finalize();
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
        ASTNode Node = createAST(testFile, Ctx);

        CodeGenOptions codeGenOpts;
        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        CodeGen CG(Diags, codeGenOpts, TargetOpts, *Ctx, Backend_EmitNothing);
        bool Success = CG.Execute();

        ASSERT_TRUE(Success);
        deleteTestFile(testFile);
    }

//    TEST_F(CodeGenTest, EmitLL) {
//
//        EXPECT_TRUE(createTestFile(testFile));
//
//        ASTContext *Ctx = new ASTContext(Diags);
//        ASTNode Node = createAST(testFile, Ctx);
//
//        CodeGenOptions codeGenOpts;
//        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
//        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
//        codeGenOpts.CodeModel = "default";
//        TargetOpts->CodeModel = "default";
//        CodeGen CG(Diags, codeGenOpts, TargetOpts, *Ctx, Backend_EmitLL);
//        bool Success = CG.Execute();
//
//        CG.getModule()->print(llvm::outs(), nullptr);
//
//        ASSERT_TRUE(Success);
//        read();
//        deleteTestFile(testFile);
//    }
//
//    TEST_F(CodeGenTest, EmitBC) {
//
//        EXPECT_TRUE(createTestFile(testFile));
//
//        ASTContext *Ctx = new ASTContext(Diags);
//        ASTNode Node = createAST(testFile, Ctx);
//
//        CodeGenOptions codeGenOpts;
//        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
//        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
//        TargetOpts->CodeModel = "default";
//        CodeGen CG(Diags, codeGenOpts, TargetOpts, *Ctx, Backend_EmitBC);
//        bool Success = CG.Execute();
//
//        ASSERT_TRUE(Success);
//        deleteTestFile(testFile);
//    }
//
//    TEST_F(CodeGenTest, EmitAS) {
//
//        EXPECT_TRUE(createTestFile(testFile));
//
//        ASTContext *Ctx = new ASTContext(Diags);
//        ASTNode Node = createAST(testFile, Ctx);
//
//        CodeGenOptions codeGenOpts;
//        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
//        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
//        TargetOpts->CodeModel = "default";
//        CodeGen CG(Diags, codeGenOpts, TargetOpts, *Ctx, Backend_EmitAssembly);
//        bool Success = CG.Execute();
//
//        ASSERT_TRUE(Success);
//        deleteTestFile(testFile);
//    }
//
//    TEST_F(CodeGenTest, EmitOBJ) {
//
//        EXPECT_TRUE(createTestFile(testFile));
//
//        ASTContext *Ctx = new ASTContext(Diags);
//        ASTNode Node = createAST(testFile, Ctx);
//
//        CodeGenOptions CodeGenOpts;
//        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
//        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
//        TargetOpts->CodeModel = "default";
//        CodeGen CG(Diags, CodeGenOpts, TargetOpts, *Ctx, Backend_EmitObj);
//        bool Success = CG.Execute();
//
//        ASSERT_TRUE(Success);
//        deleteTestFile(testFile);
//    }

    TEST_F(CodeGenTest, CGGlobalVar) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode Node = createAST(testFile, Ctx);
        GlobalVarDecl *Var = new GlobalVarDecl(SourceLoc, new IntPrimType(SourceLoc), "a");
        Node.addGlobalVar(Var);

        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();

        CodeGenOptions CodeGenOpts;
        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        TargetOpts->CodeModel = "default";
        LLVMContext LLVMCtx;
        CodeGenModule CGM(Diags, Node, LLVMCtx, *CodeGen::CreateTargetInfo(Diags, TargetOpts), CodeGenOpts);

        GlobalVariable *GVar = CGM.GenGlobalVar(Var)->getGlobalVar();

        testing::internal::CaptureStdout();
        GVar->print(llvm::outs(), true);
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "@0 = external global i32");
    }

    TEST_F(CodeGenTest, CGFunc) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode Node = createAST(testFile, Ctx);
        
        FuncDecl *MainFn = new FuncDecl(SourceLoc, new IntPrimType(SourceLoc), "main");
        MainFn->addParam(SourceLoc, new IntPrimType(SourceLoc), "P1");
        MainFn->addParam(SourceLoc, new FloatPrimType(SourceLoc), "P2");
        MainFn->addParam(SourceLoc, new BoolPrimType(SourceLoc), "P3");
        Node.addFunction(MainFn);
        
        GroupExpr *Exp = new GroupExpr();
        Exp->Add(new ValueExpr(SourceLoc, "1"));
        MainFn->getBody()->addReturn(SourceLoc, Exp);

        CodeGenOptions CodeGenOpts;
        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        TargetOpts->CodeModel = "default";
        TargetInfo *Target = CodeGen::CreateTargetInfo(Diags, TargetOpts);
        LLVMContext LLVMCtx;
        CodeGenModule CGM(Diags, Node, LLVMCtx, *Target, CodeGenOpts);

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
    }

    TEST_F(CodeGenTest, CGFuncRetVar) {
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode Node = createAST(testFile, Ctx);

        GlobalVarDecl *GVar = new GlobalVarDecl(SourceLoc, new FloatPrimType(SourceLoc), "G");
        Node.addGlobalVar(GVar);

        FuncDecl *MainFn = new FuncDecl(SourceLoc, new IntPrimType(SourceLoc), "main");
        Node.addFunction(MainFn);

        // int A
        VarDeclStmt *LocalVar1 = new VarDeclStmt(SourceLoc, MainFn->getBody(), new IntPrimType(SourceLoc), "A");
        MainFn->getBody()->addVar(LocalVar1);

        // A = 1
        VarStmt * VStmt = new VarStmt(SourceLoc, MainFn->getBody(), LocalVar1);
        const llvm::StringRef StrVal = "1";
        GroupExpr *G1 = new GroupExpr();
        G1->Add(new ValueExpr(SourceLoc, StrVal));
        VStmt->setExpr(G1);
        MainFn->getBody()->addVar(VStmt);

        // G = 1
        VarStmt * GStmt = new VarStmt(SourceLoc, MainFn->getBody(), GVar);
        GStmt->setExpr(G1);
        MainFn->getBody()->addVar(GStmt);

        // return A
        GroupExpr *Exp = new GroupExpr();
        Exp->Add(new VarRefExpr(SourceLoc, new VarRef(SourceLoc, LocalVar1)));
        MainFn->getBody()->addReturn(SourceLoc, Exp);

        CodeGenOptions CodeGenOpts;
        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        TargetOpts->CodeModel = "default";
        TargetInfo *Target = CodeGen::CreateTargetInfo(Diags, TargetOpts);
        LLVMContext LLVMCtx;
        CodeGenModule CGM(Diags, Node, LLVMCtx, *Target, CodeGenOpts);

        CodeGenGlobalVar *G = CGM.GenGlobalVar(GVar);
        Function *F = CGM.GenFunction(MainFn)->getFunction();

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
        EXPECT_TRUE(createTestFile(testFile));

        ASTContext *Ctx = new ASTContext(Diags);
        ASTNode Node = createAST(testFile, Ctx);
        FuncDecl *MainFn = new FuncDecl(SourceLoc, new IntPrimType(SourceLoc), "main");
        Node.addFunction(MainFn);

        FuncDecl *TestFn = new FuncDecl(SourceLoc, new IntPrimType(SourceLoc), "test");
        Node.addFunction(MainFn);

        FuncCallStmt * FStmt = new FuncCallStmt(SourceLoc, MainFn->getBody(), TestFn);
        MainFn->getBody()->addCall(FStmt);

        GroupExpr *Exp = new GroupExpr();
        Exp->Add(new FuncRefExpr(SourceLoc, new FuncCall(SourceLoc, TestFn)));
        MainFn->getBody()->addReturn(SourceLoc, Exp);

        CodeGenOptions CodeGenOpts;
        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        TargetOpts->CodeModel = "default";
        TargetInfo *Target = CodeGen::CreateTargetInfo(Diags, TargetOpts);
        LLVMContext LLVMCtx;
        CodeGenModule CGM(Diags, Node, LLVMCtx, *Target, CodeGenOpts);

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
    }

} // anonymous namespace
