//===--------------------------------------------------------------------------------------------------------------===//
// test/DriverTest.cpp - Driver tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <Driver/Driver.h>
#include <Driver/DriverOptions.h>
#include <llvm/Option/OptTable.h>
#include <llvm/Option/ArgList.h>
#include "gtest/gtest.h"
#include <fstream>
#include <iostream>
#include <vector>

namespace {
    using namespace fly;
    using namespace fly::driver;

    const char* testFile = "file1.fly";

    // The test fixture.
    class DriverTest : public ::testing::Test {

    public:
        DriverTest() {}
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

    TEST_F(DriverTest, Options) {
        const opt::OptTable &optTab = fly::driver::getDriverOptTable();
        unsigned MissingArgIndex, MissingArgCount;
        const opt::InputArgList &args = optTab.ParseArgs({"-v", "-help", "-version", testFile, "-o", "file.o"},
                                                         MissingArgIndex, MissingArgCount);
        EXPECT_TRUE(args.hasArg(options::OPT_VERBOSE));
        EXPECT_TRUE(args.hasArg(options::OPT_HELP));
        EXPECT_TRUE(args.hasArg(options::OPT_VERSION));
        EXPECT_TRUE(args.hasArg(options::OPT_INPUT));
        EXPECT_TRUE(args.hasArg(options::OPT_OUTPUT));
        ASSERT_EQ(args.getNumInputArgStrings(), 6);
        EXPECT_EQ(testFile, args.getLastArg(options::OPT_INPUT)->getValue());
        EXPECT_EQ("file.o", args.getLastArgValue(options::OPT_OUTPUT));
    }

    TEST_F(DriverTest, DriverOptions) {
        EXPECT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", "-v", testFile, "-o", "file.o"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        const FrontendOptions &fopts = CI.getFrontendOptions();
        for(const InputFile &inputFile : fopts.getInputFiles()) {
            ASSERT_EQ(inputFile.getFile(), "file1.fly");
            break;
        }
        ASSERT_EQ(fopts.getOutputFile().getFile(), "file.o");
        ASSERT_TRUE(fopts.isVerbose());

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, PrintHelp) {
        const char *argv[] = {"fly", "-help"};
        Driver driver(argv);
        driver.BuildCompilerInstance();
        EXPECT_TRUE(driver.Execute());
    }

    TEST_F(DriverTest, PrintVersion) {
        const char *argv[] = {"fly", "-version"};
        Driver driver(argv);
        driver.BuildCompilerInstance();
        EXPECT_TRUE(driver.Execute());
    }

} // anonymous namespace
