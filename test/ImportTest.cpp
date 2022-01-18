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

    class LibTest : public ::testing::Test {

    public:
        std::string mylib = FLY_TEST_SRC_PATH + "/mylib.fly";
        std::string import_mylib = FLY_TEST_SRC_PATH + "/import_mylib.fly";
        std::string import_mylib_alias = FLY_TEST_SRC_PATH + "/import_mylib_alias.fly";
        std::string import_mylib_flat = FLY_TEST_SRC_PATH + "/import_mylib_flat.fly";

        LibTest() {
            llvm::InitializeAllTargetInfos();
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            InitializeAllAsmParsers();
            llvm::InitializeAllAsmPrinters();
        }

        ~LibTest() {
            llvm::outs().flush();
        }

        void deleteFile(const char* testFile) {
            remove(testFile);
        }
    };

    TEST_F(LibTest, MyLib) {
        deleteFile("mylib.fly.o");
        deleteFile("import_mylib.fly.o");

        const char* Argv[] = {"fly", "-debug", LibTest::mylib.c_str(), LibTest::import_mylib.c_str(), NULL};
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

    TEST_F(LibTest, MyLibAlias) {
        deleteFile("mylib.fly.o");
        deleteFile("import_mylib_alias.fly.o");

        const char* Argv[] = {"fly",  LibTest::mylib.c_str(), LibTest::import_mylib_alias.c_str(), NULL};
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

} // anonymous namespace
