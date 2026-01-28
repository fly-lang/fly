//===--------------------------------------------------------------------------------------------------------------===//
// test/CodeGenTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGENTEST_H
#define FLY_CODEGENTEST_H

// fly
#include "../TestUtils.h"
#include "AST/ASTCall.h"
#include "AST/ASTExpr.h"
#include "Parser/Parser.h"
#include "Sema/SemaContext.h"

#include <AST/ASTBuilder.h>
#include <AST/ASTModifier.h>
#include <Basic/Debug.h>

// third party
#include "AST/ASTClass.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"

#include <AST/ASTEnum.h>
#include <CodeGen/CodeGenModule.h>
#include <Sema/SymbolTable.h>
#include <gtest/gtest.h>
#include <llvm/LTO/LTO.h>

using namespace fly;

// The test fixture.
class CodeGenTest : public ::testing::Test {

public:
    static std::shared_ptr<CompilerInstance> CI;
    llvm::LLVMContext LLVMCtx;  // Static context outlives all tests
    CodeGen *CG;
    DiagnosticsEngine &Diags;
	ASTBuilder *Builder;
    SemaContext *S;
	llvm::SmallVector<ASTModule *, 8> ASTModules;
	llvm::SmallVector<llvm::Module *, 8> Modules;
    SourceLocation SourceLoc;
    ASTType *VoidTypeRef;
    ASTType *BoolTypeRef;
    ASTType *ByteTypeRef;
    ASTType *ShortTypeRef;
    ASTType *UShortTypeRef;
    ASTType *IntTypeRef;
    ASTType *UIntTypeRef;
    ASTType *LongTypeRef;
    ASTType *ULongTypeRef;
    ASTType *FloatTypeRef;
    ASTType *DoubleTypeRef;
    ASTType *ErrorTypeRef;
	ASTType *StringTypeRef;
    llvm::SmallVector<ASTModifier *, 8> TopModifiers;
    llvm::SmallVector<ASTModifier *, 8> EmptyModifiers;
    llvm::SmallVector<ASTExpr *, 8> Args;
    llvm::SmallVector<ASTParam *, 8> Params;

    // Static setup/teardown methods
    static void SetUpTestCase();
    static void TearDownTestCase();

    // Per-test setup/teardown methods
    void SetUp() override;
    void TearDown() override;

    // Constructor and destructor
    CodeGenTest();
    virtual ~CodeGenTest();

    // Helper methods
    ASTModule *CreateModule(std::string Name = "test");
	ASTBuilder &getASTBuilder();
	ASTType *CreateType(ASTClass *Class);
	ASTType *CreateType(ASTEnum *Enum);
	void Generate();
	llvm::SmallVector<llvm::Module *, 8> &getModules();
	std::string getOutput(llvm::Module *M);
	std::string getOutput(SymbolTableList<Function> &Functions);
};

#endif //FLY_CODEGENTEST_H
