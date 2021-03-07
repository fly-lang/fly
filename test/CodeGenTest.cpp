//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <Frontend/CompilerInvocation.h>
#include <CodeGen/CodeGen.h>
#include "Basic/TargetOptions.h"
#include <llvm/Support/Host.h>
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

        IntrusiveRefCntPtr<DiagnosticIDs> diagID;
        DiagnosticsEngine diags;

        CodeGenTest() :
                diagID(new DiagnosticIDs()),
                diags(diagID, new DiagnosticOptions, new IgnoringDiagConsumer())
                {}

        IntrusiveRefCntPtr<TargetInfo> createTargetInfo() {
            const std::shared_ptr<fly::TargetOptions> targetOpts = std::make_shared<fly::TargetOptions>();
            targetOpts->Triple = llvm::sys::getDefaultTargetTriple();
            return TargetInfo::CreateTargetInfo(diags, targetOpts);
        }

    };

    bool createTestFile(const char* testFile) {
        std::fstream my_file;
        my_file.open(testFile, std::ios::out);
        if (my_file) {
            std::cout << "File " << testFile << " created successfully!";
            my_file.close();
        } else {
            std::cout << "Error File " << testFile << " not created!";
        }
        return (bool) my_file;
    }

    void deleteTestFile(const char* testFile) {
        remove(testFile);
    }

    TEST_F(CodeGenTest, SmokeTest) {

        EXPECT_TRUE(createTestFile(testFile));

        const StringRef strRef = "file.fly";
        const std::string &str = std::string(strRef);
        const PackageDecl packageDecl(str);
        ASTContext astContext(strRef, packageDecl);

        const CodeGen &codeGen = CodeGen(diags, astContext, *createTargetInfo());
        codeGen.execute();

        ASSERT_TRUE(codeGen.execute());
        deleteTestFile(testFile);
    }

} // anonymous namespace
