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
#include <vector>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
    using namespace fly;
    using testing::ElementsAre;

    static const char* testFile = "test.fly";

    // The test fixture.
    class FrontendTest : public ::testing::Test {

    public:
        FrontendTest() {}

    };

    void createTestFile() {
        std::fstream my_file;
        my_file.open(testFile, std::ios::out);
        if (!my_file) {
            std::cout << "File not created!";
        }
        else {
            std::cout << "File created successfully!";
            my_file.close();
        }
    }

    bool deleteTestFile() {
        return remove(testFile);
    }

    TEST_F(FrontendTest, FileMgr) {
        createTestFile();

        Driver driver;
        FrontendOptions &options = driver.getInvocation()->getFrontendOptions();
        options.addInputFile(testFile);
        Frontend frontend(*driver.getInvocation());
        CompilerInvocation &invocation = frontend.getInvocation();

        EXPECT_EQ(options.getInputFiles().size(), frontend.getInstances().size());

        /// A lookup of in-memory (virtual file) buffers
        auto Buf = invocation.getFileManager().getBufferForFile(testFile);
        EXPECT_FALSE(Buf.getError());

        deleteTestFile();
    }

    TEST_F(FrontendTest, CompilerExecute) {
        FrontendOptions options;

        createTestFile();

        Driver driver;
        Frontend frontend(*driver.getInvocation());

        ASSERT_TRUE(frontend.execute());

        deleteTestFile();
    }

} // anonymous namespace
