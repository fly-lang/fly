//===--------------------------------------------------------------------------------------------------------------===//
// test/ImportTest.cpp - Import statement integration tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "TestUtils.h"
#include "Driver/Driver.h"
#include "llvm/Support/TargetSelect.h"
#include "gtest/gtest.h"
#include <fstream>

extern bool DebugEnabled;

namespace {
    using namespace fly;

#ifndef FLY_LIB_FLY_DIR
#define FLY_LIB_FLY_DIR "."
#endif

    // ── Test sources ─────────────────────────────────────────────────────────

    // Case 1: import fly  →  use fly.str.len()
    static constexpr const char *ImportFlySource = R"(
import fly

main() {
    string hello = "hello"
    int size = 0
    fly.str.len(hello, size)
}
)";

    // Case 2: import fly.*  →  use str.len()
    static constexpr const char *ImportFlyWildcardSource = R"(
import fly.*

main() {
    string hello = "hello"
    int size = 0
    str.len(hello, size)
}
)";

    // Case 3: import fly.str  →  use str.len()
    static constexpr const char *ImportFlyStrSource = R"(
import fly.str

main() {
    string hello = "hello"
    int size = 0
    str.len(hello, size)
}
)";

    // Case 4: import fly.str.*  →  use len() unqualified
    static constexpr const char *ImportFlyStrWildcardSource = R"(
import fly.str.*

main() {
    string hello = "hello"
    int size = 0
    len(hello, size)
}
)";

    // Case 5: import fly.str as s  →  use s.len()
    static constexpr const char *ImportFlyStrAliasSource = R"(
import fly.str as s

main() {
    string hello = "hello"
    int size = 0
    s.len(hello, size)
}
)";

    // Case 6: import fly.str.* as s  →  compiler error (wildcard + alias forbidden)
    static constexpr const char *ImportFlyStrWildcardAliasSource = R"(
import fly.str.* as s

main() {
    string hello = "hello"
    int size = 0
    len(hello, size)
}
)";

    // ── Fixture ───────────────────────────────────────────────────────────────

    class ImportTest : public ::testing::Test {
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

        ~ImportTest() override {
            remove(mainfly);
            llvm::outs().flush();
        }
    };

    // ── Tests ─────────────────────────────────────────────────────────────────

    // import fly → fly.str.len() fully qualified
    TEST_F(ImportTest, ImportFly) {
        SetUpWithSource(ImportFlySource);
        const char *argv[] = {"fly", "-no-output", mainfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // import fly.* → str.len() (wildcard brings fly's children into scope)
    TEST_F(ImportTest, ImportFlyWildcard) {
        SetUpWithSource(ImportFlyWildcardSource);
        const char *argv[] = {"fly", "-no-output", mainfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // import fly.str → str.len() (last segment 'str' added to scope)
    TEST_F(ImportTest, ImportFlyStr) {
        SetUpWithSource(ImportFlyStrSource);
        const char *argv[] = {"fly", "-no-output", mainfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // import fly.str.* → len() unqualified (all fly.str symbols in scope)
    TEST_F(ImportTest, ImportFlyStrWildcard) {
        SetUpWithSource(ImportFlyStrWildcardSource);
        const char *argv[] = {"fly", "-no-output", mainfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // import fly.str as s → s.len()
    TEST_F(ImportTest, ImportFlyStrAlias) {
        SetUpWithSource(ImportFlyStrAliasSource);
        const char *argv[] = {"fly", "-no-output", mainfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // import fly.str.* as s → compiler error
    TEST_F(ImportTest, ImportFlyStrWildcardAlias) {
        SetUpWithSource(ImportFlyStrWildcardAliasSource);
        const char *argv[] = {"fly", "-no-output", mainfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_FALSE(drv.Execute());
    }

} // anonymous namespace
