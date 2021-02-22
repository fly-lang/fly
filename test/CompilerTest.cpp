//===- test/CompilerTest.cpp ------ Compiler tests ------------------------===//
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#include <llvm/Option/OptTable.h>
#include <Driver/Options.h>
#include <llvm/Option/ArgList.h>
#include <Basic/FileManager.h>
#include <Compiler/Compiler.h>
#include <Compiler/CompilerInvocation.h>
#include <vector>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
    using namespace fly;
    using namespace llvm;
    using testing::ElementsAre;

    static const char* testFile = "test.fly";

    // The test fixture.
    class CompilerTest : public ::testing::Test {

    public:
        CompilerTest() {

        }

    };

    void createTestFile() {
        struct stat buf;
        if (stat(testFile, &buf) == -1) {
            // Create and open a text file
            std::ofstream MyFile(testFile);

            // Write to the file
            MyFile << "Files can be tricky, but it is fun enough!";

            // Close the file
            MyFile.close();
        }
    }

    bool deleteTestFile() {
        return remove(testFile);
    }

    llvm::opt::InputArgList parseArgs(ArrayRef<const char *> ArgList) {
        const llvm::opt::OptTable &table = fly::driver::getDriverOptTable();
        unsigned MissingArgIndex, MissingArgCount;
        unsigned IncludedFlagsBitmask;
        unsigned ExcludedFlagsBitmask;
        return table.ParseArgs(ArgList, MissingArgIndex, MissingArgCount,
                                                       IncludedFlagsBitmask, ExcludedFlagsBitmask);
    }

    TEST_F(CompilerTest, OptTableArgs) {
        // Check for working directory option before accessing any files
        llvm::opt::InputArgList Args = parseArgs({"Wno-write-strings", "file1.fly"});
        for (auto &Arg : Args) {
            Arg->getOption();
        }
    }

    TEST_F(CompilerTest, FileMgr) {

        int argc = 2;
        const char *argv[] = {"fly", testFile};

        createTestFile();

        Compiler compiler = Compiler(argc, argv);
        CompilerInvocation &invocation = compiler.getInvocation();

        EXPECT_EQ(invocation.inputFiles.size(), compiler.getInstances().size());

        /// A lookup of in-memory (virtual file) buffers
        auto Buf = invocation.getFileManager().getBufferForFile(argv[1]);
        EXPECT_FALSE(Buf.getError());

        deleteTestFile();
    }

    TEST_F(CompilerTest, CompilerExecute) {
        int argc = 2;
        const char *argv[] = {"fly", testFile};

        createTestFile();

        Compiler compiler = Compiler(argc, argv);

        ASSERT_TRUE(compiler.execute());

        deleteTestFile();
    }

} // anonymous namespace
