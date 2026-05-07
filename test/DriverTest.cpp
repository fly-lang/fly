//===--------------------------------------------------------------------------------------------------------------===//
// test/DriverTest.cpp - Driver tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <Driver/Driver.h>
#include "gtest/gtest.h"
#include <fstream>
#include <iostream>
#include <vector>

namespace {
    using namespace fly;

    const char* testFile  = "file1.fly";
    const char* testFile2 = "file2.fly";

    class DriverTest : public ::testing::Test {

    public:
        DriverTest() {}

        ~DriverTest() {
            llvm::outs().flush();
        }
    };

    bool createTestFile(const char* path) {
        std::fstream f;
        f.open(path, std::ios::out);
        if (f) f.close();
        return (bool) f;
    }

    void deleteTestFile(const char* path) {
        remove(path);
    }

    // ─── Basic option parsing via Driver ─────────────────────────────────────

    TEST_F(DriverTest, Options) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", "-v", testFile, "-o", "file.o"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        const FrontendOptions &FO = CI.getFrontendOptions();

        EXPECT_TRUE(FO.Verbose);
        ASSERT_EQ(FO.getInputFiles().size(), 1u);
        EXPECT_EQ(FO.getInputFiles()[0], testFile);
        EXPECT_EQ(FO.getOutputFile(), "file.o");

