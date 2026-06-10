//===--------------------------------------------------------------------------------------------------------------===//
// test/SingleFileBuildTest.cpp - single-file build: auto-detect + import dep graph
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// Covers the single-file build feature added to the driver/frontend:
//   * output-type auto-detection from the entry AST (main → exe; suite or
//     main+--test → test exe; otherwise lib), with auto-naming;
//   * --lib/--shared forcing a library even when a main() is present;
//   * the gating that keeps explicit/per-target builds (notably flyp's) untouched:
//       - auto-detect only with a single input AND no -o AND an object backend;
//       - import-based dependency pulling ONLY with an explicit --src-dir.
//
// Gate assertions read FrontendOptions::AutoDetectOutput right after
// BuildCompilerInstance() (set in Driver::BuildOptions, before any codegen), so
// they need no link step. Behavioural cases run a full Execute() and inspect the
// resolved options / produced artifacts.
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

    // Single input file, no -o, object backend → auto-detect is enabled.
    TEST_F(SingleFileBuildTest, AutoDetectEnabledForSingleFile) {
        writeFile("sfb_gate.fly", "namespace demo\npublic void f() {}\n");
        const char *argv[] = {"fly", "sfb_gate.fly"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        EXPECT_TRUE(CI.getFrontendOptions().AutoDetectOutput);
    }

    // Explicit -o means the caller already owns the output: auto-detect off.
    // This is what keeps flyp's per-target builds (always -o) from being reinterpreted.
    TEST_F(SingleFileBuildTest, ExplicitOutputDisablesAutoDetect) {
        writeFile("sfb_gate.fly", "namespace demo\nvoid main() {}\n");
        const char *argv[] = {"fly", "sfb_gate.fly", "-o", "sfb_gate_out"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        EXPECT_FALSE(CI.getFrontendOptions().AutoDetectOutput);
    }

    // Emit backends produce intermediates and must never trigger a link step.
    TEST_F(SingleFileBuildTest, EmitLlDisablesAutoDetect) {
        writeFile("sfb_gate.fly", "namespace demo\nvoid main() {}\n");
        const char *argv[] = {"fly", "--emit-ll", "sfb_gate.fly"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        EXPECT_FALSE(CI.getFrontendOptions().AutoDetectOutput);
    }

    TEST_F(SingleFileBuildTest, NoOutputDisablesAutoDetect) {
        writeFile("sfb_gate.fly", "namespace demo\nvoid main() {}\n");
        const char *argv[] = {"fly", "--no-output", "sfb_gate.fly"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        EXPECT_FALSE(CI.getFrontendOptions().AutoDetectOutput);
    }

    // More than one explicit input keeps the classic explicit-build behaviour.
    TEST_F(SingleFileBuildTest, MultipleFilesDisableAutoDetect) {
        writeFile("sfb_a.fly", "namespace demo\npublic void a() {}\n");
        writeFile("sfb_b.fly", "namespace demo\npublic void b() {}\n");
        const char *argv[] = {"fly", "sfb_a.fly", "sfb_b.fly"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        EXPECT_FALSE(CI.getFrontendOptions().AutoDetectOutput);
    }

    // ── Output-type detection (full Execute) ────────────────────────────────────

    // No main, no suite → static library + header, auto-named from the file stem.
    TEST_F(SingleFileBuildTest, AutoDetectLibraryWhenNoMainNoSuite) {
        writeFile("sfb_lib.fly", "namespace demo\npublic int answer() { out = 42 }\n");
        track(libName("sfb_lib"));
        track("sfb_lib.fly.h");
        const char *argv[] = {"fly", "sfb_lib.fly"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        EXPECT_TRUE(CI.getFrontendOptions().CreateLibrary);
        EXPECT_TRUE(CI.getFrontendOptions().CreateHeader);
        EXPECT_FALSE(CI.getCodeGenOptions().TestMode);
        EXPECT_EQ(CI.getFrontendOptions().getOutputFile(), "sfb_lib");
        EXPECT_TRUE(exists(libName("sfb_lib")));
        EXPECT_TRUE(exists("sfb_lib.fly.h"));
    }

    // void main() and no library flag → executable, auto-named, not test mode.
    TEST_F(SingleFileBuildTest, AutoDetectExecutableFromMain) {
        writeFile("sfb_exe.fly", "namespace demo\nvoid main() {}\n");
        track(exeName("sfb_exe"));
        track("sfb_exe.fly.o"); // intermediate object (kept on the exe/test path)
        const char *argv[] = {"fly", "sfb_exe.fly"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        EXPECT_FALSE(CI.getFrontendOptions().CreateLibrary);
        EXPECT_FALSE(CI.getCodeGenOptions().TestMode);
        EXPECT_EQ(CI.getFrontendOptions().getOutputFile(), "sfb_exe");
        EXPECT_TRUE(exists(exeName("sfb_exe")));
    }

    // --lib forces a library even when a main() exists (no executable produced).
    TEST_F(SingleFileBuildTest, LibFlagOverridesMain) {
        writeFile("sfb_ovr.fly", "namespace demo\nvoid main() {}\npublic int v() { out = 1 }\n");
        track(libName("sfb_ovr"));
        track("sfb_ovr.fly.h");
        const char *argv[] = {"fly", "sfb_ovr.fly", "--lib"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        EXPECT_TRUE(CI.getFrontendOptions().CreateLibrary);
        EXPECT_TRUE(exists(libName("sfb_ovr")));
        EXPECT_FALSE(exists(exeName("sfb_ovr"))); // not linked as an executable
    }

    // A suite (test methods named *Test) → test mode is enabled automatically,
    // without an explicit --test flag.
    TEST_F(SingleFileBuildTest, AutoDetectTestFromSuite) {
        writeFile("sfb_suite.fly",
                  "import fly.assert\n\n"
                  "suite SfbSuite {\n"
                  "    void answerTest() {\n"
                  "        case \"trivial\": {\n"
                  "            assert.assertTrue(1 > 0, 1)\n"
                  "        }\n"
                  "    }\n"
                  "}\n");
        track(exeName("sfb_suite"));
        track("sfb_suite.fly.o");
        const char *argv[] = {"fly", "sfb_suite.fly", "-L", FLY_LIB_FLY_DIR};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        EXPECT_TRUE(CI.getCodeGenOptions().TestMode);
        EXPECT_FALSE(CI.getFrontendOptions().CreateLibrary);
    }

    // main() + --test → test executable (test mode on, still an executable).
    TEST_F(SingleFileBuildTest, MainPlusTestFlagIsTestMode) {
        writeFile("sfb_maintest.fly", "namespace demo\nvoid main() {}\n");
        track(exeName("sfb_maintest"));
        track("sfb_maintest.fly.o");
        const char *argv[] = {"fly", "sfb_maintest.fly", "--test", "-L", FLY_LIB_FLY_DIR};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        EXPECT_TRUE(CI.getCodeGenOptions().TestMode);
        EXPECT_FALSE(CI.getFrontendOptions().CreateLibrary);
    }

    // ── Import dependency graph (--src-dir) ─────────────────────────────────────

    // Helper: lay down a tiny two-file project where the entry imports a sibling
    // namespace via a wildcard import and calls one of its functions.
    static void writeDepProject(SingleFileBuildTest &T, const std::string &dir,
                                const std::string &entry) {
        llvm::sys::fs::create_directory(dir);
        T.CleanupDirs.push_back(dir);
        T.writeFile(dir + "/util.fly",
                    "namespace dep.util\n\npublic int helper() { out = 41 }\n");
        T.writeFile(entry,
                    "namespace dep\n\nimport dep.util.*\n\n"
                    "public int compute() { out = helper() + 1 }\n");
    }

    // With --src-dir the entry's import is resolved to the sibling source and the
    // whole graph compiles into one library.
    TEST_F(SingleFileBuildTest, SrcDirResolvesImportedDependency) {
        const std::string dir = "sfb_dep_src";
        writeDepProject(*this, dir, dir + "/main.fly");
        track(dir + "/main.a");
        track(dir + "/main.fly.h");
        track(dir + "/util.fly.h");

        const char *argv[] = {"fly", "sfb_dep_src/main.fly",
                              "--lib", "--src-dir", "sfb_dep_src",
                              "-o", "sfb_dep_src/main.a"};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
        EXPECT_TRUE(exists("sfb_dep_src/main.a"));
    }

    // Without --src-dir the sibling is NOT pulled in implicitly: the unresolved
    // call makes compilation fail. This is the guarantee that flyp's per-target
    // single-source builds never absorb neighbouring files.
    TEST_F(SingleFileBuildTest, WithoutSrcDirNoImplicitDependencyPull) {
        const std::string dir = "sfb_dep_nopull";
        writeDepProject(*this, dir, dir + "/main.fly");

        const char *argv[] = {"fly", "sfb_dep_nopull/main.fly", "--lib",
                              "-o", "sfb_dep_nopull/main.a"};
        track("sfb_dep_nopull/main.a");
        track("sfb_dep_nopull/main.fly.h");
        Driver drv(argv);
        drv.BuildCompilerInstance();
        // dep.util.helper is unresolved (no source pulled, no header on -L).
        EXPECT_FALSE(drv.Execute());
    }

    // ── --out-dir: every build artifact lands under the given directory ─────────

    // No main/suite + --out-dir → library archive AND header go into the dir,
    // nothing is left in the CWD.
    TEST_F(SingleFileBuildTest, OutDirRedirectsAutoLibrary) {
        const std::string dir = "sfb_od_lib";
        CleanupDirs.push_back(dir);
        writeFile("sfb_odlib.fly", "namespace demo\npublic int answer() { out = 42 }\n");
        const char *argv[] = {"fly", "sfb_odlib.fly", "--out-dir", dir.c_str()};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        EXPECT_TRUE(exists(dir + "/" + libName("sfb_odlib")));
        EXPECT_TRUE(exists(dir + "/sfb_odlib.fly.h"));
        EXPECT_FALSE(exists(libName("sfb_odlib")));  // not in the CWD
        EXPECT_FALSE(exists("sfb_odlib.fly.h"));
    }

    // void main() + --out-dir → executable produced inside the dir, not in the CWD.
    TEST_F(SingleFileBuildTest, OutDirRedirectsExecutable) {
        const std::string dir = "sfb_od_exe";
        CleanupDirs.push_back(dir);
        writeFile("sfb_odexe.fly", "namespace demo\nvoid main() {}\n");
        const char *argv[] = {"fly", "sfb_odexe.fly", "--out-dir", dir.c_str()};
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
        const std::string dir = "sfb_od_ll";
        CleanupDirs.push_back(dir);
        writeFile("sfb_odll.fly", "namespace demo\nvoid main() {}\n");
        const char *argv[] = {"fly", "--emit-ll", "sfb_odll.fly", "--out-dir", dir.c_str()};
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
        const std::string dir = "sfb_od_o";
        CleanupDirs.push_back(dir);
        writeFile("sfb_odo.fly", "namespace demo\npublic int v() { out = 1 }\n");
        const char *argv[] = {"fly", "sfb_odo.fly", "--lib", "-o", "foo.a",
                              "--out-dir", dir.c_str()};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        EXPECT_TRUE(ok);
        EXPECT_TRUE(exists(dir + "/foo.a"));
        EXPECT_TRUE(exists(dir + "/sfb_odo.fly.h"));
        EXPECT_FALSE(exists("foo.a"));           // not in the CWD
    }

} // anonymous namespace
