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
        deleteFile("main.fly.ll");
        deleteFile("utils.fly.ll");

        char* Argv[] = {"fly", "-ll", "src/main.fly", "src/utils.fly", NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        ASSERT_TRUE(TheDriver.Execute());

        std::ifstream main("main.fly.ll");
        ASSERT_TRUE(main && "Error opening main.fly.ll");

        std::ifstream utils("utils.fly.ll");
        ASSERT_TRUE(utils && "Error opening utils.fly.ll");
    }

    TEST_F(CmdTest, EmitBC) {
        deleteFile("main.fly.bc");
        deleteFile("utils.fly.bc");

        char* Argv[] = {"fly", "-bc", "src/main.fly",  "src/utils.fly", NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        TheDriver.Execute();

        std::ifstream main("main.fly.bc");
        ASSERT_TRUE(main && "Error opening main.fly.bc");

        std::ifstream utils("utils.fly.bc");
        ASSERT_TRUE(utils && "Error opening utils.fly.bc");
    }

    TEST_F(CmdTest, EmitAS) {
        deleteFile("main.fly.s");
        deleteFile("utils.fly.s");

        char* Argv[] = {"fly", "-as", "src/main.fly", "src/utils.fly", NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        TheDriver.Execute();

        std::ifstream main("main.fly.s");
        ASSERT_TRUE(main && "Error opening main.fly.s");

        std::ifstream utils("utils.fly.s");
        ASSERT_TRUE(utils && "Error opening utils.fly.s");
    }

    TEST_F(CmdTest, EmitObj) {
        deleteFile("main.fly.o");
        deleteFile("utils.fly.o");

        char* Argv[] = {"fly", "src/main.fly", "src/utils.fly", NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        TheDriver.Execute();

        std::ifstream main("main.fly.o");
        ASSERT_TRUE(main && "Error opening main.fly.o");

        std::ifstream utils("utils.fly.o");
        ASSERT_TRUE(utils && "Error opening utils.fly.o");
    }

    TEST_F(CmdTest, DISABLED_EmitOut) {
        deleteFile("main.fly.o");
        deleteFile("utils.fly.o");

        char* Argv[] = {"fly", "src/main.fly", "src/utils.fly", "-o", "out", NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        TheDriver.Execute();

        std::ifstream main("out");
        ASSERT_TRUE(main && "Error opening out");
    }


} // anonymous namespace
