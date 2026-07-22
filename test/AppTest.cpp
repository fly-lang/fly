//===--------------------------------------------------------------------------------------------------------------===//
// test/AppTest.cpp - Full app compilation integration tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// The CLI is directory-based: every test keeps its sources in a dedicated
// subdirectory and points fly at it with --src-dir. The entry (main.fly) is
// discovered there; utils.fly is pulled in through its import.
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "TestUtils.h"
#include "Driver/Driver.h"
#include "Frontend/InputFile.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/TargetParser/Host.h"
#include "gtest/gtest.h"
#include <fstream>

extern bool DebugLog;

namespace {
    using namespace fly;

    // ─── Source strings ──────────────────────────────────────────────────────

    // Library module: namespace utils with a public function
    static constexpr const char *UtilsAppSource = R"(
namespace utils

public void add(int a, int b) {
    int r = a + b
}
)";

    // Main module: imports utils and exercises all language features in one file
    static constexpr const char *MainAppSource = R"(
import utils

public enum Color {
    RED, GREEN, BLUE
}

public interface Shape {
    void area()
}

public struct Point {
    int x
    int y
}

public class Circle : Shape {
    int radius

    public void area() {
    }
}

void main() {
    Color c = Color.RED
    Point p = new Point()
    p.x = 10
    p.y = 20
    Circle circle = new Circle()
}

void helper(int a, int b) {
    int r = a + b
}
)";

    // main() with no parameters — args accessed via fly.os.env.argsGet()
    static constexpr const char *MainArgsSource = R"(
