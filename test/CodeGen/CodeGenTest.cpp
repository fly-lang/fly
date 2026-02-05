//===--------------------------------------------------------------------------------------------------------------===//
// test/CodeGenTest.cpp - Code Generation Test Implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGenTest.h"
#include "llvm/Support/MemoryBuffer.h"

using namespace fly;

// Define the static member variables
std::shared_ptr<CompilerInstance> CodeGenTest::CI;

// Static setup for the entire test suite
void CodeGenTest::SetUpTestCase() {
    DebugEnabled = false;
    CI = TestUtils::CreateCompilerInstance();
    // Initialize LLVM targets once for the entire test suite
    // These are global initializations and are idempotent (safe to call multiple times)
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();
}

// Static teardown for the entire test suite
void CodeGenTest::TearDownTestCase() {
    DebugEnabled = false;
}

void CodeGenTest::SetUp() {

}
void CodeGenTest::TearDown() {
	Test::TearDown();
}

// Constructor
CodeGenTest::CodeGenTest() :
    CG(TestUtils::CreateCodeGen(*CI, LLVMCtx)),  // Pass context reference
    Diags(CI->getDiagnostics()),
    Builder(new ASTBuilder(Diags)),
    S(new SemaContext(CI->getDiagnostics())),
    VoidTypeRef(ASTBuilder::CreateVoidType(SourceLoc)),
    BoolTypeRef(ASTBuilder::CreateBoolType(SourceLoc)),
    ByteTypeRef(ASTBuilder::CreateByteType(SourceLoc)),
    ShortTypeRef(ASTBuilder::CreateShortType(SourceLoc)),
    UShortTypeRef(ASTBuilder::CreateUShortType(SourceLoc)),
    IntTypeRef(ASTBuilder::CreateIntType(SourceLoc)),
    UIntTypeRef(ASTBuilder::CreateUIntType(SourceLoc)),
    LongTypeRef(ASTBuilder::CreateLongType(SourceLoc)),
    ULongTypeRef(ASTBuilder::CreateULongType(SourceLoc)),
    FloatTypeRef(ASTBuilder::CreateFloatType(SourceLoc)),
    DoubleTypeRef(ASTBuilder::CreateDoubleType(SourceLoc)),
    ErrorTypeRef(ASTBuilder::CreateErrorType(SourceLoc)),
    StringTypeRef(ASTBuilder::CreateStringType(SourceLoc)) {
    // LLVM initialization moved to SetUpTestCase()
}

// Destructor
CodeGenTest::~CodeGenTest() {
    llvm::outs().flush();
    // Clean up modules - we own them after GenerateModules transfers ownership
    // Must delete modules BEFORE deleting LLVMContext
    for (auto *M : Modules) {
        delete M;
    }
    Modules.clear();
    ASTModules.clear();
    delete Builder;
    delete S;
}

// Create a new AST module for testing
ASTModule *CodeGenTest::CreateModule(std::string Name) {
    Diags.getClient()->BeginSourceFile();
    auto Buffer = llvm::MemoryBuffer::getMemBuffer("", Name);
    auto FID = new InputFile(Diags, CI->getSourceManager(), Name);
    auto AST = Builder->CreateModule(FID);
    Diags.getClient()->EndSourceFile();
    ASTModules.push_back(AST);
    return AST;
}

// Get the AST builder
ASTBuilder &CodeGenTest::getASTBuilder() {
    return *Builder;
}

// Create type from class
ASTType *CodeGenTest::CreateType(ASTClass *Class) {
    llvm::SmallVector<ASTName *, 4> Names;
    Names.push_back(Builder->CreateName(Class->getName(), Class->getLocation()));
    return Builder->CreateType(Class->getLocation(), Names);
}

// Create type from enum
ASTType *CodeGenTest::CreateType(ASTEnum *Enum) {
    llvm::SmallVector<ASTName *, 4> Names;
    Names.push_back(Builder->CreateName(Enum->getName(), Enum->getLocation()));
    return Builder->CreateType(Enum->getLocation(), Names);
}

// Generate code from AST modules
bool CodeGenTest::Resolve() {
	// validate and resolve
	SmallVector<SemaModule *, 8> SemaModules = S->Resolve(ASTModules);
	return !Diags.hasErrorOccurred();
}

// Generate code from AST modules
void CodeGenTest::Generate() {
    // validate and resolve
    SmallVector<SemaModule *, 8> SemaModules = S->Resolve(ASTModules);
    ASSERT_FALSE(Diags.hasErrorOccurred());
    ASSERT_FALSE(SemaModules.empty());
    Modules = CG.GenerateModules(SemaModules);
    EXPECT_FALSE(Diags.hasErrorOccurred());
}

// Get generated modules
llvm::SmallVector<llvm::Module *, 8> &CodeGenTest::getModules() {
    return Modules;
}

// Get LLVM IR output from a module
std::string CodeGenTest::getOutput(llvm::Module *M) {
    testing::internal::CaptureStdout();
    verifyModule(*M);
    M->print(llvm::outs(), nullptr);
    std::string out = testing::internal::GetCapturedStdout();
    out.erase(0, out.find("\n") + 1); // skip ;ModuleID
    out.erase(0, out.find("\n") + 1); // skip source_filename
    out.erase(0, out.find("\n") + 1); // skip target datalayout
    out.erase(0, out.find("\n") + 1); // skip target triple
    return out;
}

// Get LLVM IR output from function list
std::string CodeGenTest::getOutput(SymbolTableList<Function> &Functions) {
    testing::internal::CaptureStdout();
    for (auto &F : Functions) {
        verifyFunction(F);
        F.print(llvm::outs());
    }
    return testing::internal::GetCapturedStdout();
}

