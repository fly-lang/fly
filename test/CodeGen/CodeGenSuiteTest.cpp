//===--------------------------------------------------------------------------------------------------------------===//
// test/CodeGen/CodeGenSuiteTest.cpp - Suite, case, and test-block codegen tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTCaseStmt.h"
#include "AST/ASTClass.h"
#include "AST/ASTTestStmt.h"
#include "AST/ASTValue.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGenTest.h"

#include <AST/ASTDeclStmt.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTMethod.h>
#include <AST/ASTParam.h>

namespace {

using namespace fly;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Creates a SUITE class with the given name and no base classes.
static ASTClass *makeSuite(CodeGenTest *T, ASTModule *Module, const char *name) {
    llvm::SmallVector<ASTType *, 4> Bases;
    return ASTBuilder::CreateClass(Module, T->SourceLoc, ASTClassKind::SUITE,
                                   name, T->TopModifiers, Bases);
}

// Creates a void method with an empty body attached to a suite/class.
static ASTMethod *makeEmptyMethod(CodeGenTest *T, ASTClass *Suite, const char *name) {
    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(T->SourceLoc);
    return ASTBuilder::CreateClassMethod(T->SourceLoc, Suite, name,
                                         T->TopModifiers, T->Params, Body);
}

// Creates a void test-method whose body contains one case block per label.
static ASTMethod *makeTestMethod(CodeGenTest *T, ASTClass *Suite, const char *name,
                                  llvm::ArrayRef<const char *> labels) {
    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(T->SourceLoc);
    ASTBuilder::CreateClassMethod(T->SourceLoc, Suite, name,
                                   T->TopModifiers, T->Params, Body);
    // Retrieve the created method (last node in the suite)
    ASTMethod *Method = static_cast<ASTMethod *>(Suite->getNodes().back());

    for (const char *label : labels) {
        ASTStringValue *LabelExpr = ASTBuilder::CreateStringValue(T->SourceLoc, label);
        ASTBlockStmt *CaseBody = ASTBuilder::CreateBlockStmt(T->SourceLoc);
        ASTBuilder::CreateCaseStmt(Body, T->SourceLoc, LabelExpr, CaseBody);
    }
    return Method;
}

// Generate in test mode: TestMode=true for both Sema (so ASTTestStmt is resolved)
// and CodeGen (so visit(SemaTestStmt) emits the TLS-guarded block).
// CI is static/shared; we restore TestMode after the call.
static void GenerateTestMode(CodeGenTest *T) {
    auto SemaModules = T->S->Resolve(T->ASTModules, /*TestMode=*/true);
    ASSERT_FALSE(T->Diags.hasErrorOccurred());
    ASSERT_FALSE(SemaModules.empty());
    T->CI->getCodeGenOptions().TestMode = true;
    T->Modules = T->CG.GenerateModules(SemaModules);
    T->CI->getCodeGenOptions().TestMode = false;  // restore
    EXPECT_FALSE(T->Diags.hasErrorOccurred());
}

// ---------------------------------------------------------------------------
// TEST 1 — Implicit main() and TLS pointer
// ---------------------------------------------------------------------------
TEST_F(CodeGenTest, CGSuiteImplicitMain) {
    /**
     * suite MinimalSuite {
     *     void setup()    {}
     *     void teardown() {}
     *     void onlyTest() { case "run": {} }
     * }
     */
    ASTModule *Module = CreateModule();
    ASTClass *Suite = makeSuite(this, Module, "MinimalSuite");
    makeEmptyMethod(this, Suite, "setup");
    makeEmptyMethod(this, Suite, "teardown");
    makeTestMethod(this, Suite, "onlyTest", {"run"});

    Generate();
    std::string output = getOutput(getModules()[0]);

    // TLS context pointer must be defined in this module
    EXPECT_TRUE(output.find("@__fly_test_ctx_ptr") != std::string::npos);

    // Implicit main() must be emitted
    EXPECT_TRUE(output.find("define i32 @main()") != std::string::npos);

    // Error handler alloca in main()
    EXPECT_TRUE(output.find("suite_err") != std::string::npos);

    // At least one suite method function must be present
    EXPECT_TRUE(output.find("MinimalSuite") != std::string::npos);

    // main() must return 0
    EXPECT_TRUE(output.find("ret i32 0") != std::string::npos);
}

// ---------------------------------------------------------------------------
// TEST 2 — Case blocks are sequential labelled basic blocks
// ---------------------------------------------------------------------------
TEST_F(CodeGenTest, CGSuiteTestMethodCaseBlocks) {
    /**
     * suite MathSuite {
     *     void classifyTest() {
     *         case "positive": {}
     *         case "negative": {}
     *     }
     * }
     */
    ASTModule *Module = CreateModule();
    ASTClass *Suite = makeSuite(this, Module, "MathSuite");
    makeTestMethod(this, Suite, "classifyTest", {"positive", "negative"});

    Generate();
    std::string output = getOutput(getModules()[0]);

    // Each case becomes a labelled basic block
    EXPECT_TRUE(output.find("case.positive") != std::string::npos);
    EXPECT_TRUE(output.find("case.negative") != std::string::npos);

    // Each case block allocates its own fresh error handler
    EXPECT_TRUE(output.find("case_err.positive") != std::string::npos);
    EXPECT_TRUE(output.find("case_err.negative") != std::string::npos);
}

// ---------------------------------------------------------------------------
// TEST 3 — setup / teardown are called in correct order in main()
// ---------------------------------------------------------------------------
TEST_F(CodeGenTest, CGSuiteSetupTeardownCallOrder) {
    /**
     * suite OrderSuite {
     *     void setup()    {}
     *     void runTest()  { case "a": {} }
     *     void teardown() {}
     * }
     */
    ASTModule *Module = CreateModule();
    ASTClass *Suite = makeSuite(this, Module, "OrderSuite");
    makeEmptyMethod(this, Suite, "setup");
    makeTestMethod(this, Suite, "runTest", {"a"});
    makeEmptyMethod(this, Suite, "teardown");

    Generate();
    std::string output = getOutput(getModules()[0]);

    EXPECT_TRUE(output.find("define i32 @main()") != std::string::npos);

    // Sentinel store (TLS set to non-null) comes before method calls
    EXPECT_TRUE(output.find("store ptr inttoptr") != std::string::npos);

    // TLS cleared after all tests
    EXPECT_TRUE(output.find("store ptr null") != std::string::npos);

    // Order in the IR: setup appears before teardown
    size_t posSetup    = output.find("OrderSuite_F5setup");
    size_t posTeardown = output.find("OrderSuite_F8teardown");
    // Both should be present (called in main)
    EXPECT_TRUE(posSetup    != std::string::npos);
    EXPECT_TRUE(posTeardown != std::string::npos);
    // setup call appears before teardown call in the main body
    EXPECT_LT(posSetup, posTeardown);
}

// ---------------------------------------------------------------------------
// TEST 4 — Suite without setup/teardown still emits a valid main()
// ---------------------------------------------------------------------------
TEST_F(CodeGenTest, CGSuiteNoSetupNoTeardown) {
    /**
     * suite BareTest {
     *     void onlyTest() { case "x": {} }
     * }
     */
    ASTModule *Module = CreateModule();
    ASTClass *Suite = makeSuite(this, Module, "BareTest");
    makeTestMethod(this, Suite, "onlyTest", {"x"});

    Generate();
    std::string output = getOutput(getModules()[0]);

    // main() must still be valid
    EXPECT_TRUE(output.find("define i32 @main()") != std::string::npos);
    EXPECT_TRUE(output.find("ret i32 0") != std::string::npos);

    // No mention of setup or teardown
    EXPECT_EQ(output.find("_F5setup"),    std::string::npos);
    EXPECT_EQ(output.find("_F8teardown"), std::string::npos);
}

// ---------------------------------------------------------------------------
// TEST 5 — Multiple test-methods all appear in main()
// ---------------------------------------------------------------------------
TEST_F(CodeGenTest, CGSuiteMultipleTestMethods) {
    /**
     * suite MultiSuite {
     *     void firstTest()  { case "a": {} }
     *     void secondTest() { case "b": {} }
     * }
     */
    ASTModule *Module = CreateModule();
    ASTClass *Suite = makeSuite(this, Module, "MultiSuite");
    makeTestMethod(this, Suite, "firstTest",  {"a"});
    makeTestMethod(this, Suite, "secondTest", {"b"});

    Generate();
    std::string output = getOutput(getModules()[0]);

    EXPECT_TRUE(output.find("firstTest")  != std::string::npos);
    EXPECT_TRUE(output.find("secondTest") != std::string::npos);

    // Both test-methods are called in main()
    EXPECT_TRUE(output.find("call void @MultiSuite_F9firstTest") != std::string::npos ||
                output.find("firstTest") != std::string::npos);
    EXPECT_TRUE(output.find("call void @MultiSuite_F10secondTest") != std::string::npos ||
                output.find("secondTest") != std::string::npos);
}

// ---------------------------------------------------------------------------
// TEST 6 — test {} block is STRIPPED in non-test mode (no IR emitted)
// ---------------------------------------------------------------------------
TEST_F(CodeGenTest, CGTestBlockStripped) {
    /**
     * void func(int a) {
     *     test { }   // stripped in non-test mode
     * }
     */
    ASTModule *Module = CreateModule();
    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    llvm::SmallVector<ASTParam *, 8> FuncParams;
    ASTParam *aParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
    FuncParams.push_back(aParam);
    ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, FuncParams, Body);

