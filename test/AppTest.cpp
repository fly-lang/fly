//===--------------------------------------------------------------------------------------------------------------===//
// test/AppTest.cpp - Full app compilation integration tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "TestUtils.h"
#include "Driver/Driver.h"
#include "AST/ASTBuilder.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTFunction.h"
#include "AST/ASTImport.h"
#include "AST/ASTModule.h"
#include "AST/ASTAttribute.h"
#include "AST/ASTParam.h"
#include "AST/ASTNameSpace.h"
#include "Frontend/InputFile.h"
#include "Parser/Parser.h"
#include "Sema/SemaContext.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/TargetParser/Host.h"
#include "gtest/gtest.h"
#include <fstream>

extern bool DebugLog;

namespace {
    using namespace fly;

    // ─── Source strings (mirror the Sources/*.fly files) ─────────────────────

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

    // ─── Sources for the "emit with resolved imports" regression ──────────────
    // The dependency is NOT listed on the command line: it is discovered from the
    // import via --src-dir. The entry instantiates a CLASS defined there and calls a
    // method on it — the cross-module class reference that per-file lowering cannot
    // resolve (the sibling module's CodeGen is null → crash).

    static constexpr const char *DepLibSource = R"(
namespace deplib

public class Greeter {
    int n

    public void bump() {
    }
}
)";

    static constexpr const char *DepMainSource = R"(
import deplib.Greeter

