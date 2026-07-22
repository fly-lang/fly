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
        // Directory mode: the source under test lives in a dedicated subdirectory.
        const char *srcDir = "implib_src";

        void SetUpWithSource(const char *Src) {
            DebugLog = false;
            llvm::sys::fs::create_directory(srcDir);
            { std::ofstream f(std::string(srcDir) + "/main.fly"); f << Src; }
            llvm::InitializeAllTargetInfos();
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmParsers();
            llvm::InitializeAllAsmPrinters();
        }

        ~ImportLibTest() override {
            llvm::sys::fs::remove_directories(srcDir);
            llvm::outs().flush();
        }

        static void deleteFile(const char *path) { remove(path); }
    };

    // Compiles the source dir with -no-output; stdlib headers loaded implicitly.
    TEST_F(ImportLibTest, ImportFlyLib) {
    	SetUpWithSource(FlyStringMainSource);
        const char *argv[] = {"fly", "-no-output", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // Round-trip a MULTI-RETURN function through a generated header: emit the .fly.h
    // with --header, then compile a consumer that only sees that header.
    //
    // This is the pair of steps that has to agree. What the header spells
    // (`int, int divmod(const int, const int)`) is not the ABI: the Resolver lowers
    // the extra returns into trailing __out_N params, so the callable signature is
    // divmod(int, int, int, int). ParseHeader used to record only the first return
    // type, leaving the declaration with no return types at all — the consumer then
    // saw a plain divmod(int, int) and no call could ever match it. Emitting the
    // signature correctly is not enough; reading it back has to reconstruct the ABI.
    TEST_F(ImportLibTest, MultiReturnThroughGeneratedHeader) {
        const char *libDir   = "tmp_mrlib";
        const char *libSrc   = "tmp_mrlib/mrlib.fly";
        const char *libHdr   = "tmp_mrlib/mrlib.fly.h";
        const char *userDir  = "tmp_mr_main";
        const char *userFile = "tmp_mr_main/main.fly";

        llvm::sys::fs::create_directory(libDir);
        llvm::sys::fs::create_directory(userDir);
        { std::ofstream f(libSrc);
          f << "namespace mr\n\n"
               "public int,int divmod(const int a, const int b) {\n"
               "    out[0] = a / b\n"
               "    out[1] = a % b\n"
               "}\n"; }

        // Step 1: generate the header. --no-output keeps this at the frontend —
        // the header is the artifact under test, and no backend target is needed.
        {
            const char *argv[] = {"fly", "--header", "--no-output", "--src-dir", libDir, "--out-dir", libDir};
            Driver drv(argv);
            drv.BuildCompilerInstance();
            ASSERT_TRUE(drv.Execute());
        }
        ASSERT_TRUE(std::ifstream(libHdr).good());

        // The header must carry both return types…
        {
            std::ifstream h(libHdr);
            std::string text((std::istreambuf_iterator<char>(h)),
                              std::istreambuf_iterator<char>());
            EXPECT_NE(text.find("int, int divmod"), std::string::npos) << text;
        }

        // Step 2: a consumer that sees ONLY the header. The outputs are passed as
        // trailing arguments — the lowered form the archive symbol actually exposes.
        { std::ofstream f(userFile);
          f << "import mr\n\n"
               "void main() {\n"
               "    int q = 0\n"
               "    int r = 0\n"
               "    mr.divmod(17, 5, q, r)\n"
               "}\n"; }

        const char *argv[] = {"fly", "-no-output", "-L", libDir, "--src-dir", userDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        const bool ok = drv.Execute();

        llvm::sys::fs::remove_directories(userDir);
        llvm::sys::fs::remove_directories(libDir);

        EXPECT_TRUE(ok);
    }

    // Verify that -L <dir> makes the namespace in that dir available for import.
    // We create a temporary lib dir with a source file declaring "namespace ext.lib",
    // then compile a user file that imports "ext.lib" — it should succeed.
    TEST_F(ImportLibTest, LibDirFlag) {
        // Create a temporary lib directory with one .fly source file
        const char *libDir   = "tmp_libdir";
        const char *libFile  = "tmp_libdir/mylib.fly";
        const char *userDir  = "tmp_libdir_main";
        const char *userFile = "tmp_libdir_main/main.fly";

        llvm::sys::fs::create_directory(libDir);
        llvm::sys::fs::create_directory(userDir);
        { std::ofstream f(libFile);  f << "namespace ext.lib\npublic void libFunc() {}\n"; }
        { std::ofstream f(userFile); f << "import ext.lib\nvoid main() { ext.lib.libFunc() }\n"; }

        const char *argv[] = {"fly", "-no-output", "-L", libDir, "--src-dir", userDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        bool ok = drv.Execute();

        llvm::sys::fs::remove_directories(userDir);
        llvm::sys::fs::remove_directories(libDir);

        EXPECT_TRUE(ok);
    }

    // A non-.fly file in a -L dir should produce a warning but not fail compilation.
    // The .fly.h generated header is always skipped silently (no warning).
    TEST_F(ImportLibTest, LibDirNonFlyFileWarning) {
        const char *libDir    = "tmp_warn_libdir";
        const char *flyFile   = "tmp_warn_libdir/mylib.fly";
        const char *flyHFile  = "tmp_warn_libdir/mylib.fly.h";  // silently skipped
        const char *txtFile   = "tmp_warn_libdir/README.txt";   // triggers warning
        const char *userDir   = "tmp_warn_libdir_main";
        const char *userFile  = "tmp_warn_libdir_main/main.fly";

        llvm::sys::fs::create_directory(libDir);
        llvm::sys::fs::create_directory(userDir);
        { std::ofstream f(flyFile);  f << "namespace warn.lib\npublic void warnFunc() {}\n"; }
        { std::ofstream f(flyHFile); f << "namespace warn.lib\npublic void warnFunc()\n"; }
        { std::ofstream f(txtFile);  f << "this is a readme\n"; }
        { std::ofstream f(userFile); f << "import warn.lib\nvoid main() { warn.lib.warnFunc() }\n"; }

        const char *argv[] = {"fly", "-no-output", "-L", libDir, "--src-dir", userDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        // Compilation succeeds (warning does not abort)
        bool ok = drv.Execute();

        llvm::sys::fs::remove_directories(userDir);
        llvm::sys::fs::remove_directories(libDir);

        EXPECT_TRUE(ok);
    }

    // Emits LLVM IR for main.fly and verifies that fly.str.len is
    // referenced as an extern declare with the namespace-mangled name.
    TEST_F(ImportLibTest, ImportFlyLibEmitLL) {
    	SetUpWithSource(FlyStringMainSource);
        const char *llMainFile = "main.fly.ll";
        deleteFile(llMainFile);

        const char *argv[] = {"fly", "-emit-ll", "--src-dir", srcDir};
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
