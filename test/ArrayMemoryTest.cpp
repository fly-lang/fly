//===--------------------------------------------------------------------------------------------------------------===//
// test/ArrayMemoryTest.cpp - array memory semantics, checked by RUNNING the program
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// Arrays are heap-backed and REFERENCE COUNTED (docs §6.6). Almost nothing about
// that model is visible in the AST or in a compile-only test: whether a buffer is
// shared or copied, released once or twice, alive or dangling, only shows up when
// the program RUNS. So these tests compile a small program to a real executable
// and read its exit code, which the program sets with `fail <value>`.
//
// What lives HERE and not in ExecArraySuite.fly: the suites are a shared gate,
// compiled by BOTH the reference and the self-host, and the self-host parses
// neither the subscript (`xs[i]`) nor an array of a class type. Everything that
// needs either of those is reference-only until the port lands, and this is its
// home. Behaviour both compilers accept belongs in the suite instead.
//
// A double release shows up as a crash or a garbage exit code, never as the
// expected value, so an exact exit-code match is a real assertion about lifetime.
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Driver/Driver.h"
#include "Frontend/CompilerInstance.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/TargetSelect.h"
#include "gtest/gtest.h"

#include <fstream>
#include <string>

extern bool DebugLog;

namespace {
    using namespace fly;

    class ArrayMemoryTest : public ::testing::Test {
    public:
        ArrayMemoryTest() {
            DebugLog = false;
            llvm::InitializeAllTargetInfos();
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmParsers();
            llvm::InitializeAllAsmPrinters();
        }

        ~ArrayMemoryTest() override {
            for (const auto &D : CleanupDirs)
                llvm::sys::fs::remove_directories(D);
            llvm::outs().flush();
        }

        // Compile Src into its own directory and RUN it, returning the exit code.
        // Returns -1000 when the program could not be built (kept distinct from any
        // exit code a test expects, so a build failure never reads as a wrong value).
        int buildAndRun(const std::string &Name, const std::string &Src) {
            const std::string Dir = "arrmem_" + Name;
            llvm::sys::fs::remove_directories(Dir);
            llvm::sys::fs::create_directory(Dir);
            CleanupDirs.push_back(Dir);

            { std::ofstream f(Dir + "/main.fly"); f << Src; }

            const std::string Exe = Dir + "/" + Name + ExeExt;
            const char *argv[] = {"fly", "--src-dir", Dir.c_str(), "-o", Name.c_str(),
                                  "--out-dir", Dir.c_str()};
            {
                Driver drv(argv);
                drv.BuildCompilerInstance();
                if (!drv.Execute())
                    return -1000;
            }
            if (!llvm::sys::fs::exists(Exe))
                return -1000;

            std::string Err;
            llvm::SmallVector<llvm::StringRef, 1> Args{Exe};
            return llvm::sys::ExecuteAndWait(Exe, Args, std::nullopt, {}, 0, 0, &Err);
        }

#ifdef _WIN32
        static constexpr const char *ExeExt = ".exe";
#else
        static constexpr const char *ExeExt = "";
#endif

        std::vector<std::string> CleanupDirs;
    };

    // ── Reference semantics ────────────────────────────────────────────────────

    // Binding an array shares its buffer instead of copying it: a write through one
    // name is visible through the other. This is the rule that separates arrays from
    // every other type in the language, so it gets its own test.
    TEST_F(ArrayMemoryTest, BindingSharesTheBuffer) {
        EXPECT_EQ(buildAndRun("share",
                  "void main() {\n"
                  "    int[] k = {1, 2, 3}\n"
                  "    int[] j = k\n"
                  "    j[0] = 9\n"
                  "    fail k[0] + k[1] + k[2] + 100\n"   // 9 + 2 + 3 + 100
                  "}\n"), 114);
    }

