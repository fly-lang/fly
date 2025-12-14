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
#include "Parser/Parser.h"
#include "Sema/Sema.h"
#include "AST/ASTCall.h"
#include "AST/ASTFunction.h"
#include "AST/ASTExpr.h"
#include <AST/ASTModifier.h>
#include <AST/ASTVar.h>
#include <AST/ASTBuilder.h>
#include <Sema/SemaBuilder.h>
#include "Sema/SemaType.h"


// third party
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetSelect.h"

#include <AST/ASTEnum.h>
#include <AST/ASTIdentifier.h>
#include <CodeGen/CodeGenModule.h>
#include <Sema/SymbolTable.h>
#include <gtest/gtest.h>
#include <llvm/LTO/LTO.h>

using namespace fly;

// The test fixture.
class CodeGenTest : public ::testing::Test {

public:
    const CompilerInstance CI;
    CodeGen *CG;
    DiagnosticsEngine &Diags;
	ASTBuilder *Builder;
    Sema *S;
	llvm::SmallVector<ASTModule *, 8> ASTModules;
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
	ASTType *CharTypeRef;
    llvm::SmallVector<ASTModifier *, 8> TopModifiers;
    llvm::SmallVector<ASTModifier *, 8> EmptyModifiers;
    llvm::SmallVector<ASTExpr *, 8> Args;
    llvm::SmallVector<ASTParam *, 8> Params;

    CodeGenTest() : CI(*TestUtils::CreateCompilerInstance()),
                    CG(TestUtils::CreateCodeGen(CI)),
                    Diags(CI.getDiagnostics()),
					Builder(new ASTBuilder(Diags)),
                    S(new Sema(CI.getDiagnostics())),
                    VoidTypeRef(Builder->CreateVoidType(SourceLoc)),
                    BoolTypeRef(Builder->CreateBoolType(SourceLoc)),
                    ByteTypeRef(Builder->CreateByteType(SourceLoc)),
                    ShortTypeRef(Builder->CreateShortType(SourceLoc)),
                    UShortTypeRef(Builder->CreateUShortType(SourceLoc)),
                    IntTypeRef(Builder->CreateIntType(SourceLoc)),
                    UIntTypeRef(Builder->CreateUIntType(SourceLoc)),
                    LongTypeRef(Builder->CreateLongType(SourceLoc)),
                    ULongTypeRef(Builder->CreateULongType(SourceLoc)),
                    FloatTypeRef(Builder->CreateFloatType(SourceLoc)),
                    DoubleTypeRef(Builder->CreateDoubleType(SourceLoc)),
                    ErrorTypeRef(Builder->CreateErrorType(SourceLoc)),
					StringTypeRef(Builder->CreateStringType(SourceLoc)) {
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();
    }

    ASTModule *CreateModule(std::string Name = "test") {
        Diags.getClient()->BeginSourceFile();
    	auto Buffer = llvm::MemoryBuffer::getMemBuffer("", Name);
    	auto FID = new InputFile(Diags, CI.getSourceManager(), Name);
        auto AST = Builder->CreateModule(FID);
        Diags.getClient()->EndSourceFile();
    	ASTModules.push_back(AST);
        return AST;
    }

    virtual ~CodeGenTest() {
        llvm::outs().flush();
    	ASTModules.clear();
    	delete Builder;
    	delete S;
    	delete CG;
    }

	ASTBuilder &getASTBuilder() {
	    return *Builder;
    }

	ASTType *CreateType(ASTClass *Class) {
    	llvm::SmallVector<ASTName *, 4> Names;
    	Names.push_back(Builder->CreateName(Class->getName(), Class->getLocation()));
    	return Builder->CreateType(Class->getLocation(), Names);
    }

	ASTType *CreateType(ASTEnum *Enum) {
    	llvm::SmallVector<ASTName *, 4> Names;
    	Names.push_back(Builder->CreateName(Enum->getName(), Enum->getLocation()));
    	return Builder->CreateType(Enum->getLocation(), Names);
    }

	std::vector<llvm::Module *> &Generate() {
    	// validate and resolve
    	SmallVector<SemaModule *, 8> SemaModules = S->Resolve(ASTModules);
    	EXPECT_FALSE(Diags.hasErrorOccurred());
    	EXPECT_FALSE(SemaModules.empty());
    	std::vector<llvm::Module *> CGModules = CG->GenerateModules(SemaModules);
    	EXPECT_FALSE(Diags.hasErrorOccurred());
    	return CGModules;
    }

	std::string getOutput(llvm::Module *M) {
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

	std::string getOutput(SymbolTableList<Function> &Functions) {
    	testing::internal::CaptureStdout();
		for (auto &F : Functions) {
			verifyFunction(F);
			F.print(llvm::outs());
		}
    	return testing::internal::GetCapturedStdout();
	}


};

#endif //FLY_CODEGENTEST_H
