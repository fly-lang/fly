//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <Frontend/CompilerInstance.h>
#include <CodeGen/CodeGen.h>
#include "Basic/TargetOptions.h"
#include <llvm/Support/Host.h>
#include "gtest/gtest.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <Basic/Builtins.h>

namespace {
    using namespace fly;

    const char* testFile = "test.fly";

    // The test fixture.
    class CodeGenTest : public ::testing::Test {

    public:

        IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
        DiagnosticsEngine Diags;

        CodeGenTest() :
                DiagID(new DiagnosticIDs()),
                Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer())
                {}

        TargetInfo* CreateTargetInfo() {
            const std::shared_ptr<fly::TargetOptions> TargetOpts = std::make_shared<fly::TargetOptions>();
            //targetOpts->Triple = llvm::sys::getDefaultTargetTriple();
            TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
            TargetInfo *target = TargetInfo::CreateTargetInfo(Diags, TargetOpts);
            return target;
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
    }

    TEST_F(CodeGenTest, EmitNothing) {

        const StringRef strRef = "file.fly";
        const std::string &str = std::string(strRef);
        const PackageDecl packageDecl(str);
        ASTContext astContext(strRef, packageDecl);
        CodeGenOptions codeGenOpts;
        fly::TargetOptions targetOpts;

        CodeGen CG(Diags, codeGenOpts, targetOpts, astContext,
                   *CreateTargetInfo(), Backend_EmitNothing);
        bool Success = CG.execute();

        ASSERT_TRUE(Success);
    }

    TEST_F(CodeGenTest, EmitLL) {

        EXPECT_TRUE(createTestFile(testFile));

        const StringRef strRef = "file.fly";
        const std::string &str = std::string(strRef);
        const PackageDecl packageDecl(str);
        ASTContext astContext(strRef, packageDecl);
        CodeGenOptions codeGenOpts;
        fly::TargetOptions targetOpts;

        CodeGen CG(Diags, codeGenOpts, targetOpts, astContext,
                   *CreateTargetInfo(), Backend_EmitLL);
        bool Success = CG.execute();

        CG.getModule()->print(llvm::outs(), nullptr);

        ASSERT_TRUE(Success);
        read();
        deleteTestFile(testFile);
    }

    TEST_F(CodeGenTest, EmitBC) {

        EXPECT_TRUE(createTestFile(testFile));

        const StringRef strRef = "file.fly";
        const std::string &str = std::string(strRef);
        const PackageDecl packageDecl(str);
        ASTContext astContext(strRef, packageDecl);
        CodeGenOptions codeGenOpts;
        fly::TargetOptions targetOpts;

        CodeGen CG(Diags, codeGenOpts, targetOpts, astContext,
                   *CreateTargetInfo(), Backend_EmitBC);
        bool Success = CG.execute();

        ASSERT_TRUE(Success);
        deleteTestFile(testFile);
    }

    TEST_F(CodeGenTest, EmitAS) {

        EXPECT_TRUE(createTestFile(testFile));

        const StringRef strRef = "file.fly";
        const std::string &str = std::string(strRef);
        const PackageDecl packageDecl(str);
        ASTContext astContext(strRef, packageDecl);
        CodeGenOptions codeGenOpts;
        fly::TargetOptions targetOpts;

        CodeGen CG(Diags, codeGenOpts, targetOpts, astContext,
                   *CreateTargetInfo(), Backend_EmitAssembly);
        bool Success = CG.execute();

        ASSERT_TRUE(Success);
        deleteTestFile(testFile);
    }

    TEST_F(CodeGenTest, EmitOBJ) {

        EXPECT_TRUE(createTestFile(testFile));

        const StringRef strRef = "file.fly";
        const std::string &str = std::string(strRef);
        const PackageDecl Package(str);
        ASTContext Ctx(strRef, Package);
        CodeGenOptions CodeGenOpts;
        fly::TargetOptions TargetOpts;

        CodeGen CG(Diags, CodeGenOpts, TargetOpts, Ctx,
                   *CreateTargetInfo(), Backend_EmitObj);
        bool Success = CG.execute();

        ASSERT_TRUE(Success);
        deleteTestFile(testFile);
    }

} // anonymous namespace
