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

    // ─── ImportFlyLib ─────────────────────────────────────────────────────────

#ifndef FLY_LIB_FLY_DIR
#define FLY_LIB_FLY_DIR "."
#endif

    // Main module: imports fly.str and calls fly.str.len().
    // The stdlib header (strings.fly.h) is loaded implicitly by the compiler
    // from FLY_LIB_FLY_DIR; no explicit str.fly source needs to be passed.
    static constexpr const char *FlyStringMainSource = R"(
import fly.str

void main() {
    string hello = "hello"
    int size = fly.str.len(hello)
}
)";

    class ImportLibTest : public ::testing::Test {
    public:
        const char *mainfly = "main.fly";

        void SetUpWithSource(const char *Src) {
            DebugLog = false;
            { std::ofstream f(mainfly); f << Src; }
            llvm::InitializeAllTargetInfos();
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmParsers();
            llvm::InitializeAllAsmPrinters();
        }

        ~ImportLibTest() override {
            remove(mainfly);
            llvm::outs().flush();
        }

        static void deleteFile(const char *path) { remove(path); }
    };

    // Compiles main.fly with -no-output; stdlib headers loaded implicitly.
    TEST_F(ImportLibTest, ImportFlyLib) {
    	SetUpWithSource(FlyStringMainSource);
        const char *argv[] = {"fly", "-no-output", mainfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // Verify that -L <dir> makes the namespace in that dir available for import.
    // We create a temporary lib dir with a source file declaring "namespace ext.lib",
    // then compile a user file that imports "ext.lib" — it should succeed.
    TEST_F(ImportLibTest, LibDirFlag) {
        // Create a temporary lib directory with one .fly source file
        const char *libDir  = "tmp_libdir";
        const char *libFile = "tmp_libdir/mylib.fly";
        const char *userFile = "tmp_libdir_main.fly";

        llvm::sys::fs::create_directory(libDir);
        { std::ofstream f(libFile);  f << "namespace ext.lib\npublic void libFunc() {}\n"; }
        { std::ofstream f(userFile); f << "import ext.lib\nvoid main() { ext.lib.libFunc() }\n"; }

        const char *argv[] = {"fly", "-no-output", "-L", libDir, userFile};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        remove(libFile);
        remove(userFile);
        llvm::sys::fs::remove(libDir);

        EXPECT_TRUE(ok);
    }

    // A non-.fly file in a -L dir should produce a warning but not fail compilation.
    // The .fly.h generated header is always skipped silently (no warning).
    TEST_F(ImportLibTest, LibDirNonFlyFileWarning) {
        const char *libDir    = "tmp_warn_libdir";
        const char *flyFile   = "tmp_warn_libdir/mylib.fly";
        const char *flyHFile  = "tmp_warn_libdir/mylib.fly.h";  // silently skipped
        const char *txtFile   = "tmp_warn_libdir/README.txt";   // triggers warning
        const char *userFile  = "tmp_warn_libdir_main.fly";

        llvm::sys::fs::create_directory(libDir);
        { std::ofstream f(flyFile);  f << "namespace warn.lib\npublic void warnFunc() {}\n"; }
        { std::ofstream f(flyHFile); f << "namespace warn.lib\npublic void warnFunc()\n"; }
        { std::ofstream f(txtFile);  f << "this is a readme\n"; }
        { std::ofstream f(userFile); f << "import warn.lib\nvoid main() { warn.lib.warnFunc() }\n"; }

        const char *argv[] = {"fly", "-no-output", "-L", libDir, userFile};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        // Compilation succeeds (warning does not abort)
        bool ok = drv.Execute();

        remove(flyFile);
        remove(flyHFile);
        remove(txtFile);
        remove(userFile);
        llvm::sys::fs::remove(libDir);

        EXPECT_TRUE(ok);
    }

    // Emits LLVM IR for main.fly and verifies that fly.str.len is
    // referenced as an extern declare with the namespace-mangled name.
    TEST_F(ImportLibTest, ImportFlyLibEmitLL) {
    	SetUpWithSource(FlyStringMainSource);
        const char *llMainFile = "main.fly.ll";
        deleteFile(llMainFile);

        const char *argv[] = {"fly", "-emit-ll", mainfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());

        std::ifstream ll(llMainFile);
        ASSERT_TRUE(ll.good()) << "Expected " << llMainFile << " to be emitted";
        std::string ir((std::istreambuf_iterator<char>(ll)),
                        std::istreambuf_iterator<char>());

        // fly.str.len(const string src, int out) mangled: _F7fly_str3len_Ss_i
        EXPECT_NE(ir.find("_F7fly_str3len_Ss_i"), std::string::npos);

        deleteFile(llMainFile);
    }

} // anonymous namespace
