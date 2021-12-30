//===--------------------------------------------------------------------------------------------------------------===//
// test/CmdTest.cpp - Driver tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "TestConfig.h"
#include <Driver/Driver.h>
#include <Driver/DriverOptions.h>
#include "llvm/Support/TargetSelect.h"
#include "Basic/Debug.h"
#include "gtest/gtest.h"
#include <fstream>
#include <llvm/Support/InitLLVM.h>

namespace {
    using namespace fly;

    class CmdTest : public ::testing::Test {

    public:
        std::string mainfly = FLY_TEST_SRC_PATH + "/main.fly";
        std::string utilsfly = FLY_TEST_SRC_PATH + "/utils.fly";

        CmdTest() {
            llvm::InitializeAllTargetInfos();
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            InitializeAllAsmParsers();
            llvm::InitializeAllAsmPrinters();
        }

        ~CmdTest() {
            llvm::outs().flush();
        }

        void deleteFile(const char* testFile) {
            remove(testFile);
        }
    };

    TEST_F(CmdTest, EmitOut) {
        const char* Argv[] = {"fly", "-debug", "test_lib.fly", "-o", "out", NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        ASSERT_TRUE(TheDriver.Execute());

        const llvm::Triple &T = TargetInfo::CreateTargetInfo(CI.getDiagnostics(), CI.getTargetOptions())->getTriple();
        if (T.isWindowsMSVCEnvironment()) {
            std::ifstream out("out.exe");
            ASSERT_TRUE(out && "Error opening out");
        } else {
            std::ifstream out("out");
            ASSERT_TRUE(out && "Error opening out");
        }
    }


} // anonymous namespace
