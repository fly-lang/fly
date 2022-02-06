//===--------------------------------------------------------------------------------------------------------------===//
// test/LibTest.cpp - Driver tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "TestConfig.h"
#include "Driver/Driver.h"
#include "Driver/DriverOptions.h"
#include "llvm/Support/TargetSelect.h"
#include "Basic/Debug.h"
#include "gtest/gtest.h"
#include <fstream>

namespace {
    using namespace fly;

    class ImportTest : public ::testing::Test {

    public:
        std::string mylib = FLY_TEST_SRC_PATH + "/mylib.fly";
        std::string import_mylib = FLY_TEST_SRC_PATH + "/import_mylib.fly";
        std::string import_mylib_alias = FLY_TEST_SRC_PATH + "/import_mylib_alias.fly";
        std::string import_mylib_external = FLY_TEST_SRC_PATH + "/import_mylib_external.fly";

        ImportTest() {
            llvm::InitializeAllTargetInfos();
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            InitializeAllAsmParsers();
            llvm::InitializeAllAsmPrinters();
        }

        ~ImportTest() {
            llvm::outs().flush();
        }

        void deleteFile(const char* testFile) {
            remove(testFile);
        }
    };

    TEST_F(ImportTest, MyLib) {
        deleteFile("mylib.fly.o");
        deleteFile("import_mylib.fly.o");

        const char* Argv[] = {"fly", "-debug", ImportTest::mylib.c_str(), ImportTest::import_mylib.c_str(), NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        ASSERT_TRUE(TheDriver.Execute());

        std::ifstream main("mylib.fly.o");
        ASSERT_TRUE(main && "Error opening mylib.fly.o");

        std::ifstream utils("import_mylib.fly.o");
        ASSERT_TRUE(utils && "Error opening import_mylib.fly.o");
    }

    TEST_F(ImportTest, MyLibAlias) {
        deleteFile("mylib.fly.o");
        deleteFile("import_mylib_alias.fly.o");

        const char* Argv[] = {"fly",  ImportTest::mylib.c_str(), ImportTest::import_mylib_alias.c_str(), NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        ASSERT_TRUE(TheDriver.Execute());

        std::ifstream main("mylib.fly.o");
        ASSERT_TRUE(main && "Error opening mylib.fly.o");

        std::ifstream utils("import_mylib_alias.fly.o");
        ASSERT_TRUE(utils && "Error opening import_mylib_alias.fly.o");
    }

    TEST_F(ImportTest, Header) {
        deleteFile("mylib.fly.o");
        deleteFile("mylib.fly.h");

        const char* Argv[] = {"fly", "-debug", "-H", ImportTest::mylib.c_str(), NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;

        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        ASSERT_TRUE(TheDriver.Execute());

        std::ifstream main("mylib.fly.o");
        ASSERT_TRUE(main && "Error opening mylib.fly.o");

        std::ifstream utils("mylib.fly.h");
        ASSERT_TRUE(utils && "Error opening mylib.fly.h");
    }

    TEST_F(ImportTest, MyLibExternal) {
        deleteFile("mylib.lib");
        deleteFile("import_mylib_external.fly.o");

        const char* Argv[] = {"fly", "-debug", ImportTest::mylib.c_str(), "-lib", "mylib", NULL};
        int Argc = sizeof(Argv) / sizeof(char*) - 1;
        SmallVector<const char *, 256> ArgList(Argv, Argv + Argc);
        Driver TheDriver(ArgList);
        CompilerInstance &CI = TheDriver.BuildCompilerInstance();
        ASSERT_TRUE(TheDriver.Execute());

        const char* Argv2[] = {"fly", "-debug", "mylib.lib", ImportTest::import_mylib_external.c_str(), NULL};
        int Argc2 = sizeof(Argv2) / sizeof(char*) - 1;
        SmallVector<const char *, 256> ArgList2(Argv2, Argv2 + Argc2);
        Driver TheDriver2(ArgList2);
        CompilerInstance &CI2 = TheDriver2.BuildCompilerInstance();
        ASSERT_TRUE(TheDriver2.Execute());

        std::ifstream main("mylib.lib");
        ASSERT_TRUE(main && "Error opening mylib.lib");

        std::ifstream utils("import_mylib_external.fly.o");
        ASSERT_TRUE(utils && "Error opening import_mylib_external.fly.o");
    }

} // anonymous namespace
