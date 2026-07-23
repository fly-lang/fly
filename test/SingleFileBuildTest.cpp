//===--------------------------------------------------------------------------------------------------------------===//
// test/SingleFileBuildTest.cpp - directory build: input discovery + auto-detect
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// Covers the directory-based build feature of the driver/frontend:
//   * input discovery from the source root (--src-dir, default: cwd):
//       - executable: the single file declaring main() (0 or >1 → error);
//       - test mode:  the files declaring suites (--suite Name selects one);
//       - --lib/--lib-dyn and non-linking stages: the whole directory;
//   * output-type auto-detection from the entry AST with auto-naming (entry
//     file stem, suite name, or source-root name), gated on "no -o + link";
//   * --lib/--lib-dyn forcing a library even when a main() is present;
//   * import-based dependency pulling from the same source root.
//
// Gate assertions read FrontendOptions right after BuildCompilerInstance() (set
// in Driver::BuildOptions, before any codegen), so they need no link step.
// Behavioural cases run a full Execute() and inspect the resolved options /
// produced artifacts.
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Driver/Driver.h"
#include "Frontend/CompilerInstance.h"
#include "Frontend/FrontendOptions.h"
#include "Basic/CodeGenOptions.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "gtest/gtest.h"

#include <fstream>

extern bool DebugLog;

namespace {
    using namespace fly;

#ifndef FLY_LIB_FLY_DIR
#define FLY_LIB_FLY_DIR "."
#endif

    class SingleFileBuildTest : public ::testing::Test {
    public:
        SingleFileBuildTest() {
            DebugLog = false;
            llvm::InitializeAllTargetInfos();
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmParsers();
            llvm::InitializeAllAsmPrinters();
        }

        // Write Src to Path, registering it for cleanup.
        void writeFile(const std::string &Path, const std::string &Src) {
            std::ofstream f(Path);
            f << Src;
            Cleanup.push_back(Path);
        }

        // Create a source directory for one test, registering it for cleanup.
        void makeDir(const std::string &Dir) {
            llvm::sys::fs::create_directory(Dir);
            CleanupDirs.push_back(Dir);
        }

        // Mark an expected output artifact for cleanup (don't assume it exists).
        void track(const std::string &Path) { Cleanup.push_back(Path); }

        static bool exists(const std::string &Path) {
            return llvm::sys::fs::exists(Path);
        }

        // Platform artifact suffixes the ToolChain appends to auto-named outputs:
        // executables get ".exe" and static libraries ".lib" on Windows (MSVC),
        // ".a" / no suffix elsewhere. Tests must check the platform-correct name.
#ifdef _WIN32
        static constexpr const char *ExeExt = ".exe";
        static constexpr const char *LibExt = ".lib";
#else
        static constexpr const char *ExeExt = "";
        static constexpr const char *LibExt = ".a";
#endif
        static std::string exeName(const std::string &Stem) { return Stem + ExeExt; }
        static std::string libName(const std::string &Stem) { return Stem + LibExt; }

        ~SingleFileBuildTest() override {
            for (const auto &P : Cleanup)
                llvm::sys::fs::remove(P);
            for (const auto &D : CleanupDirs)
                llvm::sys::fs::remove_directories(D); // recursive: dir may hold artifacts
            llvm::outs().flush();
        }

        std::vector<std::string> Cleanup;
        std::vector<std::string> CleanupDirs;
    };

    // ── Gate tests (no link; assert the AutoDetectOutput flag) ──────────────────

