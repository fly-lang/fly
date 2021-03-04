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
#include <vector>
#include <iostream>
#include <fstream>
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
    using namespace fly;
    using namespace fly::driver;
    using testing::ElementsAre;

    static const char* testFile = "file1.fly";

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

    // The test fixture.
    class DriverTest : public ::testing::Test {

    public:
        DriverTest() {}
    };

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
        createTestFile();

        const char *argv[] = {"fly", "-v", testFile, "-o", "file.o"};
        Driver driver(5, argv);

        const FrontendOptions &fopts = driver.getInvocation()->getFrontendOptions();
        EXPECT_EQ(fopts.getInputFiles()[0].getFile(), testFile);
        EXPECT_EQ(fopts.getOutputFile().getFile(), "file.o");
        EXPECT_TRUE(fopts.isVerbose());

        deleteTestFile();
    }

    TEST_F(DriverTest, PrintHelp) {
        const char *argv[] = {"fly", "-help"};
        Driver driver(2, argv);
        driver.execute();
    }

    TEST_F(DriverTest, PrintVersion) {
        const char *argv[] = {"fly", "-version"};
        Driver driver(2, argv);
        driver.execute();
    }

} // anonymous namespace
