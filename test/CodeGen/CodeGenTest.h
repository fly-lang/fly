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

#include <CodeGen/CodeGenModule.h>
#include <Sema/SemaBuiltin.h>
#include <Sema/SemaBuiltinType.h>
#include <Sema/SymbolTable.h>
#include <gtest/gtest.h>

using namespace fly;

// The test fixture.
class CodeGenTest : public ::testing::Test {

public:
    const CompilerInstance CI;
    CodeGen *CG;
    DiagnosticsEngine &Diags;
	ASTBuilder *Builder;
    Sema *S;
    SourceLocation SourceLoc;
    ASTTypeRef *VoidTypeRef;
    ASTTypeRef *BoolTypeRef;
    ASTTypeRef *ByteTypeRef;
    ASTTypeRef *ShortTypeRef;
    ASTTypeRef *UShortTypeRef;
    ASTTypeRef *IntTypeRef;
    ASTTypeRef *UIntTypeRef;
    ASTTypeRef *LongTypeRef;
    ASTTypeRef *ULongTypeRef;
    ASTTypeRef *FloatTypeRef;
    ASTTypeRef *DoubleTypeRef;
    ASTTypeRef *ErrorTypeRef;
	ASTTypeRef *StringTypeRef;
	ASTTypeRef *CharTypeRef;
    llvm::SmallVector<ASTModifier *, 8> TopModifiers;
    llvm::SmallVector<ASTModifier *, 8> EmptyModifiers;
    llvm::SmallVector<ASTExpr *, 8> Args;
    llvm::SmallVector<ASTVar *, 8> Params;

    CodeGenTest() : CI(*TestUtils::CreateCompilerInstance()),
                    CG(TestUtils::CreateCodeGen(CI)),
                    Diags(CI.getDiagnostics()),
					Builder(new ASTBuilder(Diags)),
                    S(new Sema(CI.getDiagnostics())),
                    VoidTypeRef(Builder->CreateVoidTypeRef(SourceLoc)),
                    BoolTypeRef(Builder->CreateBoolTypeRef(SourceLoc)),
                    ByteTypeRef(Builder->CreateByteTypeRef(SourceLoc)),
                    ShortTypeRef(Builder->CreateShortTypeRef(SourceLoc)),
                    UShortTypeRef(Builder->CreateUShortTypeRef(SourceLoc)),
                    IntTypeRef(Builder->CreateIntTypeRef(SourceLoc)),
                    UIntTypeRef(Builder->CreateUIntTypeRef(SourceLoc)),
                    LongTypeRef(Builder->CreateLongTypeRef(SourceLoc)),
                    ULongTypeRef(Builder->CreateULongTypeRef(SourceLoc)),
                    FloatTypeRef(Builder->CreateFloatTypeRef(SourceLoc)),
                    DoubleTypeRef(Builder->CreateDoubleTypeRef(SourceLoc)),
                    ErrorTypeRef(Builder->CreateErrorTypeRef(SourceLoc)),
					StringTypeRef(Builder->CreateStringTypeRef(SourceLoc)) {
    	TopModifiers.push_back(getASTBuilder().CreateModifier(SourceLoc, ASTModifierKind::MOD_DEFAULT));
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();
    }

    ASTModule *CreateModule(std::string Name = "test") {
        Diags.getClient()->BeginSourceFile();
        auto AST = Builder->CreateModule(Name);
        Diags.getClient()->EndSourceFile();
        return AST;
    }

    virtual ~CodeGenTest() {
        llvm::outs().flush();
    }

	ASTBuilder &getASTBuilder() {
	    return *Builder;
    }

	ASTTypeRef * CreateArrayTypeRef(SemaType *T) {
    	SemaArrayType *A = SemaBuiltin::getArrayType(T);
    	return Builder->CreateTypeRef(SourceLoc, A);
    }

	ASTCall *CreateCall(llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args, ASTCallKind Kind, ASTRef *Parent = nullptr) {
    	ASTCall *Call = getASTBuilder().CreateCall(SourceLocation(), Name, Args, Kind, Parent);
    	return Call;
    }

    ASTCall *CreateCall(ASTFunction *Function, llvm::SmallVector<ASTExpr *, 8> &Args, ASTCallKind Kind, ASTRef *Parent = nullptr) {
        ASTCall *Call = getASTBuilder().CreateCall(SourceLocation(), Function->getName(), Args, Kind, Parent);
        return Call;
    }

	Module *Generate() {
    	CodeGenModule *CGM = CG->GenerateModule(S->getSymTable().getDefaultNameSpace());
    	CGM->GenAll();
    	Module * M = CGM->getModule();
    	EXPECT_FALSE(Diags.hasErrorOccurred());
    	return M;
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
