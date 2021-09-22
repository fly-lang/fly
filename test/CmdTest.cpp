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
#include <fstream>
#include <Basic/Stack.h>
#include <llvm/Support/ManagedStatic.h>
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/InitLLVM.h"
#include "gtest/gtest.h"

namespace {
    using namespace fly;

    // The test fixture.
    class CmdTest : public ::testing::Test {

    public:
        CmdTest() {}
    };

    TEST_F(CmdTest, LaunchAsMain) {
        char* Args[] = {"fly", "-debug", "-ll", "src/main.fly"};
        char** Argv = (char**)Args;
        int Argc = 4;

        llvm::InitLLVM X(Argc, Argv);

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();

        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        TheDriver.Execute();

        std::ifstream reader("main.fly.ll") ;
        ASSERT_TRUE(reader && "Error opening main.fly");

        llvm::llvm_shutdown();
    }

} // anonymous namespace