        deleteTestFile(testFile);
    }

    // ─── doExecute=false paths (no compilation needed) ───────────────────────

    TEST_F(DriverTest, PrintHelp) {
        const char *argv[] = {"fly", "-help"};
        Driver driver(argv);
        driver.BuildCompilerInstance();
        EXPECT_TRUE(driver.Execute());
    }

    TEST_F(DriverTest, PrintVersion) {
        const char *argv[] = {"fly", "-version"};
        Driver driver(argv);
        driver.BuildCompilerInstance();
        EXPECT_TRUE(driver.Execute());
    }

    TEST_F(DriverTest, PrintVersionShort) {
        const char *argv[] = {"fly", "-v"};
        Driver driver(argv);
        driver.BuildCompilerInstance();
        EXPECT_TRUE(driver.Execute());
    }

    TEST_F(DriverTest, UnknownOption) {
        const char *argv[] = {"fly", "--unknown-opt", testFile};
        Driver driver(argv);
        driver.BuildCompilerInstance();
        // doExecute=false after unknown option → Execute() returns true (no crash)
        EXPECT_TRUE(driver.Execute());
    }

    TEST_F(DriverTest, NoInputFiles) {
        const char *argv[] = {"fly"};
        Driver driver(argv);
        driver.BuildCompilerInstance();
        EXPECT_TRUE(driver.Execute());
    }

    // ─── Input / output option parsing ───────────────────────────────────────

    TEST_F(DriverTest, DriverOptions) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", "-v", testFile, "-o", "file.o"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        const FrontendOptions &FO = CI.getFrontendOptions();

        ASSERT_EQ(FO.getInputFiles().size(), 1u);
        EXPECT_EQ(FO.getInputFiles()[0], testFile);
        EXPECT_EQ(FO.getOutputFile(), "file.o");
        EXPECT_TRUE(FO.Verbose);

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, MultipleInputFiles) {
        ASSERT_TRUE(createTestFile(testFile));
        ASSERT_TRUE(createTestFile(testFile2));

        const char *argv[] = {"fly", testFile, testFile2};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        const FrontendOptions &FO = CI.getFrontendOptions();

        EXPECT_EQ(FO.getInputFiles().size(), 2u);
        EXPECT_EQ(FO.getInputFiles()[0], testFile);
        EXPECT_EQ(FO.getInputFiles()[1], testFile2);

        deleteTestFile(testFile);
        deleteTestFile(testFile2);
    }

    TEST_F(DriverTest, OutputLib) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "--lib", "-o", "out.a"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        const FrontendOptions &FO = CI.getFrontendOptions();

        EXPECT_EQ(FO.getOutputFile(), "out.a");
        EXPECT_TRUE(FO.CreateLibrary);
        EXPECT_TRUE(FO.CreateHeader);
        EXPECT_EQ(FO.BackendAction, BackendActionKind::Backend_EmitObj);

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, HeaderGenerator) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-header"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getFrontendOptions().CreateHeader);

        deleteTestFile(testFile);
    }

    // ─── Backend action options ───────────────────────────────────────────────

    TEST_F(DriverTest, EmitLL) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-emit-ll"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitLL);

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, EmitBC) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-emit-bc"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitBC);

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, EmitAS) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-emit-as"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitAssembly);

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, NoOutput) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-no-output"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitNothing);

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, DefaultBackendIsEmitObj) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitObj);

        deleteTestFile(testFile);
    }

    // ─── Output conflict detection ────────────────────────────────────────────

    TEST_F(DriverTest, OutputConflictWithEmitLL) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-emit-ll", "-o", "out.o"};
        Driver driver(argv);
        driver.BuildCompilerInstance();
        // -o is incompatible with -emit-ll → doExecute=false → Execute() returns true
        EXPECT_TRUE(driver.Execute());

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, OutputConflictWithEmitBC) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-emit-bc", "-o", "out.o"};
        Driver driver(argv);
        driver.BuildCompilerInstance();
        EXPECT_TRUE(driver.Execute());

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, OutputConflictWithEmitAS) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-emit-as", "-o", "out.o"};
        Driver driver(argv);
        driver.BuildCompilerInstance();
        EXPECT_TRUE(driver.Execute());

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, OutputConflictWithNoOutput) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-no-output", "-o", "out.o"};
        Driver driver(argv);
        driver.BuildCompilerInstance();
        EXPECT_TRUE(driver.Execute());

        deleteTestFile(testFile);
    }

    // ─── Stats / timers ───────────────────────────────────────────────────────

    TEST_F(DriverTest, ShowStats) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-print-stats"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getFrontendOptions().ShowStats);

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, ShowTimers) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-ftime-report"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getFrontendOptions().ShowTimers);

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, StatsFile) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-stats-file", "stats.json"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getFrontendOptions().StatsFile, "stats.json");

        deleteTestFile(testFile);
    }

    // ─── Target options ───────────────────────────────────────────────────────

    TEST_F(DriverTest, TargetTriple) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "--target", "x86_64-linux-gnu"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getTargetOptions()->Triple, "x86_64-linux-gnu");

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, DefaultTargetTriple) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_FALSE(CI.getTargetOptions()->Triple.empty());

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, TargetCPU) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "--target-cpu", "generic"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getTargetOptions()->CPU, "generic");

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, CodeModel) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-mcmodel", "small"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getTargetOptions()->CodeModel,   "small");
        EXPECT_EQ(CI.getCodeGenOptions().CodeModel,   "small");

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, DefaultCodeModel) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getTargetOptions()->CodeModel, "default");
        EXPECT_EQ(CI.getCodeGenOptions().CodeModel, "default");

        deleteTestFile(testFile);
    }

    // ─── CodeGen / thread model ───────────────────────────────────────────────

    TEST_F(DriverTest, ThreadModelPosix) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-mthread-model", "posix"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getCodeGenOptions().ThreadModel, "posix");

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, ThreadModelSingle) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-mthread-model", "single"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getCodeGenOptions().ThreadModel, "single");

        deleteTestFile(testFile);
    }

    TEST_F(DriverTest, DefaultThreadModel) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getCodeGenOptions().ThreadModel, "posix");

        deleteTestFile(testFile);
    }

    // ─── Diagnostics options ─────────────────────────────────────────────────

    TEST_F(DriverTest, NoWarning) {
        ASSERT_TRUE(createTestFile(testFile));

        const char *argv[] = {"fly", testFile, "-w"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getDiagnostics().getIgnoreAllWarnings());

        deleteTestFile(testFile);
    }

} // anonymous namespace
