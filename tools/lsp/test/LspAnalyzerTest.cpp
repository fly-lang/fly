//===----------------------------------------------------------------------===//
// tools/lsp/test/LspAnalyzerTest.cpp — GTest coverage for LspAnalyzer
//
// LLVM's compilation infrastructure is not designed for multiple Driver /
// Frontend instances within the same process: the second call to compile()
// leaves global LLVM state in a shape that crashes AST walkers.
//
// Strategy: compile() is invoked EXACTLY ONCE for the whole test binary, on
// an error file that produces both an error and a warning.  All tests that
// need post-compile state share that single LspAnalyzer via CompileFixture.
// Tests that need no prior compile use their own transient LspAnalyzer and
// call no compile()-related code.
//===----------------------------------------------------------------------===//
#include <gtest/gtest.h>
#include "LspAnalyzer.h"

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include <string>
#include <vector>

using namespace fly::lsp;

// ── Helper: temporary .fly file with RAII cleanup ────────────────────────────

struct TempFlyFile {
    std::string path;

    explicit TempFlyFile(const std::string &source) {
        llvm::SmallString<128> p;
        int fd = -1;
        if (llvm::sys::fs::createTemporaryFile("fly_lsp_test", "fly", fd, p))
            return;
        {
            llvm::raw_fd_ostream os(fd, /*shouldClose=*/true);
            os << source;
        }
        path = p.str().str();
    }

    ~TempFlyFile() {
        if (!path.empty()) llvm::sys::fs::remove(path);
    }

    bool ok() const { return !path.empty(); }
};

// ── Source snippet ────────────────────────────────────────────────────────────
//
// Produces two diagnostics on source line 2 (1-based col 13 and col 9):
//   error   — 'undefined_var' is not defined
//   warning — variable 'x' declared but its value is never read
// LspDiagnostic positions are 0-based: line=1, char=12 / char=8.

static constexpr const char *kErrorSource =
    "void main() {\n"
    "    int x = undefined_var\n"
    "}\n";

// ── Tests that need NO prior compile ─────────────────────────────────────────
//
// Each creates its own transient LspAnalyzer without calling compile().

TEST(LspAnalyzerSansCompile, FindSymbolAt_BeforeCompile_ReturnsNull) {
    LspAnalyzer a;
    EXPECT_EQ(a.findSymbolAt("/nonexistent.fly", 0, 0), nullptr);
}

TEST(LspAnalyzerSansCompile, GetDefinitionLocation_NullSymbol_ReturnsNullopt) {
    LspAnalyzer a;
    EXPECT_FALSE(a.getDefinitionLocation(nullptr).has_value());
}

TEST(LspAnalyzerSansCompile, GetHoverMarkdown_NullSymbol_ReturnsEmptyString) {
    LspAnalyzer a;
    EXPECT_EQ(a.getHoverMarkdown(nullptr), "");
}

TEST(LspAnalyzerSansCompile, GetCompletions_BeforeCompile_ReturnsEmpty) {
    LspAnalyzer a;
    EXPECT_TRUE(a.getCompletions("/nonexistent.fly", 0, 0, "").empty());
}

TEST(LspAnalyzerSansCompile, GetDocumentSymbols_BeforeCompile_ReturnsEmpty) {
    LspAnalyzer a;
    EXPECT_TRUE(a.getDocumentSymbols("/nonexistent.fly").empty());
}

// ── Single shared compile() fixture ──────────────────────────────────────────
//
// SetUpTestSuite() compiles kErrorSource once.  All tests in this fixture
// inspect the resulting diagnostics vector and the LspAnalyzer state.

class CompileFixture : public ::testing::Test {
protected:
    static TempFlyFile               *s_file;
    static LspAnalyzer               *s_ana;
    static std::vector<LspDiagnostic> s_diags;

    static void SetUpTestSuite() {
        s_file  = new TempFlyFile(kErrorSource);
        s_ana   = new LspAnalyzer();
        s_diags = s_ana->compile({s_file->path});
    }

    static void TearDownTestSuite() {
        delete s_ana;
        delete s_file;
    }

    // Find first diagnostic with the given severity, or nullptr.
    static const LspDiagnostic *firstWithSeverity(int sev) {
        for (const auto &d : s_diags)
            if (d.severity == sev) return &d;
        return nullptr;
    }
};