void main() {
    Greeter g = new Greeter()
    g.bump()
}
)";

    // ─── Driver-level fixture ─────────────────────────────────────────────────

    class AppTest : public ::testing::Test {
    public:
        const char *mainfly  = "main.fly";
        const char *utilsfly = "utils.fly";

        AppTest() {
            DebugLog = false;
            { std::ofstream f(mainfly);  f << MainAppSource; }
            { std::ofstream f(utilsfly); f << UtilsAppSource; }
            llvm::InitializeAllTargetInfos();
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmParsers();
            llvm::InitializeAllAsmPrinters();
        }

        ~AppTest() override {
            remove(mainfly);
            remove(utilsfly);
            // Every test shares this working directory, and --src-dir defaults to "."
            // — so a .fly left behind here would take part in the NEXT test's import
            // resolution, pulling an extra dependency and silently turning a per-file
            // emit into a combined one. Sweep the derived artifacts too: a test that
            // fails midway must not make the following ones fail as well.
            remove("main.fly.o");
            remove("utils.fly.o");
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
        const char *argv[] = {"fly", "-no-output", mainfly, utilsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // ─── Driver: emit formats ─────────────────────────────────────────────────

    TEST_F(AppTest, EmitLL) {
        deleteFile("main.fly.ll");
        deleteFile("utils.fly.ll");

        const char *argv[] = {"fly", "-emit-ll", mainfly, utilsfly};
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

        const char *argv[] = {"fly", "-emit-bc", mainfly, utilsfly};
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

        const char *argv[] = {"fly", "-emit-as", mainfly, utilsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());

        EXPECT_TRUE(std::ifstream("main.fly.s").good());
        EXPECT_TRUE(std::ifstream("utils.fly.s").good());

        deleteFile("main.fly.s");
        deleteFile("utils.fly.s");
    }

    // Regression: --emit-ll on a source whose imports are resolved from --src-dir.
    // The dependency module is pulled in by ResolveSourceDeps, so it is NOT an
    // explicitly listed input. Before the fix an emit action always forced per-file
    // lowering, so the cross-module class reference had a null CodeGen and the
    // compiler segfaulted; now such a build is lowered into ONE module.
    TEST_F(AppTest, EmitLLWithResolvedImports) {
        const char *depmain = "depmain.fly";
        const char *deplib  = "deplib.fly";
        { std::ofstream f(depmain); f << DepMainSource; }
        { std::ofstream f(deplib);  f << DepLibSource; }
        deleteFile("out.ll");
        deleteFile("depmain.fly.ll");
        deleteFile("deplib.fly.ll");

        // (a) explicit -o: one combined .ll written exactly there
        {
            const char *argv[] = {"fly", depmain, "--src-dir", ".", "-emit-ll", "-o", "out.ll"};
            Driver drv(argv);
            drv.BuildCompilerInstance();
            ASSERT_TRUE(drv.Execute());
            EXPECT_TRUE(std::ifstream("out.ll").good());
        }

        // (b) no -o: still ONE combined module. Assert on the COUNT rather than on which
        // file it is named after (that follows module order): exactly one .ll must exist,
        // never one per file — two would mean the per-file path was taken.
        {
            const char *argv[] = {"fly", depmain, "--src-dir", ".", "-emit-ll"};
            Driver drv(argv);
            drv.BuildCompilerInstance();
            ASSERT_TRUE(drv.Execute());
            const bool MainLL = std::ifstream("depmain.fly.ll").good();
            const bool LibLL  = std::ifstream("deplib.fly.ll").good();
            EXPECT_TRUE(MainLL || LibLL);   // something was emitted
            EXPECT_FALSE(MainLL && LibLL);  // but combined, not one per file
        }

        deleteFile("out.ll");
        deleteFile("depmain.fly.ll");
        deleteFile("deplib.fly.ll");
        remove(depmain);
        remove(deplib);
    }

    TEST_F(AppTest, EmitObj) {
        deleteFile("main.fly.o");
        deleteFile("utils.fly.o");

        const char *argv[] = {"fly", mainfly, utilsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());

        EXPECT_TRUE(std::ifstream("main.fly.o").good());
        EXPECT_TRUE(std::ifstream("utils.fly.o").good());

        deleteFile("main.fly.o");
        deleteFile("utils.fly.o");
    }

    TEST_F(AppTest, EmitOut) {
    	DebugLog = true;
        deleteFile("out");
        deleteFile("main.fly.o");
        deleteFile("utils.fly.o");

        const char *argv[] = {"fly", mainfly, utilsfly, "-o", "out"};
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
        deleteFile("utils.fly.o");
    }

    // -c stops after the object file even when -o names an artifact. Before -c
    // existed, "compile only" could be expressed only by OMITTING -o, so asking for
    // one combined object without linking it was impossible.
    TEST_F(AppTest, CompileOnlyWithOutput) {
        deleteFile("combined.o");
        deleteFile("combined");
        deleteFile("combined.exe");

        const char *argv[] = {"fly", mainfly, utilsfly, "-c", "-o", "combined.o"};
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
    // out is respected instead (covered by CompileOnlyWithOutput, which passes -o
    // combined.o and gets exactly that).
    TEST_F(AppTest, OutputExtensionIsAppended) {
        deleteFile("extless.ll");
        deleteFile("extless.o");

        {
            const char *argv[] = {"fly", mainfly, utilsfly, "-emit-ll", "-o", "extless"};
            Driver drv(argv);
            drv.BuildCompilerInstance();
            ASSERT_TRUE(drv.Execute());
            EXPECT_TRUE(std::ifstream("extless.ll").good());
        }
        {
            const char *argv[] = {"fly", mainfly, utilsfly, "-c", "-o", "extless"};
            Driver drv(argv);
            drv.BuildCompilerInstance();
            ASSERT_TRUE(drv.Execute());
            EXPECT_TRUE(std::ifstream("extless.o").good());
        }

        deleteFile("extless.ll");
        deleteFile("extless.o");
    }

    // Regression: --lib-dyn (and --header alongside any link) also emits a .fly.h.
    // Only the archive path filtered it out, so the linker received the header and
    // failed with "unknown file type" — broken since before 0.13.8.
    TEST_F(AppTest, DynamicLibraryDoesNotLinkHeader) {
        const char *libfly = "dynlib.fly";
        { std::ofstream f(libfly); f << UtilsAppSource; }

        const char *argv[] = {"fly", libfly, "--lib-dyn", "-o", "dynout"};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        const bool ok = drv.Execute();

        remove(libfly);
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

    // Verify that main(string[] args) compiles without errors.
    TEST_F(AppTest, MainWithArgs) {
        const char *argsfly = "main_args.fly";
        { std::ofstream f(argsfly); f << MainArgsSource; }

        const char *argv[] = {"fly", "-no-output", argsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        bool ok = drv.Execute();
        remove(argsfly);
        EXPECT_TRUE(ok);
    }

    // Emit IR for main() and verify the expected LLVM patterns:
    //   - C entry-point signature:  define i32 @main(i32 %0, ptr %1)
    //   - env_init call to make argc/argv available via fly.os.env.argsGet()
    TEST_F(AppTest, MainWithArgsEmitLL) {
        const char *argsfly = "main_args.fly";
        const char *argsll  = "main_args.fly.ll";
        deleteFile(argsll);
        { std::ofstream f(argsfly); f << MainArgsSource; }

        const char *argv[] = {"fly", "-emit-ll", argsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        bool ok = drv.Execute();
        remove(argsfly);
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