    // No -o on a linking build → auto-detect is enabled.
    TEST_F(SingleFileBuildTest, AutoDetectEnabledByDefault) {
        const char *argv[] = {"fly"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        EXPECT_TRUE(CI.getFrontendOptions().AutoDetectOutput);
    }

    // Explicit -o means the caller already owns the output: auto-detect off.
    TEST_F(SingleFileBuildTest, ExplicitOutputDisablesAutoDetect) {
        const char *argv[] = {"fly", "-o", "sfb_gate_out"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        EXPECT_FALSE(CI.getFrontendOptions().AutoDetectOutput);
    }

    // Emit backends produce intermediates and must never trigger a link step.
    TEST_F(SingleFileBuildTest, EmitLlDisablesAutoDetect) {
        const char *argv[] = {"fly", "--emit-ll"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        EXPECT_FALSE(CI.getFrontendOptions().AutoDetectOutput);
    }

    TEST_F(SingleFileBuildTest, NoOutputDisablesAutoDetect) {
        const char *argv[] = {"fly", "--no-output"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        EXPECT_FALSE(CI.getFrontendOptions().AutoDetectOutput);
    }

    // ── Input discovery (full Execute) ──────────────────────────────────────────

    // An executable build needs exactly one main(): none under the root → error.
    TEST_F(SingleFileBuildTest, NoMainIsAnError) {
        const std::string dir = "sfb_nomain_src";
        makeDir(dir);
        writeFile(dir + "/sfb_lib.fly", "namespace demo\npublic int answer() { out = 42 }\n");
        const char *argv[] = {"fly", "--src-dir", "sfb_nomain_src"};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_FALSE(drv.Execute());
    }

    // Two files declaring main() under the same root → error, nothing built.
    TEST_F(SingleFileBuildTest, MultipleMainsIsAnError) {
        const std::string dir = "sfb_twomains_src";
        makeDir(dir);
        writeFile(dir + "/a.fly", "namespace demo\nvoid main() {}\n");
        writeFile(dir + "/b.fly", "namespace demo2\nvoid main() {}\n");
        track(exeName("a"));
        track(exeName("b"));
        const char *argv[] = {"fly", "--src-dir", "sfb_twomains_src"};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_FALSE(drv.Execute());
        EXPECT_FALSE(exists(exeName("a")));
        EXPECT_FALSE(exists(exeName("b")));
    }

    // void main() and no library flag → executable, auto-named from the file
    // declaring main(), not test mode.
    TEST_F(SingleFileBuildTest, AutoDetectExecutableFromMain) {
        const std::string dir = "sfb_exe_src";
        makeDir(dir);
        writeFile(dir + "/sfb_exe.fly", "namespace demo\nvoid main() {}\n");
        track(exeName("sfb_exe"));
        track("sfb_exe.fly.o"); // intermediate object (kept on the exe/test path)
        const char *argv[] = {"fly", "--src-dir", "sfb_exe_src"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        EXPECT_FALSE(CI.getFrontendOptions().CreateLibrary);
        EXPECT_FALSE(CI.getCodeGenOptions().TestMode);
        EXPECT_EQ(CI.getFrontendOptions().getOutputFile(), "sfb_exe");
        EXPECT_TRUE(exists(exeName("sfb_exe")));
    }

    // --lib compiles the whole directory into an archive named after the source
    // root, even when a main() exists (no executable produced).
    TEST_F(SingleFileBuildTest, LibFlagOverridesMain) {
        const std::string dir = "sfb_ovr";
        makeDir(dir);
        writeFile(dir + "/sfb_ovr.fly", "namespace demo\nvoid main() {}\npublic int v() { out = 1 }\n");
        track(libName("sfb_ovr"));
        track("sfb_ovr.fly.h");
        const char *argv[] = {"fly", "--lib", "--src-dir", "sfb_ovr"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        EXPECT_TRUE(CI.getFrontendOptions().CreateLibrary);
        // Auto-named after the source-root directory.
        EXPECT_EQ(CI.getFrontendOptions().getOutputFile(), "sfb_ovr");
        EXPECT_TRUE(exists(libName("sfb_ovr")));
        // Not linked as an executable. On Linux the executable name has no
        // extension, so it collides with the SOURCE DIRECTORY's name — require
        // a regular file, not mere path existence.
        EXPECT_FALSE(llvm::sys::fs::is_regular_file(exeName("sfb_ovr")));
    }

    // --test discovers the suite (no main() needed): test mode on, executable
    // auto-named after the SUITE, not after the file.
    TEST_F(SingleFileBuildTest, TestModeDiscoversSuite) {
        const std::string dir = "sfb_suite_src";
        makeDir(dir);
        writeFile(dir + "/sfb_suite.fly",
                  "import fly.assert\n\n"
                  "suite SfbSuite {\n"
                  "    void answerTest() {\n"
                  "        case \"trivial\": {\n"
                  "            assert.assertTrue(1 > 0, 1)\n"
                  "        }\n"
                  "    }\n"
                  "}\n");
        track(exeName("SfbSuite"));
        track("sfb_suite.fly.o");
        const char *argv[] = {"fly", "--test", "--src-dir", "sfb_suite_src",
                              "-L", FLY_LIB_FLY_DIR};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        EXPECT_TRUE(CI.getCodeGenOptions().TestMode);
        EXPECT_FALSE(CI.getFrontendOptions().CreateLibrary);
        EXPECT_EQ(CI.getFrontendOptions().getOutputFile(), "SfbSuite");
        EXPECT_TRUE(exists(exeName("SfbSuite")));
    }

    // main() + --test and no suite anywhere → test executable from main().
    TEST_F(SingleFileBuildTest, MainPlusTestFlagIsTestMode) {
        const std::string dir = "sfb_maintest_src";
        makeDir(dir);
        writeFile(dir + "/sfb_maintest.fly", "namespace demo\nvoid main() {}\n");
        track(exeName("sfb_maintest"));
        track("sfb_maintest.fly.o");
        const char *argv[] = {"fly", "--test", "--src-dir", "sfb_maintest_src",
                              "-L", FLY_LIB_FLY_DIR};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        EXPECT_TRUE(CI.getCodeGenOptions().TestMode);
        EXPECT_FALSE(CI.getFrontendOptions().CreateLibrary);
        EXPECT_EQ(CI.getFrontendOptions().getOutputFile(), "sfb_maintest");
    }

    // --suite Name: the named suite is discovered from the source root, built and
    // RUN — fly's success includes the suite binary exiting 0.
    TEST_F(SingleFileBuildTest, SuiteByNameIsDiscoveredAndRun) {
        const std::string dir = "sfb_named_src";
        makeDir(dir);
        writeFile(dir + "/first.fly",
                  "import fly.assert\n\n"
                  "suite SfbFirst {\n"
                  "    void okTest() {\n"
                  "        case \"ok\": {\n"
                  "            assert.assertTrue(1 > 0, 1)\n"
                  "        }\n"
                  "    }\n"
                  "}\n");
        writeFile(dir + "/second.fly",
                  "import fly.assert\n\n"
                  "suite SfbSecond {\n"
                  "    void okTest() {\n"
                  "        case \"ok\": {\n"
                  "            assert.assertTrue(2 > 1, 1)\n"
                  "        }\n"
                  "    }\n"
                  "}\n");
        track(exeName("SfbSecond"));
        track("second.fly.o");
        const char *argv[] = {"fly", "--suite", "SfbSecond",
                              "--src-dir", "sfb_named_src", "-L", FLY_LIB_FLY_DIR};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        EXPECT_EQ(drv.getRunExitCode(), 0);
        // Only the file declaring SfbSecond was compiled, named after the suite.
        EXPECT_EQ(CI.getFrontendOptions().getOutputFile(), "SfbSecond");
        EXPECT_TRUE(exists(exeName("SfbSecond")));
        EXPECT_FALSE(exists(exeName("SfbFirst")));
    }

    // --suite with a name no file declares → error.
    TEST_F(SingleFileBuildTest, SuiteNotFoundIsAnError) {
        const std::string dir = "sfb_nosuite_src";
        makeDir(dir);
        writeFile(dir + "/only.fly", "namespace demo\nvoid main() {}\n");
        const char *argv[] = {"fly", "--suite", "Missing",
                              "--src-dir", "sfb_nosuite_src", "-L", FLY_LIB_FLY_DIR};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_FALSE(drv.Execute());
    }

    // ── Import dependency graph (--src-dir) ─────────────────────────────────────

    // Helper: lay down a tiny two-file project where the entry imports a sibling
    // namespace via a wildcard import and calls one of its functions.
    static void writeDepProject(SingleFileBuildTest &T, const std::string &dir) {
        llvm::sys::fs::create_directory(dir);
        T.CleanupDirs.push_back(dir);
        T.writeFile(dir + "/util.fly",
                    "namespace dep.util\n\npublic int helper() { out = 41 }\n");
        T.writeFile(dir + "/main.fly",
                    "namespace dep\n\nimport dep.util.*\n\n"
                    "public int compute() { out = helper() + 1 }\n");
    }

    // --lib compiles the whole root (entry + sibling) into one archive at -o.
    TEST_F(SingleFileBuildTest, SrcDirResolvesImportedDependency) {
        const std::string dir = "sfb_dep_src";
        writeDepProject(*this, dir);
        track(dir + "/main.a");
        track(dir + "/main.fly.h");
        track(dir + "/util.fly.h");

        const char *argv[] = {"fly", "--lib", "--src-dir", "sfb_dep_src",
                              "-o", "sfb_dep_src/main.a"};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
        EXPECT_TRUE(exists("sfb_dep_src/main.a"));
    }

    // --src-dir overrides the default root and is meaningful at most once:
    // a second occurrence is a driver error and nothing is built.
    TEST_F(SingleFileBuildTest, SrcDirGivenTwiceIsAnError) {
        const std::string dir = "sfb_dep_twice";
        writeDepProject(*this, dir);
        track("sfb_dep_twice/main.a");

        const char *argv[] = {"fly", "--lib",
                              "--src-dir", "sfb_dep_twice", "--src-dir", ".",
                              "-o", "sfb_dep_twice/main.a"};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        // The option error stops the driver AND is a failure (exit 1).
        EXPECT_FALSE(drv.Execute());
        EXPECT_FALSE(exists("sfb_dep_twice/main.a"));
    }

    // ── --out-dir: every build artifact lands under the given directory ─────────

    // --lib + --out-dir → library archive AND headers go into the dir,
    // nothing is left in the CWD.
    TEST_F(SingleFileBuildTest, OutDirRedirectsAutoLibrary) {
        const std::string src = "sfb_odlib_src";
        const std::string dir = "sfb_od_lib";
        makeDir(src);
        CleanupDirs.push_back(dir);
        writeFile(src + "/sfb_odlib.fly", "namespace demo\npublic int answer() { out = 42 }\n");
        const char *argv[] = {"fly", "--lib", "--src-dir", "sfb_odlib_src",
                              "--out-dir", dir.c_str()};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        // Archive auto-named after the source root; headers named per module.
        EXPECT_TRUE(exists(dir + "/" + libName("sfb_odlib_src")));
        EXPECT_TRUE(exists(dir + "/sfb_odlib.fly.h"));
        EXPECT_FALSE(exists(libName("sfb_odlib_src")));  // not in the CWD
        EXPECT_FALSE(exists("sfb_odlib.fly.h"));
    }

    // void main() + --out-dir → executable produced inside the dir, not in the CWD.
    TEST_F(SingleFileBuildTest, OutDirRedirectsExecutable) {
        const std::string src = "sfb_odexe_src";
        const std::string dir = "sfb_od_exe";
        makeDir(src);
        CleanupDirs.push_back(dir);
        writeFile(src + "/sfb_odexe.fly", "namespace demo\nvoid main() {}\n");
        const char *argv[] = {"fly", "--src-dir", "sfb_odexe_src", "--out-dir", dir.c_str()};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        EXPECT_TRUE(exists(dir + "/" + exeName("sfb_odexe")));
        EXPECT_FALSE(exists(exeName("sfb_odexe")));
        EXPECT_FALSE(exists("sfb_odexe.fly.o")); // intermediate also redirected
    }

    // Emit backend (--emit-ll) + --out-dir → the .ll lands in the dir, no link step,
    // and auto-detect stays off (no executable produced).
    TEST_F(SingleFileBuildTest, OutDirRedirectsEmitLl) {
        const std::string src = "sfb_odll_src";
        const std::string dir = "sfb_od_ll";
        makeDir(src);
        CleanupDirs.push_back(dir);
        writeFile(src + "/sfb_odll.fly", "namespace demo\nvoid main() {}\n");
        const char *argv[] = {"fly", "--emit-ll", "--src-dir", "sfb_odll_src",
                              "--out-dir", dir.c_str()};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        EXPECT_TRUE(exists(dir + "/sfb_odll.fly.ll"));
        EXPECT_FALSE(exists("sfb_odll.fly.ll")); // not in the CWD
        EXPECT_FALSE(exists(dir + "/sfb_odll")); // emit mode never links
    }

    // Explicit --lib -o foo.a together with --out-dir → the archive is resolved under
    // the dir (build/foo.a); the per-module header also lands there.
    TEST_F(SingleFileBuildTest, OutDirWithExplicitOutput) {
        const std::string src = "sfb_odo_src";
        const std::string dir = "sfb_od_o";
        makeDir(src);
        CleanupDirs.push_back(dir);
        writeFile(src + "/sfb_odo.fly", "namespace demo\npublic int v() { out = 1 }\n");
        const char *argv[] = {"fly", "--lib", "-o", "foo.a",
                              "--src-dir", "sfb_odo_src", "--out-dir", dir.c_str()};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        EXPECT_TRUE(exists(dir + "/foo.a"));
        EXPECT_TRUE(exists(dir + "/sfb_odo.fly.h"));
        EXPECT_FALSE(exists("foo.a"));           // not in the CWD
    }

} // anonymous namespace
