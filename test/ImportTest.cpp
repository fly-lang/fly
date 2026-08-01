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
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "gtest/gtest.h"
#include <fstream>

extern bool DebugLog;

namespace {
    using namespace fly;

    // ── Library stub: namespace my.utils with a function and a class ──────────

    static constexpr const char *UtilsSource = R"(
namespace my.utils

public void foo() {}
public class MyUtil {}
)";

    // ── Test sources ─────────────────────────────────────────────────────────

    // Case 1: import my  →  use my.utils.foo() fully qualified
    static constexpr const char *ImportMySource = R"(
import my

void main() {
    my.utils.foo()
}
)";

    // Case 2: import my.*  →  use utils.foo() (wildcard brings my's children into scope)
    static constexpr const char *ImportMyWildcardSource = R"(
import my.*

void main() {
    utils.foo()
}
)";

    // Case 3: import my.utils  →  use utils.foo() (last segment 'utils' in scope)
    static constexpr const char *ImportMyUtilsSource = R"(
import my.utils

void main() {
    utils.foo()
}
)";

    // Case 4: import my.utils.*  →  use foo() unqualified (all symbols in scope)
    static constexpr const char *ImportMyUtilsWildcardSource = R"(
import my.utils.*

void main() {
    foo()
}
)";

    // Case 5: import my.utils as u  →  use u.foo()
    static constexpr const char *ImportMyUtilsAliasSource = R"(
import my.utils as u

void main() {
    u.foo()
}
)";

    // Case 6: import my.utils.* as u  →  compiler error (wildcard + alias forbidden)
    static constexpr const char *ImportMyUtilsWildcardAliasSource = R"(
import my.utils.* as u

void main() {
    foo()
}
)";

    // Case 7: import my.utils.MyUtil  →  Java-style class import → MyUtil in scope
    static constexpr const char *ImportClassSource = R"(
import my.utils.MyUtil

void main() {
    MyUtil u = new MyUtil()
}
)";

    // Case 8: import my.utils.foo.*  →  compiler error (wildcard on a function)
    static constexpr const char *ImportWildcardOnFunctionSource = R"(
import my.utils.foo.*

void main() {}
)";

    // Case 9: import my.utils.*  →  both MyUtil and foo in scope
    static constexpr const char *ImportUtilsWildcardAllSource = R"(
import my.utils.*

void main() {
    MyUtil u = new MyUtil()
    foo()
}
)";

    // Case 10: import my.utils.*  →  utils.foo() is an ERROR: a wildcard injects
    // the namespace's SYMBOLS, it never binds the 'utils' prefix (canonical
    // semantics: wildcard = bare calls only, plain import = prefix only).
    static constexpr const char *ImportWildcardPrefixSource = R"(
import my.utils.*

void main() {
    utils.foo()
}
)";

    // Case 11: import my.utils  →  bare foo() is an ERROR: a plain import binds
    // only the 'utils' namespace symbol, no member enters the scope.
    static constexpr const char *ImportPlainBareSource = R"(
import my.utils

void main() {
    foo()
}
)";

    // ── Fixture ───────────────────────────────────────────────────────────────

    class ImportTest : public ::testing::Test {
    public:
        // Directory mode: both sources live in a dedicated subdirectory; -no-output
        // is a non-linking stage, so the whole directory is the compilation unit.
        const char *srcDir = "import_src";

        void SetUpWithSource(const char *MainSrc) {
            DebugLog = false;
            llvm::sys::fs::create_directory(srcDir);
            { std::ofstream f(std::string(srcDir) + "/main.fly");  f << MainSrc; }
            { std::ofstream f(std::string(srcDir) + "/utils.fly"); f << UtilsSource; }
            llvm::InitializeAllTargetInfos();
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmParsers();
            llvm::InitializeAllAsmPrinters();
        }

        ~ImportTest() override {
            llvm::sys::fs::remove_directories(srcDir);
            llvm::outs().flush();
        }
    };

    // ── Tests ─────────────────────────────────────────────────────────────────

    // import my → my.utils.foo() fully qualified
    TEST_F(ImportTest, ImportMy) {
        SetUpWithSource(ImportMySource);
        const char *argv[] = {"fly", "-no-output", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // import my.* → utils.foo() (wildcard brings my's children into scope)
    TEST_F(ImportTest, ImportMyWildcard) {
        SetUpWithSource(ImportMyWildcardSource);
        const char *argv[] = {"fly", "-no-output", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // import my.utils → utils.foo() (last segment 'utils' added to scope)
    TEST_F(ImportTest, ImportMyUtils) {
        SetUpWithSource(ImportMyUtilsSource);
        const char *argv[] = {"fly", "-no-output", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // import my.utils.* → foo() unqualified (all my.utils symbols in scope)
    TEST_F(ImportTest, ImportMyUtilsWildcard) {
        SetUpWithSource(ImportMyUtilsWildcardSource);
        const char *argv[] = {"fly", "-no-output", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // import my.utils as u → u.foo()
    TEST_F(ImportTest, ImportMyUtilsAlias) {
        SetUpWithSource(ImportMyUtilsAliasSource);
        const char *argv[] = {"fly", "-no-output", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // import my.utils.* as u → compiler error
    TEST_F(ImportTest, ImportMyUtilsWildcardAlias) {
        SetUpWithSource(ImportMyUtilsWildcardAliasSource);
        const char *argv[] = {"fly", "-no-output", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_FALSE(drv.Execute());
    }

    // Java-style class import: import my.utils.MyUtil → new MyUtil()
    TEST_F(ImportTest, ImportClassJavaStyle) {
        SetUpWithSource(ImportClassSource);
        const char *argv[] = {"fly", "-no-output", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // Wildcard on a non-namespace (function) → compiler error
    TEST_F(ImportTest, ImportWildcardOnFunctionIsError) {
        SetUpWithSource(ImportWildcardOnFunctionSource);
        const char *argv[] = {"fly", "-no-output", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_FALSE(drv.Execute());
    }

    // Wildcard import brings both classes and functions into scope
    TEST_F(ImportTest, ImportWildcardBringsAllSymbols) {
        SetUpWithSource(ImportUtilsWildcardAllSource);
        const char *argv[] = {"fly", "-no-output", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // A wildcard import does NOT bind the namespace prefix → utils.foo() fails
    TEST_F(ImportTest, ImportWildcardDoesNotBindPrefix) {
        SetUpWithSource(ImportWildcardPrefixSource);
        const char *argv[] = {"fly", "-no-output", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_FALSE(drv.Execute());
    }

    // A plain namespace import brings no bare symbols → foo() fails
    TEST_F(ImportTest, ImportPlainDoesNotInjectSymbols) {
        SetUpWithSource(ImportPlainBareSource);
        const char *argv[] = {"fly", "-no-output", "--src-dir", srcDir};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_FALSE(drv.Execute());
    }

} // anonymous namespace
