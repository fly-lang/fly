//===--------------------------------------------------------------------------------------------------------------===//
// test/CmdTest.cpp - Driver tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include <Driver/Driver.h>
#include <Driver/DriverOptions.h>
#include "llvm/Support/TargetSelect.h"
#include "Basic/Debug.h"
#include "gtest/gtest.h"
#include <fstream>

namespace {
    using namespace fly;

    // The test fixture.
    class CmdTest : public ::testing::Test {

    public:
        CmdTest() {}
    };

    TEST_F(CmdTest, LaunchMain) {
        char* Argv[] = {"fly", "-debug", "-ll", "src/main.fly", NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();

        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        TheDriver.Execute();

        std::ifstream reader("main.fly.ll");
        ASSERT_TRUE(reader && "Error opening main.fly.ll");
    }

    TEST_F(CmdTest, ShowVersion) {
        char* Argv[] = {"fly", "--version-short", NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();

        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        TheDriver.Execute();

    }

} // anonymous namespace
