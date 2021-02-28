//===----------------------------------------------------------------------===//
// test/DriverTest.cpp - Compiler tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#include <Driver/Driver.h>
#include <Driver/DriverOptions.h>
#include <llvm/Option/OptTable.h>
#include <llvm/Option/ArgList.h>
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

//    llvm::opt::InputArgList parseArgs(ArrayRef<const char *> ArgList) {
//        const llvm::opt::OptTable &table = fly::driver::getDriverOptTable();
//        unsigned MissingArgIndex, MissingArgCount;
//        unsigned IncludedFlagsBitmask;
//        unsigned ExcludedFlagsBitmask;
//        return table.ParseArgs(ArgList, MissingArgIndex, MissingArgCount,
//                               IncludedFlagsBitmask, ExcludedFlagsBitmask);
//    }

    // The test fixture.
    class DriverTest : public ::testing::Test {

    public:
        DriverTest() {}
    };

    TEST_F(DriverTest, DriverOptions) {
        const char *argv[] = {"fly", "-i", "file1.fly", "-o", "file.o"};
        Driver driver(5, argv);
        const llvm::opt::InputArgList &Args = driver.getArgList();
        // Check for working directory option before accessing any files
//        for (auto &Arg : Args) {
//            Arg->getOption();
//        }
//        EXPECT_TRUE(Args.hasArg(driver::options::OPT_input_file));
    }

} // anonymous namespace
