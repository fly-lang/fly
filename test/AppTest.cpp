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