    // …and both names are OWNERS. The buffer must be released exactly once: too few
    // releases leak (invisible here), too many corrupt the heap (visible as a crash
    // or a garbage code). The inner scope makes both owners die before the check.
    TEST_F(ArrayMemoryTest, TwoOwnersReleaseTheBufferOnce) {
        EXPECT_EQ(buildAndRun("twoowners",
                  "void main() {\n"
                  "    int total = 0\n"
                  "    int i = 0\n"
                  "    while i < 2000 {\n"
                  "        int[] k = {1, 2, 3}\n"
                  "        int[] j = k\n"
                  "        j[2] = 7\n"
                  "        total = k[2]\n"
                  "    	   i = i + 1\n"
                  "    }\n"
                  "    fail total + 100\n"
                  "}\n"), 107);
    }

    // Self-assignment must retain before it releases, or the count touches zero and
    // frees a buffer that is about to be stored straight back.
    TEST_F(ArrayMemoryTest, SelfAssignmentKeepsTheBuffer) {
        EXPECT_EQ(buildAndRun("selfassign",
                  "void main() {\n"
                  "    int[] k = {4, 5, 6}\n"
                  "    k = k\n"
                  "    k = k\n"
                  "    fail k[0] + k[1] + k[2] + 100\n"
                  "}\n"), 115);
    }

    // Reassignment releases the buffer the destination held. Run it enough times
    // that a missing release would be a runaway leak rather than a rounding error.
    TEST_F(ArrayMemoryTest, ReassignmentReleasesThePreviousBuffer) {
        EXPECT_EQ(buildAndRun("reassign",
                  "void main() {\n"
                  "    int[] k = {0, 0, 0}\n"
                  "    int i = 0\n"
                  "    while i < 200000 {\n"
                  "        k = {i, i, i}\n"
                  "        i = i + 1\n"
                  "    }\n"
                  "    fail k[0] - 199899\n"              // 199999 - 199899
                  "}\n"), 100);
    }

    // ── Ownership across a call ────────────────────────────────────────────────

    // Returning through `out` hands the caller the only reference: the callee
    // retains on its behalf, then its own local releases. Retaining nowhere frees
    // the buffer before the caller reads it; retaining twice strands it forever.
    TEST_F(ArrayMemoryTest, ArrayReturnedThroughOutSurvivesTheCallee) {
        EXPECT_EQ(buildAndRun("outreturn",
                  "int[] make(const int n) {\n"
                  "    int[] xs = {n, n + 1, n + 2}\n"
                  "    out = xs\n"
                  "}\n"
                  "\n"
                  "void main() {\n"
                  "    int total = 0\n"
                  "    int i = 0\n"
                  "    while i < 2000 {\n"
                  "        int[] r = make(10)\n"
                  "        total = r[0] + r[1] + r[2]\n"   // 33
                  "        i = i + 1\n"
                  "    }\n"
                  "    fail total + 100\n"
                  "}\n"), 133);
    }

    // An argument is a BORROW: the callee neither retains nor releases it, so the
    // caller's array is still whole after the call. A stray release in the callee
    // would free it while the caller still holds the only reference.
    TEST_F(ArrayMemoryTest, ArrayArgumentIsBorrowed) {
        EXPECT_EQ(buildAndRun("borrow",
                  "int sum(const int[] xs) {\n"
                  "    int t = 0\n"
                  "    for x in xs {\n"
                  "        t = t + x\n"
                  "    }\n"
                  "    out = t\n"
                  "}\n"
                  "\n"
                  "void main() {\n"
                  "    int[] k = {1, 2, 3}\n"
                  "    int a = sum(k)\n"
                  "    int b = sum(k)\n"                   // still usable
                  "    fail a + b + k[0] + 100\n"          // 6 + 6 + 1 + 100
                  "}\n"), 113);
    }

    // ── Nested arrays ──────────────────────────────────────────────────────────

