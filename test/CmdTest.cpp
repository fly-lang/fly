//===--------------------------------------------------------------------------------------------------------------===//
// test/CmdTest.cpp - Driver tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "ConfigTest.h"
#include <Driver/Driver.h>
#include <Driver/DriverOptions.h>
#include <llvm/Support/ManagedStatic.h>
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/InitLLVM.h"
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

    TEST_F(CmdTest, LaunchAsMain) {
        std::string Source = std::string(FLY_TEST_SRC_PATH).append("/main.fly");
        char* Args[] = {"fly", "-debug", "-ll", const_cast<char *>(Source.c_str())};
        char** Argv = (char**)Args;
        int Argc = 4;
        llvm::outs() << "Testing " << Source << "\n";
        llvm::outs() << "FLY_TEST_BUILD_PATH=" << FLY_TEST_BUILD_PATH << "\n";
        llvm::InitLLVM X(Argc, Argv);

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();

        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        TheDriver.Execute();

        std::ifstream reader(std::string(FLY_TEST_BUILD_PATH).append("/main.fly.ll")) ;
        ASSERT_TRUE(reader && "Error opening main.fly");

        llvm::llvm_shutdown();
    }

} // anonymous namespace