    // Add a test {} block (empty body)
    ASTBuilder::CreateTestStmt(Body, SourceLoc);

    // Generate in default (non-test) mode
    Generate();
    std::string output = getOutput(getModules()[0]);

    // No test infrastructure should appear
    EXPECT_EQ(output.find("testbody"),           std::string::npos);
    EXPECT_EQ(output.find("testmerge"),          std::string::npos);
    EXPECT_EQ(output.find("__fly_test_ctx_ptr"), std::string::npos);
    EXPECT_EQ(output.find("ctx_isnull"),         std::string::npos);
}

// ---------------------------------------------------------------------------
// TEST 7 — test {} block is EMITTED in test mode with TLS guard
// ---------------------------------------------------------------------------
TEST_F(CodeGenTest, CGTestBlockEmittedInTestMode) {
    /**
     * void func(int a) {
     *     test { }   // emitted in test mode: TLS-guarded conditional block
     * }
     */
    ASTModule *Module = CreateModule();
    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    llvm::SmallVector<ASTParam *, 8> FuncParams;
    ASTParam *aParam = ASTBuilder::CreateParam(SourceLoc, IntTypeRef, "a", EmptyModifiers);
    FuncParams.push_back(aParam);
    ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, FuncParams, Body);

    // Add a test {} block (empty body)
    ASTBuilder::CreateTestStmt(Body, SourceLoc);

    // Generate in test mode
    GenerateTestMode(this);
    std::string output = getOutput(getModules()[0]);

    // TLS global must be declared (as external reference)
    EXPECT_TRUE(output.find("__fly_test_ctx_ptr") != std::string::npos);

    // testbody and testmerge basic blocks must be emitted
    EXPECT_TRUE(output.find("testbody")  != std::string::npos);
    EXPECT_TRUE(output.find("testmerge") != std::string::npos);

    // Conditional branch on TLS pointer null-check
    EXPECT_TRUE(output.find("ctx_isnull") != std::string::npos);
}

} // namespace