    // One buffer and one count PER LEVEL. Releasing the outer array walks its
    // elements first — the inner buffers used to be abandoned entirely.
    TEST_F(ArrayMemoryTest, NestedArrayReleasesEveryLevel) {
        EXPECT_EQ(buildAndRun("nested",
                  "void main() {\n"
                  "    int last = 0\n"
                  "    int i = 0\n"
                  "    while i < 100000 {\n"
                  "        int[][] rows = {{1}, {2, 3}, {4, 5, 6}}\n"
                  "        last = rows[2][2]\n"
                  "        i = i + 1\n"
                  "    }\n"
                  "    fail last + 100\n"
                  "}\n"), 106);
    }

    // Two owners of a NESTED array: the inner buffers have a single holder each —
    // the outer buffer — so they must be released once, when the outer count
    // reaches zero, and not on every scope exit.
    TEST_F(ArrayMemoryTest, NestedArrayWithTwoOwnersIsReleasedOnce) {
        EXPECT_EQ(buildAndRun("nestedshare",
                  "void main() {\n"
                  "    int last = 0\n"
                  "    int i = 0\n"
                  "    while i < 100000 {\n"
                  "        int[][] a = {{1, 2}, {3, 4, 5}}\n"
                  "        int[][] b = a\n"
                  "        last = b[1][2]\n"
                  "        i = i + 1\n"
                  "    }\n"
                  "    fail last + 100\n"
                  "}\n"), 105);
    }

    // A ROW of a nested array can be bound to a name of its own. It is an ordinary
    // array binding: the row's buffer is retained, so it stays alive as long as the
    // name does, and writing through it is visible from the parent.
    TEST_F(ArrayMemoryTest, NestedRowCanBeBoundToItsOwnName) {
        EXPECT_EQ(buildAndRun("nestedrow",
                  "void main() {\n"
                  "    int[][] rows = {{1, 2}, {3, 4, 5}}\n"
                  "    int[] row = rows[1]\n"
                  "    row[0] = 9\n"
                  "    int sum = 0\n"
                  "    for v in row {\n"
                  "        sum = sum + v\n"
                  "    }\n"
                  "    fail sum + rows[1][0] + 73\n"       // (9+4+5) + 9
                  "}\n"), 100);
    }

    // ── Empty arrays ───────────────────────────────────────────────────────────

    // An empty array has NO buffer: its data pointer is null. Declaring, binding
    // and releasing one must all be legal — the null guards in the retain and the
    // release exist for exactly this, and without them each would dereference null.
    TEST_F(ArrayMemoryTest, EmptyArrayBindsAndReleases) {
        EXPECT_EQ(buildAndRun("empty",
                  "void main() {\n"
                  "    int count = 0\n"
                  "    int i = 0\n"
                  "    while i < 2000 {\n"
                  "        int[0] z\n"
                  "        int[] y = z\n"
                  "        y = z\n"
                  "        for v in y {\n"
                  "            count = count + 1\n"
                  "        }\n"
                  "        i = i + 1\n"
                  "    }\n"
                  "    fail count + 100\n"                 // never iterates
                  "}\n"), 100);
    }

    // ── Subscript ──────────────────────────────────────────────────────────────

    // Read and write with computed indices, at both ends of the range.
    TEST_F(ArrayMemoryTest, SubscriptReadsAndWrites) {
        EXPECT_EQ(buildAndRun("subscript",
                  "void main() {\n"
                  "    int[5] k\n"
                  "    k[0] = 1\n"
                  "    k[4] = 2\n"
                  "    int i = 2\n"
                  "    k[i + 1] = 4\n"
                  "    fail k[0] + k[4] + k[3] + k[1] + 100\n"   // 1 + 2 + 4 + 0
                  "}\n"), 107);
    }

