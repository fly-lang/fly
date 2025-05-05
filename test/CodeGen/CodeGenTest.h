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
#include "Sema/SemaBuilderScopes.h"
#include "AST/ASTCall.h"
#include "AST/ASTFunction.h"
#include "AST/ASTValue.h"
#include "AST/ASTExpr.h"
#include <AST/ASTScopes.h>
#include <AST/ASTVar.h>
#include <Sema/ASTBuilder.h>
#include <Sema/SymBuilder.h>
#include "Sym/SymType.h"


// third party
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetSelect.h"

#include <CodeGen/CodeGenModule.h>
#include <gtest/gtest.h>

using namespace fly;

// The test fixture.
class CodeGenTest : public ::testing::Test {

public:
    const CompilerInstance CI;
    CodeGen *CG;
    DiagnosticsEngine &Diags;
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
    llvm::SmallVector<ASTScope *, 8> TopScopes;
    llvm::SmallVector<ASTScope *, 8> EmptyScopes;
    llvm::SmallVector<ASTExpr *, 8> Args;
    llvm::SmallVector<ASTVar *, 8> Params;

    CodeGenTest() : CI(*TestUtils::CreateCompilerInstance()),
                    CG(TestUtils::CreateCodeGen(CI)),
                    Diags(CI.getDiagnostics()),
                    S(Sema::CreateSema(CI.getDiagnostics())),
                    VoidTypeRef(S->getASTBuilder().CreateVoidTypeRef(SourceLoc)),
                    BoolTypeRef(S->getASTBuilder().CreateBoolTypeRef(SourceLoc)),
                    ByteTypeRef(S->getASTBuilder().CreateByteTypeRef(SourceLoc)),
                    ShortTypeRef(S->getASTBuilder().CreateShortTypeRef(SourceLoc)),
                    UShortTypeRef(S->getASTBuilder().CreateUShortTypeRef(SourceLoc)),
                    IntTypeRef(S->getASTBuilder().CreateIntTypeRef(SourceLoc)),
                    UIntTypeRef(S->getASTBuilder().CreateUIntTypeRef(SourceLoc)),
                    LongTypeRef(S->getASTBuilder().CreateLongTypeRef(SourceLoc)),
                    ULongTypeRef(S->getASTBuilder().CreateULongTypeRef(SourceLoc)),
                    FloatTypeRef(S->getASTBuilder().CreateFloatTypeRef(SourceLoc)),
                    DoubleTypeRef(S->getASTBuilder().CreateDoubleTypeRef(SourceLoc)),
                    ErrorTypeRef(S->getASTBuilder().CreateErrorTypeRef(SourceLoc)),
					StringTypeRef(S->getASTBuilder().CreateStringTypeRef(SourceLoc)),
					CharTypeRef(S->getASTBuilder().CreateCharTypeRef(SourceLoc)),
                    TopScopes(SemaBuilderScopes::Build()
                              ->addVisibility(SourceLocation(), ASTVisibilityKind::V_DEFAULT)->getScopes()),
                    EmptyScopes(SemaBuilderScopes::Build()->getScopes()) {
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();
    }

    ASTModule *CreateModule(std::string Name = "test") {
        Diags.getClient()->BeginSourceFile();
        auto AST = S->getASTBuilder().CreateModule(Name);
    	//SymModule *Module = S->getSymBuilder().CreateModule(AST);
        Diags.getClient()->EndSourceFile();
        return AST;
    }

    virtual ~CodeGenTest() {
        llvm::outs().flush();
    }

	ASTBuilder &getASTBuilder() {
	    return S->getASTBuilder();
    }

	ASTTypeRef * CreateArrayTypeRef(SymType *T) {
    	SymTypeArray *A = S->getSymBuilder().CreateArrayType(T);
    	return S->getASTBuilder().CreateTypeRef(SourceLoc, A);
    }

    ASTVarRef *CreateVarRef(ASTVar *Var, ASTRef *Parent = nullptr) {
        return getASTBuilder().CreateVarRef(Var, Parent);
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
