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

extern bool DebugLog;

namespace {
    using namespace fly;

    // ── Library stub: namespace my.utils with one public function ─────────────

    static constexpr const char *UtilsSource = R"(
namespace my.utils

public foo() {}
)";

    // ── Test sources ─────────────────────────────────────────────────────────

    // Case 1: import my  →  use my.utils.foo() fully qualified
    static constexpr const char *ImportMySource = R"(
import my

main() {
    my.utils.foo()
}
)";

    // Case 2: import my.*  →  use utils.foo() (wildcard brings my's children into scope)
    static constexpr const char *ImportMyWildcardSource = R"(
import my.*

main() {
    utils.foo()
}
)";

    // Case 3: import my.utils  →  use utils.foo() (last segment 'utils' in scope)
    static constexpr const char *ImportMyUtilsSource = R"(
import my.utils

main() {
    utils.foo()
}
)";

    // Case 4: import my.utils.*  →  use foo() unqualified (all symbols in scope)
    static constexpr const char *ImportMyUtilsWildcardSource = R"(
import my.utils.*

main() {
    foo()
}
)";

    // Case 5: import my.utils as u  →  use u.foo()
    static constexpr const char *ImportMyUtilsAliasSource = R"(
import my.utils as u

main() {
    u.foo()
}
)";

    // Case 6: import my.utils.* as u  →  compiler error (wildcard + alias forbidden)
    static constexpr const char *ImportMyUtilsWildcardAliasSource = R"(
import my.utils.* as u

main() {
    foo()
}
)";

    // ── Fixture ───────────────────────────────────────────────────────────────

    class ImportTest : public ::testing::Test {
    public:
        const char *mainfly  = "main.fly";
        const char *utilsfly = "utils.fly";

        void SetUpWithSource(const char *MainSrc) {
            DebugLog = false;
            { std::ofstream f(mainfly);  f << MainSrc; }
            { std::ofstream f(utilsfly); f << UtilsSource; }
            llvm::InitializeAllTargetInfos();
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmParsers();
            llvm::InitializeAllAsmPrinters();
        }

        ~ImportTest() override {
            remove(mainfly);
            remove(utilsfly);
            llvm::outs().flush();
        }
    };

    // ── Tests ─────────────────────────────────────────────────────────────────

    // import my → my.utils.foo() fully qualified
    TEST_F(ImportTest, ImportMy) {
        SetUpWithSource(ImportMySource);
        const char *argv[] = {"fly", "-no-output", mainfly, utilsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // import my.* → utils.foo() (wildcard brings my's children into scope)
    TEST_F(ImportTest, ImportMyWildcard) {
        SetUpWithSource(ImportMyWildcardSource);
        const char *argv[] = {"fly", "-no-output", mainfly, utilsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // import my.utils → utils.foo() (last segment 'utils' added to scope)
    TEST_F(ImportTest, ImportMyUtils) {
        SetUpWithSource(ImportMyUtilsSource);
        const char *argv[] = {"fly", "-no-output", mainfly, utilsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // import my.utils.* → foo() unqualified (all my.utils symbols in scope)
    TEST_F(ImportTest, ImportMyUtilsWildcard) {
        SetUpWithSource(ImportMyUtilsWildcardSource);
        const char *argv[] = {"fly", "-no-output", mainfly, utilsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // import my.utils as u → u.foo()
    TEST_F(ImportTest, ImportMyUtilsAlias) {
        SetUpWithSource(ImportMyUtilsAliasSource);
        const char *argv[] = {"fly", "-no-output", mainfly, utilsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // import my.utils.* as u → compiler error
    TEST_F(ImportTest, ImportMyUtilsWildcardAlias) {
        SetUpWithSource(ImportMyUtilsWildcardAliasSource);
        const char *argv[] = {"fly", "-no-output", mainfly, utilsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_FALSE(drv.Execute());
    }

} // anonymous namespace