    // The bounds check is TWO-SIDED and covers reads as well as writes. Each of the
    // four combinations is caught by `handle`; testing only the upper bound would
    // let a negative index read before the buffer.
    TEST_F(ArrayMemoryTest, SubscriptOutOfRangeIsCaught) {
        EXPECT_EQ(buildAndRun("bounds",
                  "void main() {\n"
                  "    int[] k = {5, 6, 7}\n"
                  "    int hits = 0\n"
                  "\n"
                  "    handle {\n"
                  "        int a = k[3]\n"
                  "    }\n"
                  "    bool c1 = error\n"
                  "    if c1 == true {\n"
                  "        hits = hits + 1\n"
                  "    }\n"
                  "    handle {\n"
                  "        int b = k[-1]\n"
                  "    }\n"
                  "    bool c2 = error\n"
                  "    if c2 == true {\n"
                  "        hits = hits + 1\n"
                  "    }\n"
                  "    handle {\n"
                  "        k[3] = 0\n"
                  "    }\n"
                  "    bool c3 = error\n"
                  "    if c3 == true {\n"
                  "        hits = hits + 1\n"
                  "    }\n"
                  "    handle {\n"
                  "        k[-1] = 0\n"
                  "    }\n"
                  "    bool c4 = error\n"
                  "    if c4 == true {\n"
                  "        hits = hits + 1\n"
                  "    }\n"
                  "    fail hits + 100\n"
                  "}\n"), 104);
    }

    // Uncaught, the failure carries the dedicated code the docs name: 2989 (0x0BAD).
    //
    // The value the PARENT observes is not the same on both systems, and that is the
    // operating system's doing, not the compiler's: POSIX passes only the low 8 bits
    // of the status through wait(), so 2989 arrives as 173. Windows keeps the full
    // 32-bit value. `main` returns the same number either way — this expectation
    // simply reads it back through whatever the platform preserves.
#ifdef _WIN32
        static constexpr int ExpectedBoundsExit = 2989;
#else
        static constexpr int ExpectedBoundsExit = 2989 & 0xFF;   // 173
#endif
    TEST_F(ArrayMemoryTest, SubscriptOutOfRangeExitsWithItsOwnCode) {
        EXPECT_EQ(buildAndRun("boundscode",
                  "void main() {\n"
                  "    int[] k = {5, 6, 7}\n"
                  "    int bad = k[9]\n"
                  "}\n"), ExpectedBoundsExit);
    }

    // A CAUGHT failure leaves the frame's locals alone: control resumes in the same
    // scope, where they are all still live. The cleanup used to run on this path, so
    // the array's buffer was freed and every later read returned garbage.
    TEST_F(ArrayMemoryTest, ArraySurvivesACaughtFailure) {
        EXPECT_EQ(buildAndRun("caughtfail",
                  "void main() {\n"
                  "    int[] k = {5, 6, 7}\n"
                  "    handle {\n"
                  "        int bad = k[9]\n"
                  "    }\n"
                  "    k[0] = 40\n"
                  "    int sum = 0\n"
                  "    for v in k {\n"
                  "        sum = sum + v\n"
                  "    }\n"
                  "    fail sum + 47\n"                    // 40 + 6 + 7
                  "}\n"), 100);
    }

    // ── Arrays of classes ──────────────────────────────────────────────────────

    // A sized array of a class is born full: one instance per index, built with the
    // no-argument constructor. They must be DISTINCT — a single instance repeated
    // would make a write to one element visible through all the others.
    TEST_F(ArrayMemoryTest, ClassArrayHasOneInstancePerIndex) {
        EXPECT_EQ(buildAndRun("clsarray",
                  "public class Cell {\n"
                  "    int v\n"
                  "    public Cell() { this.v = 1 }\n"
                  "    public void set(const int n) { this.v = n }\n"
                  "    public int get() { out = this.v }\n"
                  "}\n"
                  "\n"
                  "void main() {\n"
                  "    Cell[4] cs\n"
                  "    Cell a = cs[0]\n"
                  "    a.set(5)\n"
                  "    Cell d = cs[3]\n"
                  "    d.set(9)\n"
                  "    int sum = 0\n"
                  "    for c in cs {\n"
                  "        sum = sum + c.get()\n"
                  "    }\n"
                  "    fail sum + 84\n"                    // 5 + 1 + 1 + 9
                  "}\n"), 100);
    }