void main() {
}
)";

    // ─── Driver-level fixture ─────────────────────────────────────────────────

    class AppTest : public ::testing::Test {
    public:
        // Each test compiles this directory (entry: main.fly, dep: utils.fly).
        const char *srcDir = "app_src";

        AppTest() {
            DebugLog = false;
            llvm::sys::fs::create_directory(srcDir);
            { std::ofstream f(std::string(srcDir) + "/main.fly");  f << MainAppSource; }
            { std::ofstream f(std::string(srcDir) + "/utils.fly"); f << UtilsAppSource; }
            llvm::InitializeAllTargetInfos();
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmParsers();
            llvm::InitializeAllAsmPrinters();
        }

        ~AppTest() override {
            llvm::sys::fs::remove_directories(srcDir);
            // Derived artifacts land in the CWD (named from the module basename):
            // sweep them so a test that fails midway cannot poison the next one.
            remove("main.fly.o");
            remove("utils.fly.o");
            remove("main.fly.ll");
            remove("utils.fly.ll");
            llvm::outs().flush();
        }

        static void deleteFile(const char *path) { remove(path); }
    };

    // ─── Driver: meta options ─────────────────────────────────────────────────

    TEST_F(AppTest, ShowVersion) {
        const char *argv[] = {"fly", "-version"};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    TEST_F(AppTest, NoOutput) {
        const char *argv[] = {"fly", "-no-output", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // ─── Driver: emit formats ─────────────────────────────────────────────────
    // A non-linking stage compiles the whole directory: every source is an
    // explicit input, so without -o each file gets its own artifact (per-file
    // emission, exactly as the old multi-file CLI behaved).

    TEST_F(AppTest, EmitLL) {
        deleteFile("main.fly.ll");
        deleteFile("utils.fly.ll");

        const char *argv[] = {"fly", "-emit-ll", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());

        EXPECT_TRUE(std::ifstream("main.fly.ll").good());
        EXPECT_TRUE(std::ifstream("utils.fly.ll").good());

        deleteFile("main.fly.ll");
        deleteFile("utils.fly.ll");
    }

    TEST_F(AppTest, EmitBC) {
        deleteFile("main.fly.bc");
        deleteFile("utils.fly.bc");

        const char *argv[] = {"fly", "-emit-bc", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());

        EXPECT_TRUE(std::ifstream("main.fly.bc").good());
        EXPECT_TRUE(std::ifstream("utils.fly.bc").good());

        deleteFile("main.fly.bc");
        deleteFile("utils.fly.bc");
    }

    TEST_F(AppTest, EmitAS) {
        deleteFile("main.fly.s");
        deleteFile("utils.fly.s");

        const char *argv[] = {"fly", "-emit-as", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());

        EXPECT_TRUE(std::ifstream("main.fly.s").good());
        EXPECT_TRUE(std::ifstream("utils.fly.s").good());

        deleteFile("main.fly.s");
        deleteFile("utils.fly.s");
    }

    // Explicit -o with an emit action: the single combined artifact is written
    // exactly there.
    TEST_F(AppTest, EmitLLWithExplicitOutput) {
        deleteFile("out.ll");

        const char *argv[] = {"fly", "-emit-ll", "--src-dir", srcDir, "-o", "out.ll"};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());
        EXPECT_TRUE(std::ifstream("out.ll").good());

        deleteFile("out.ll");
    }

    // ─── Compile only (-c): per-file objects, no link ─────────────────────────

    TEST_F(AppTest, CompileOnlyEmitsPerFileObjects) {
        deleteFile("main.fly.o");
        deleteFile("utils.fly.o");

        const char *argv[] = {"fly", "-c", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());

        EXPECT_TRUE(std::ifstream("main.fly.o").good());
        EXPECT_TRUE(std::ifstream("utils.fly.o").good());

        deleteFile("main.fly.o");
        deleteFile("utils.fly.o");
    }

    // -c stops after the object file even when -o names an artifact: one combined
    // object, no link.
    TEST_F(AppTest, CompileOnlyWithOutput) {
        deleteFile("combined.o");
        deleteFile("combined");
        deleteFile("combined.exe");

        const char *argv[] = {"fly", "-c", "-o", "combined.o", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());

        EXPECT_TRUE(std::ifstream("combined.o").good());
        EXPECT_FALSE(std::ifstream("combined").good());
        EXPECT_FALSE(std::ifstream("combined.exe").good());

        deleteFile("combined.o");
    }

    // -o carries no extension: the one implied by the chosen format is appended.
    // `--emit-ll -o out` → out.ll, `-c -o out` → out.o. An extension the user spelled
    // out is respected instead (covered by CompileOnlyWithOutput).
    TEST_F(AppTest, OutputExtensionIsAppended) {
        deleteFile("extless.ll");
        deleteFile("extless.o");

        {
            const char *argv[] = {"fly", "-emit-ll", "-o", "extless", "--src-dir", srcDir};
            Driver drv(argv);
            drv.BuildCompilerInstance();
            ASSERT_TRUE(drv.Execute());
            EXPECT_TRUE(std::ifstream("extless.ll").good());
        }
        {
            const char *argv[] = {"fly", "-c", "-o", "extless", "--src-dir", srcDir};
            Driver drv(argv);
            drv.BuildCompilerInstance();
            ASSERT_TRUE(drv.Execute());
            EXPECT_TRUE(std::ifstream("extless.o").good());
        }

        deleteFile("extless.ll");
        deleteFile("extless.o");
    }

    // ─── Link step ────────────────────────────────────────────────────────────

    TEST_F(AppTest, EmitOut) {
        deleteFile("out");
        deleteFile("out.exe");
        deleteFile("main.fly.o");

        const char *argv[] = {"fly", "--src-dir", srcDir, "-o", "out"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());

        const llvm::Triple &T = TargetInfo::CreateTargetInfo(
            CI.getDiagnostics(), CI.getTargetOptions())->getTriple();
        if (T.isWindowsMSVCEnvironment()) {
            EXPECT_TRUE(std::ifstream("out.exe").good());
            deleteFile("out.exe");
        } else {
            EXPECT_TRUE(std::ifstream("out").good());
            deleteFile("out");
        }

        deleteFile("main.fly.o");
    }

    // No -o: the executable is auto-named after the file declaring main().
    TEST_F(AppTest, AutoNamedExecutable) {
        deleteFile("main");
        deleteFile("main.exe");
        deleteFile("main.fly.o");

        const char *argv[] = {"fly", "--src-dir", srcDir};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());

        const llvm::Triple &T = TargetInfo::CreateTargetInfo(
            CI.getDiagnostics(), CI.getTargetOptions())->getTriple();
        if (T.isWindowsMSVCEnvironment()) {
            EXPECT_TRUE(std::ifstream("main.exe").good());
            deleteFile("main.exe");
        } else {
            EXPECT_TRUE(std::ifstream("main").good());
            deleteFile("main");
        }

        deleteFile("main.fly.o");
    }

    // Regression: --lib-dyn (and --header alongside any link) also emits a .fly.h.
    // Only the archive path filtered it out, so the linker received the header and
    // failed with "unknown file type" — broken since before 0.13.8.
    TEST_F(AppTest, DynamicLibraryDoesNotLinkHeader) {
        const char *libDir = "dynlib_src";
        llvm::sys::fs::create_directory(libDir);
        { std::ofstream f(std::string(libDir) + "/dynlib.fly"); f << UtilsAppSource; }

        const char *argv[] = {"fly", "--lib-dyn", "-o", "dynout", "--src-dir", libDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        const bool ok = drv.Execute();

        llvm::sys::fs::remove_directories(libDir);
        deleteFile("dynlib.fly.o");
        deleteFile("dynlib.fly.h");
        deleteFile("dynout.so");
        deleteFile("dynout.dll");
        deleteFile("dynout.dylib");
        deleteFile("dynout.lib");
        deleteFile("dynout.exp");
        EXPECT_TRUE(ok);
    }

    // ─── main(string[] args) ──────────────────────────────────────────────────

    // Verify that main() with args via fly.os.env compiles without errors.
    TEST_F(AppTest, MainWithArgs) {
        const char *argsDir = "args_src";
        llvm::sys::fs::create_directory(argsDir);
        { std::ofstream f(std::string(argsDir) + "/main_args.fly"); f << MainArgsSource; }

        const char *argv[] = {"fly", "-no-output", "--src-dir", argsDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        bool ok = drv.Execute();
        llvm::sys::fs::remove_directories(argsDir);
        EXPECT_TRUE(ok);
    }

    // Emit IR for main() and verify the expected LLVM patterns:
    //   - C entry-point signature:  define i32 @main(i32 %0, ptr %1)
    //   - env_init call to make argc/argv available via fly.os.env.argsGet()
    TEST_F(AppTest, MainWithArgsEmitLL) {
        const char *argsDir = "args_src";
        const char *argsll  = "main_args.fly.ll";
        llvm::sys::fs::create_directory(argsDir);
        deleteFile(argsll);
        { std::ofstream f(std::string(argsDir) + "/main_args.fly"); f << MainArgsSource; }

        const char *argv[] = {"fly", "-emit-ll", "--src-dir", argsDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        bool ok = drv.Execute();
        llvm::sys::fs::remove_directories(argsDir);
        ASSERT_TRUE(ok);
        ASSERT_TRUE(std::ifstream(argsll).good());

        std::ifstream llfile(argsll);
        std::string ir((std::istreambuf_iterator<char>(llfile)),
                        std::istreambuf_iterator<char>());
        deleteFile(argsll);

        // C entry-point signature must carry argc (i32) and argv (ptr) for C ABI
        EXPECT_NE(ir.find("define i32 @main(i32"), std::string::npos);
        // env_init wires argc/argv into the env args store for fly.os.env.argsGet()
        EXPECT_NE(ir.find("env_init"), std::string::npos);
    }

} // anonymous namespace
