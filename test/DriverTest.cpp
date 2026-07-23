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
#include <iostream>
#include <vector>

namespace {
    using namespace fly;

    // The CLI has no positional arguments: fly compiles a source DIRECTORY
    // (--src-dir, default: current directory). These tests exercise option
    // parsing only — BuildCompilerInstance() resolves the options without
    // running discovery, so no source files are needed on disk.

    class DriverTest : public ::testing::Test {

    public:
        DriverTest() {}

        ~DriverTest() {
            llvm::outs().flush();
        }
    };

    // ─── Basic option parsing via Driver ─────────────────────────────────────

    TEST_F(DriverTest, Options) {
        const char *argv[] = {"fly", "-v", "-o", "file.o"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        const FrontendOptions &FO = CI.getFrontendOptions();

        EXPECT_TRUE(FO.Verbose);
        EXPECT_TRUE(FO.getInputFiles().empty());
        EXPECT_TRUE(FO.DiscoverInputs);
        EXPECT_EQ(FO.getOutputFile(), "file.o");
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

    TEST_F(DriverTest, UnknownOption) {
        const char *argv[] = {"fly", "--unknown-opt"};
        Driver driver(argv);
        driver.BuildCompilerInstance();
        // A CLI error is a FAILURE: Execute() is a no-op returning false → exit 1.
        EXPECT_FALSE(driver.Execute());
    }

    // Positional arguments are rejected: sources are never named on the command
    // line, they are discovered from the source directory.
    TEST_F(DriverTest, PositionalArgumentIsRejected) {
        const char *argv[] = {"fly", "file1.fly"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        // A CLI error is a FAILURE (exit 1); nothing was compiled.
        EXPECT_FALSE(driver.Execute());
        EXPECT_TRUE(CI.getFrontendOptions().getInputFiles().empty());
    }

    // With no arguments the driver enters directory mode: inputs are discovered
    // by the Frontend from the current directory at Execute() time.
    TEST_F(DriverTest, DirectoryModeIsDefault) {
        const char *argv[] = {"fly"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getFrontendOptions().DiscoverInputs);
        EXPECT_TRUE(CI.getFrontendOptions().getInputFiles().empty());
    }

    // ─── Suite / test options ────────────────────────────────────────────────

    // --suite takes the suite NAME as its optional value: with no positional
    // arguments on the CLI there is nothing it can swallow by mistake.
    TEST_F(DriverTest, SuiteWithNameSpaceForm) {
        const char *argv[] = {"fly", "--suite", "DemoSuite"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getCodeGenOptions().TestMode);
        EXPECT_EQ(CI.getCodeGenOptions().SuiteName, "DemoSuite");
    }

    TEST_F(DriverTest, SuiteWithNameEqForm) {
        const char *argv[] = {"fly", "--suite=DemoSuite"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getCodeGenOptions().TestMode);
        EXPECT_EQ(CI.getCodeGenOptions().SuiteName, "DemoSuite");
    }

    // Bare --suite: every suite discovered under the source root runs.
    TEST_F(DriverTest, SuiteWithoutName) {
        const char *argv[] = {"fly", "--suite"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getCodeGenOptions().TestMode);
        EXPECT_TRUE(CI.getCodeGenOptions().SuiteName.empty());
    }

    // --suite Name --test Method: the method filter rides along.
    TEST_F(DriverTest, SuiteWithTestMethodFilter) {
        const char *argv[] = {"fly", "--suite", "DemoSuite", "--test", "sum"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getCodeGenOptions().TestMode);
        EXPECT_EQ(CI.getCodeGenOptions().SuiteName, "DemoSuite");
        EXPECT_EQ(CI.getCodeGenOptions().TestFilter, "sum");
    }

    // Bare --test: compile-only test mode (test {} blocks enabled, no run).
    TEST_F(DriverTest, BareTestMode) {
        const char *argv[] = {"fly", "--test"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getCodeGenOptions().TestMode);
        EXPECT_TRUE(CI.getCodeGenOptions().SuiteName.empty());
    }

    // ─── Input / output option parsing ───────────────────────────────────────

    TEST_F(DriverTest, OutputLib) {
        const char *argv[] = {"fly", "--lib", "-o", "out.a"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        const FrontendOptions &FO = CI.getFrontendOptions();

        EXPECT_EQ(FO.getOutputFile(), "out.a");
        EXPECT_TRUE(FO.CreateLibrary);
        EXPECT_TRUE(FO.CreateHeader);
        EXPECT_EQ(FO.BackendAction, BackendActionKind::Backend_EmitObj);
    }

    TEST_F(DriverTest, HeaderGenerator) {
        const char *argv[] = {"fly", "-header"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getFrontendOptions().CreateHeader);
    }

    // ─── Backend action options ───────────────────────────────────────────────

    TEST_F(DriverTest, EmitLL) {
        const char *argv[] = {"fly", "-emit-ll"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitLL);
    }

    TEST_F(DriverTest, EmitBC) {
        const char *argv[] = {"fly", "-emit-bc"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitBC);
    }

    TEST_F(DriverTest, EmitAS) {
        const char *argv[] = {"fly", "-emit-as"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitAssembly);
    }

    TEST_F(DriverTest, NoOutput) {
        const char *argv[] = {"fly", "-no-output"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitNothing);
    }

    TEST_F(DriverTest, DefaultBackendIsEmitObj) {
        const char *argv[] = {"fly"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitObj);
    }

    // ─── Explicit -o with an emit action ──────────────────────────────────────
    // -o is VALID together with --emit-ll/-bc/-as: it names the single emitted
    // artifact, and a non-empty output file is what makes the Frontend lower every
    // input into one module so cross-file references resolve.

    TEST_F(DriverTest, ExplicitOutputWithEmitLL) {
        const char *argv[] = {"fly", "-emit-ll", "-o", "out.ll"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitLL);
        EXPECT_EQ(CI.getFrontendOptions().getOutputFile(), "out.ll");
    }

    TEST_F(DriverTest, ExplicitOutputWithEmitBC) {
        const char *argv[] = {"fly", "-emit-bc", "-o", "out.bc"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitBC);
        EXPECT_EQ(CI.getFrontendOptions().getOutputFile(), "out.bc");
    }

    TEST_F(DriverTest, ExplicitOutputWithEmitAS) {
        const char *argv[] = {"fly", "-emit-as", "-o", "out.s"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitAssembly);
        EXPECT_EQ(CI.getFrontendOptions().getOutputFile(), "out.s");
    }

    // ─── The three option axes ────────────────────────────────────────────────
    // FORMAT (--emit-*) / STAGE (-c, --no-output) / SHAPE (--lib, --lib-dyn) are
    // independent: none of them may quietly rewrite another. LinkStep records the
    // stage decision once so nothing downstream has to re-derive it.

    TEST_F(DriverTest, DefaultStageLinks) {
        const char *argv[] = {"fly"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getFrontendOptions().LinkStep);
    }

    TEST_F(DriverTest, CompileOnlyStopsBeforeLink) {
        const char *argv[] = {"fly", "-c"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        // -c changes only the STAGE: the format stays the default object file.
        EXPECT_EQ(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitObj);
        EXPECT_FALSE(CI.getFrontendOptions().LinkStep);
    }

    TEST_F(DriverTest, NonObjectFormatStopsBeforeLink) {
        const char *argv[] = {"fly", "-emit-ll", "-o", "out.ll"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        // Even with -o naming a real artifact, a .ll must never reach the linker.
        EXPECT_FALSE(CI.getFrontendOptions().LinkStep);
    }

    TEST_F(DriverTest, NoOutputStopsBeforeLink) {
        const char *argv[] = {"fly", "-no-output"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_FALSE(CI.getFrontendOptions().LinkStep);
    }

    TEST_F(DriverTest, LibStaticAlias) {
        const char *argv[] = {"fly", "--lib-static", "-o", "out"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getFrontendOptions().CreateLibrary);
        EXPECT_FALSE(CI.getFrontendOptions().CreateSharedLib);
        EXPECT_EQ(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitObj);
        EXPECT_TRUE(CI.getFrontendOptions().LinkStep);
    }

    TEST_F(DriverTest, LibDynAlias) {
        const char *argv[] = {"fly", "--lib-dyn", "-o", "out"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getFrontendOptions().CreateSharedLib);
        EXPECT_FALSE(CI.getFrontendOptions().CreateLibrary);
        EXPECT_TRUE(CI.getFrontendOptions().LinkStep);
    }

    TEST_F(DriverTest, LibDynamicLongAlias) {
        const char *argv[] = {"fly", "--lib-dynamic", "-o", "out"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getFrontendOptions().CreateSharedLib);
    }

    // --shared was an old spelling and is gone: the dynamic library is
    // requested with --lib-dyn / --lib-dynamic only. It must be rejected as an
    // unknown option (a CLI error → Execute() false, exit 1).
    TEST_F(DriverTest, SharedFlagIsGone) {
        const char *argv[] = {"fly", "--shared", "-o", "out"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_FALSE(driver.Execute());
        EXPECT_FALSE(CI.getFrontendOptions().CreateSharedLib);
    }

    // ─── Axis conflicts are diagnosed, never resolved by precedence ───────────
    // Each of these used to be a silent override: --lib rewrote the format, so a
    // request for IR came back as an object file without a word.

    TEST_F(DriverTest, LibWithEmitLLIsRejected) {
        const char *argv[] = {"fly", "--lib", "-emit-ll", "-o", "out.ll"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_FALSE(driver.Execute());  // rejected cleanly: a CLI error → exit 1
        // The driver bailed out before applying either axis.
        EXPECT_NE(CI.getFrontendOptions().BackendAction, BackendActionKind::Backend_EmitLL);
        EXPECT_FALSE(CI.getFrontendOptions().CreateLibrary);
    }

    TEST_F(DriverTest, LibWithCompileOnlyIsRejected) {
        const char *argv[] = {"fly", "--lib", "-c", "-o", "out"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_FALSE(driver.Execute());
        EXPECT_FALSE(CI.getFrontendOptions().CreateLibrary);
    }

    TEST_F(DriverTest, LibWithLibDynIsRejected) {
        const char *argv[] = {"fly", "--lib", "--lib-dyn", "-o", "out"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_FALSE(driver.Execute());
        EXPECT_FALSE(CI.getFrontendOptions().CreateLibrary);
        EXPECT_FALSE(CI.getFrontendOptions().CreateSharedLib);
    }

    // ─── Output conflict detection ────────────────────────────────────────────
    // --no-output emits nothing, so naming an output makes no sense: rejected as
    // a CLI error (Execute() false → exit 1).

    TEST_F(DriverTest, OutputConflictWithNoOutput) {
        const char *argv[] = {"fly", "-no-output", "-o", "out.o"};
        Driver driver(argv);
        driver.BuildCompilerInstance();
        EXPECT_FALSE(driver.Execute());
    }

    // ─── Stats / timers ───────────────────────────────────────────────────────

    TEST_F(DriverTest, ShowStats) {
        const char *argv[] = {"fly", "-print-stats"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getFrontendOptions().ShowStats);
    }

    TEST_F(DriverTest, ShowTimers) {
        const char *argv[] = {"fly", "-ftime-report"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getFrontendOptions().ShowTimers);
    }

    TEST_F(DriverTest, StatsFile) {
        const char *argv[] = {"fly", "-stats-file", "stats.json"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getFrontendOptions().StatsFile, "stats.json");
    }

    // ─── Target options ───────────────────────────────────────────────────────

    TEST_F(DriverTest, TargetTriple) {
        const char *argv[] = {"fly", "--target", "x86_64-linux-gnu"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getTargetOptions()->Triple, "x86_64-linux-gnu");
    }

    TEST_F(DriverTest, DefaultTargetTriple) {
        const char *argv[] = {"fly"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_FALSE(CI.getTargetOptions()->Triple.empty());
    }

    TEST_F(DriverTest, TargetCPU) {
        const char *argv[] = {"fly", "--target-cpu", "generic"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getTargetOptions()->CPU, "generic");
    }

    TEST_F(DriverTest, CodeModel) {
        const char *argv[] = {"fly", "-mcmodel", "small"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getTargetOptions()->CodeModel,   "small");
        EXPECT_EQ(CI.getCodeGenOptions().CodeModel,   "small");
    }

    TEST_F(DriverTest, DefaultCodeModel) {
        const char *argv[] = {"fly"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getTargetOptions()->CodeModel, "default");
        EXPECT_EQ(CI.getCodeGenOptions().CodeModel, "default");
    }

    // ─── CodeGen / thread model ───────────────────────────────────────────────

    TEST_F(DriverTest, ThreadModelPosix) {
        const char *argv[] = {"fly", "-mthread-model", "posix"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getCodeGenOptions().ThreadModel, "posix");
    }

    TEST_F(DriverTest, ThreadModelSingle) {
        const char *argv[] = {"fly", "-mthread-model", "single"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getCodeGenOptions().ThreadModel, "single");
    }

    TEST_F(DriverTest, DefaultThreadModel) {
        const char *argv[] = {"fly"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_EQ(CI.getCodeGenOptions().ThreadModel, "posix");
    }

    // ─── Diagnostics options ─────────────────────────────────────────────────

    TEST_F(DriverTest, NoWarning) {
        const char *argv[] = {"fly", "-w"};
        Driver driver(argv);
        CompilerInstance &CI = driver.BuildCompilerInstance();
        EXPECT_TRUE(CI.getDiagnostics().getIgnoreAllWarnings());
    }

} // anonymous namespace