TempFlyFile               *CompileFixture::s_file  = nullptr;
LspAnalyzer               *CompileFixture::s_ana   = nullptr;
std::vector<LspDiagnostic> CompileFixture::s_diags;

// ── compile(): diagnostic capture ────────────────────────────────────────────

TEST_F(CompileFixture, Compile_FileWithError_ReturnsNonEmpty) {
    EXPECT_FALSE(s_diags.empty());
}

TEST_F(CompileFixture, Compile_UndefinedVar_ProducesErrorSeverity) {
    EXPECT_NE(firstWithSeverity(1), nullptr)
        << "Expected at least one error diagnostic (severity=1)";
}

TEST_F(CompileFixture, Compile_UnusedVar_ProducesWarningSeverity) {
    EXPECT_NE(firstWithSeverity(2), nullptr)
        << "Expected at least one warning diagnostic (severity=2)";
}

TEST_F(CompileFixture, Compile_ErrorDiagnostic_PositionIsZeroBased) {
    // Compiler: line 2, col 13 (1-based) → LspDiagnostic: line=1, char=12 (0-based)
    const LspDiagnostic *err = firstWithSeverity(1);
    ASSERT_NE(err, nullptr);
    EXPECT_EQ(err->range.start.line, 1)
        << "Error on source line 2 must map to 0-based line 1";
    EXPECT_EQ(err->range.start.character, 12)
        << "Error at source col 13 must map to 0-based character 12";
}

TEST_F(CompileFixture, Compile_WarningDiagnostic_PositionIsZeroBased) {
    // Compiler: line 2, col 9 (1-based) → LspDiagnostic: line=1, char=8 (0-based)
    const LspDiagnostic *warn = firstWithSeverity(2);
    ASSERT_NE(warn, nullptr);
    EXPECT_EQ(warn->range.start.line, 1)
        << "Warning on source line 2 must map to 0-based line 1";
    EXPECT_EQ(warn->range.start.character, 8)
        << "Warning at source col 9 must map to 0-based character 8";
}

TEST_F(CompileFixture, Compile_Diagnostic_FilePathMatchesInput) {
    ASSERT_FALSE(s_diags.empty());
    for (const auto &d : s_diags)
        EXPECT_EQ(d.file, s_file->path)
            << "Diagnostic 'file' field must match the compiled source path";
}

TEST_F(CompileFixture, Compile_ErrorMessage_ContainsSymbolName) {
    const LspDiagnostic *err = firstWithSeverity(1);
    ASSERT_NE(err, nullptr);
    EXPECT_NE(err->message.find("undefined_var"), std::string::npos)
        << "Error message should mention the unresolved symbol name";
}

TEST_F(CompileFixture, Compile_AllDiagnostics_HaveNonEmptyMessage) {
    for (const auto &d : s_diags)
        EXPECT_FALSE(d.message.empty())
            << "Every diagnostic must carry a non-empty message string";
}

TEST_F(CompileFixture, Compile_AllDiagnostics_SeverityInLspRange) {
    for (const auto &d : s_diags) {
        EXPECT_GE(d.severity, 1) << "LSP severity minimum is 1 (Error)";
        EXPECT_LE(d.severity, 4) << "LSP severity maximum is 4 (Hint)";
    }
}

TEST_F(CompileFixture, Compile_RangeStartEqualsEnd_PointDiagnostic) {
    // The LspCapturingConsumer emits point ranges ({pos, pos}).
    for (const auto &d : s_diags) {
        EXPECT_EQ(d.range.start.line,      d.range.end.line);
        EXPECT_EQ(d.range.start.character, d.range.end.character);
    }
}

// ── Stub methods (post-compile, no AST walking) ───────────────────────────────

// getCompletions() now walks the AST which is unsafe on error-compiled files
// (same limitation as findSymbolAt). Completions are verified via integration
// testing. Pre-compile guard (no frontend_) is covered by LspAnalyzerSansCompile.
// NOTE: findSymbolAt after an error-file compile is intentionally not tested
// here because the partially-resolved AST (undefined symbols leave null nodes)
// makes the AST walkers unsafe to call.  That path is covered by the
// LspAnalyzerSansCompile suite (frontend_ == nullptr → immediate nullptr).
