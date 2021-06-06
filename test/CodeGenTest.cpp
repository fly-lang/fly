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
#include "CodeGen/CGFunction.h"
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
//FIXME TESTS
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

        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        TargetOpts->CodeModel = "default";
        CodeGenModule CGM(Diags, Node, *CodeGen::CreateTargetInfo(Diags, TargetOpts));

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
        FuncDecl *Func = new FuncDecl(SourceLoc, new IntPrimType(SourceLoc), "testFunc");
        Node.addFunction(Func);

        std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        CodeGenModule CGM(Diags, Node, *CodeGen::CreateTargetInfo(Diags, TargetOpts));

        Function *F = CGM.GenFunction(Func)->getFunction();

        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "declare i32 @testFunc() addrspace(0)\n");
    }

} // anonymous namespace
