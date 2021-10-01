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
#include <llvm/Support/InitLLVM.h>

namespace {
    using namespace fly;

    // The test fixture.
    class CmdTest : public ::testing::Test {

    public:
        CmdTest() {
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmPrinters();
        }

        void deleteFile(const char* testFile) {
            remove(testFile);
        }
    };

    TEST_F(CmdTest, ShowVersion) {
        char* Argv[] = {"fly", "--version-short", NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        TheDriver.Execute();
    }

    TEST_F(CmdTest, EmitNothing) {
        char* Argv[] = {"fly", "-no", NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        TheDriver.Execute();
    }

    TEST_F(CmdTest, EmitLL) {
        char* Argv[] = {"fly", "-debug", "-ll", "src/main.fly", NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        TheDriver.Execute();

        std::ifstream reader("main.fly.ll");
        ASSERT_TRUE(reader && "Error opening main.fly.ll");

        deleteFile("main.fly.ll");
    }

    TEST_F(CmdTest, EmitBC) {
        char* Argv[] = {"fly", "-bc", "src/main.fly", NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        TheDriver.Execute();

        std::ifstream reader("main.fly.bc");
        ASSERT_TRUE(reader && "Error opening main.fly.bc");
        deleteFile("main.fly.bc");
    }

    TEST_F(CmdTest, EmitAS) {
        char* Argv[] = {"fly", "-as", "src/main.fly", NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        TheDriver.Execute();

        std::ifstream reader("main.fly.as");
        ASSERT_TRUE(reader && "Error opening main.fly.as");
        deleteFile("main.fly.as");
    }

    TEST_F(CmdTest, EmitObj) {
        char* Argv[] = {"fly", "src/main.fly", NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        TheDriver.Execute();

        std::ifstream reader("main.fly.o");
        ASSERT_TRUE(reader && "Error opening main.fly.o");
        deleteFile("main.fly.o");
    }


} // anonymous namespace
