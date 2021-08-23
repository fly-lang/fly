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
#include "llvm/Option/OptTable.h"
#include "llvm/Support/ManagedStatic.h"
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
        const char* Argv[] = {"fly", "-ll", "main.fly", "-o", "test"};
        int Argc = sizeof Argv / sizeof Argv[0];

        SmallVector<const char *, 256> Args(Argv, Argv + Argc);
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();
//        llvm::InitializeAllAsmParsers();

        testing::internal::CaptureStdout();

        Driver driver(Args);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        driver.Execute();

        std::string output = testing::internal::GetCapturedStdout();
        llvm::outs() << output;
    }

} // anonymous namespace
