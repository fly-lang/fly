//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <Driver/Driver.h>
#include <Frontend/FrontendOptions.h>
#include <Frontend/Frontend.h>
#include <Frontend/CompilerInvocation.h>
#include "gtest/gtest.h"
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <vector>

namespace {
    using namespace fly;

    const char* testFile = "test.fly";

    // The test fixture.
    class FrontendTest : public ::testing::Test  {

    public:
        FrontendTest() {}
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

    TEST_F(FrontendTest, FileMgr) {
        EXPECT_TRUE(createTestFile(testFile));

        Driver driver;
        FrontendOptions &options = driver.getInvocation()->getFrontendOptions();
        options.addInputFile(testFile);
        Frontend frontend(*driver.getInvocation());
        CompilerInvocation &invocation = frontend.getInvocation();

        EXPECT_EQ(options.getInputFiles().size(), frontend.getInstances().size());

        /// A lookup of in-memory (virtual file) buffers
        auto Buf = invocation.getFileManager().getBufferForFile(testFile);
        EXPECT_FALSE(Buf.getError());

        deleteTestFile(testFile);
    }

    TEST_F(FrontendTest, ExecuteCompilerWithoutParse) {

        EXPECT_TRUE(createTestFile(testFile));

        Driver driver;
        FrontendOptions &frontendOpts = driver.getInvocation()->getFrontendOptions();
        frontendOpts.setSkipParse(); // do not parse input
        frontendOpts.addInputFile(testFile);
        Frontend frontend(*driver.getInvocation());

        ASSERT_TRUE(frontend.execute());

        deleteTestFile(testFile);
    }

} // anonymous namespace