    // The array owns its BUFFER and nothing else: releasing it drops the pointers
    // and never the objects. An instance taken out of the array is still usable
    // after the array has gone.
    TEST_F(ArrayMemoryTest, ClassInstancesOutliveTheArray) {
        EXPECT_EQ(buildAndRun("clsoutlive",
                  "public class Cell {\n"
                  "    int v\n"
                  "    public Cell() { this.v = 3 }\n"
                  "    public int get() { out = this.v }\n"
                  "}\n"
                  "\n"
                  "int keepOne() {\n"
                  "    Cell kept\n"
                  "    {\n"
                  "        Cell[2] cs\n"
                  "        kept = cs[1]\n"
                  "    }\n"                                // the buffer is released here
                  "    out = kept.get()\n"
                  "}\n"
                  "\n"
                  "void main() {\n"
                  "    fail keepOne() + 100\n"
                  "}\n"), 103);
    }

    // A literal of objects stores POINTERS, so the buffer strides by pointer width.
    // Sizing it by the struct would place every element after the first out of reach.
    TEST_F(ArrayMemoryTest, ClassArrayLiteralStoresReferences) {
        EXPECT_EQ(buildAndRun("clsliteral",
                  "public class Box {\n"
                  "    int v\n"
                  "    public Box() { this.v = 0 }\n"
                  "    public void set(const int n) { this.v = n }\n"
                  "    public int get() { out = this.v }\n"
                  "}\n"
                  "\n"
                  "void main() {\n"
                  "    Box p = new Box()\n"
                  "    p.set(4)\n"
                  "    Box q = new Box()\n"
                  "    q.set(6)\n"
                  "    Box[] bs = {p, q}\n"
                  "    int sum = 0\n"
                  "    for b in bs {\n"
                  "        sum = sum + b.get()\n"
                  "    }\n"
                  "    p.set(90)\n"                        // the array holds p itself…
                  "    Box first = bs[0]\n"
                  "    fail sum + first.get()\n"           // …so this sees 90
                  "}\n"), 100);
    }

    // ── Borrowed elements ──────────────────────────────────────────────────────

    // A STRUCT element sits inline in the buffer and is copied out by value, the
    // same semantics as any other struct binding. Reading one used to abort the
    // backend outright.
    TEST_F(ArrayMemoryTest, StructElementIsCopiedOut) {
        EXPECT_EQ(buildAndRun("structelem",
                  "struct Pt { int x  int y }\n"
                  "\n"
                  "void main() {\n"
                  "    Pt[2] ps\n"
                  "    Pt a = ps[0]\n"
                  "    a.x = 4\n"
                  "    a.y = 5\n"
                  "    Pt b = ps[0]\n"                     // an independent copy
                  "    fail a.x + a.y + b.x + b.y + 91\n"  // 4 + 5 + 0 + 0
                  "}\n"), 100);
    }

    // A STRING element is a borrow: the array frees nothing per element, so reading
    // one into an owned slot must CLONE it. Taking it as-is made the local free a
    // buffer the array still pointed at. The loop makes a leaked clone obvious too.
    TEST_F(ArrayMemoryTest, StringElementIsClonedOnRead) {
        EXPECT_EQ(buildAndRun("strelem",
                  "import fly.str\n"
                  "\n"
                  "void main() {\n"
                  "    string[] xs = {\"alpha\", \"beta\"}\n"
                  "    int n = 0\n"
                  "    int i = 0\n"
                  "    while i < 100000 {\n"
                  "        string s = xs[0]\n"
                  "        n = str.len(s)\n"
                  "        i = i + 1\n"
                  "    }\n"
                  "    string t = xs[1]\n"
                  "    fail n + str.len(t) + 91\n"         // 5 + 4
                  "}\n"), 100);
    }
} // anonymous namespace
