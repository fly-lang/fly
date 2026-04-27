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
#include "Driver/DriverOptions.h"
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

extern bool DebugEnabled;

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

main() {
    string hello = "hello"
    int size = 0
    fly.str.len(hello, size)
}
)";

    class ImportLibTest : public ::testing::Test {
    public:
        const char *mainfly = "main.fly";

        void SetUpWithSource(const char *Src) {
            DebugEnabled = false;
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
