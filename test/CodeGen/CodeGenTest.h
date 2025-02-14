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
#include "Sema/Sema.h"
#include "Sema/SemaBuilderScopes.h"
#include "AST/ASTCall.h"
#include "AST/ASTFunction.h"
#include "AST/ASTValue.h"
#include "AST/ASTExpr.h"

// third party
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"

#include <AST/ASTScopes.h>
#include <AST/ASTVar.h>
#include <gtest/gtest.h>
#include <Sema/ASTBuilder.h>
#include <Sema/SymBuilder.h>

using namespace fly;

// The test fixture.
class CodeGenTest : public ::testing::Test {

public:
    const CompilerInstance CI;
    CodeGen *CG;
    DiagnosticsEngine &Diags;
    Sema *S;
    SourceLocation SourceLoc;
    ASTBuilder &Builder;
    ASTTypeRef *VoidType;
    ASTTypeRef *BoolType;
    ASTTypeRef *ByteType;
    ASTTypeRef *ShortType;
    ASTTypeRef *UShortType;
    ASTTypeRef *IntType;
    ASTTypeRef *UIntType;
    ASTTypeRef *LongType;
    ASTTypeRef *ULongType;
    ASTTypeRef *FloatType;
    ASTTypeRef *DoubleType;
    ASTTypeRef *ArrayInt0Type;
    ASTTypeRef *ErrorType;
    llvm::SmallVector<ASTScope *, 8> TopScopes;
    llvm::SmallVector<ASTScope *, 8> EmptyScopes;
    llvm::SmallVector<ASTExpr *, 8> Args;
    llvm::SmallVector<ASTVar *, 8> Params;

    CodeGenTest() : CI(*TestUtils::CreateCompilerInstance()),
                    CG(TestUtils::CreateCodeGen(CI)),
                    Diags(CI.getDiagnostics()),
                    S(Sema::CreateSema(CI.getDiagnostics())),
                    Builder(S->getASTBuilder()),
                    VoidType(S->getASTBuilder().CreateVoidTypeRef(SourceLoc)),
                    BoolType(S->getASTBuilder().CreateBoolTypeRef(SourceLoc)),
                    ByteType(S->getASTBuilder().CreateByteTypeRef(SourceLoc)),
                    ShortType(S->getASTBuilder().CreateShortTypeRef(SourceLoc)),
                    UShortType(S->getASTBuilder().CreateUShortTypeRef(SourceLoc)),
                    IntType(S->getASTBuilder().CreateIntTypeRef(SourceLoc)),
                    UIntType(S->getASTBuilder().CreateUIntTypeRef(SourceLoc)),
                    LongType(S->getASTBuilder().CreateLongTypeRef(SourceLoc)),
                    ULongType(S->getASTBuilder().CreateULongTypeRef(SourceLoc)),
                    FloatType(S->getASTBuilder().CreateFloatTypeRef(SourceLoc)),
                    DoubleType(S->getASTBuilder().CreateDoubleTypeRef(SourceLoc)),
                    ArrayInt0Type(S->getASTBuilder().CreateArrayTypeRef(SourceLoc, IntType,
                                                          Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, "0")))),
                    ErrorType(Builder.CreateErrorTypeRef(SourceLoc)),
                    TopScopes(SemaBuilderScopes::Create()
                                      ->addVisibility(SourceLocation(), ASTVisibilityKind::V_DEFAULT)->getScopes()),
                    EmptyScopes(SemaBuilderScopes::Create()->getScopes()) {
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();
    }

    SymModule *CreateModule(std::string Name = "test") {
        Diags.getClient()->BeginSourceFile();
        auto AST = Builder.CreateModule(Name);
    	SymModule *Module = S->getSymBuilder().CreateModule(AST);
        Diags.getClient()->EndSourceFile();
        return Module;
    }

    virtual ~CodeGenTest() {
        llvm::outs().flush();
    }

	ASTBuilder &getASTBuilder() {
	    return Builder;
    }

    std::string getOutput(llvm::Module *Module) {
        testing::internal::CaptureStdout();
        Module->print(llvm::outs(), nullptr);
        std::string output = testing::internal::GetCapturedStdout();
        output.erase(0, output.find("\n") + 1);
        output.erase(0, output.find("\n") + 1);
        output.erase(0, output.find("\n") + 1);
        output.erase(0, output.find("\n") + 2);
        return output;
    }

    ASTVarRef *CreateVarRef(ASTVar *Var, ASTRef *Parent = nullptr) {
        return Builder.CreateVarRef(Var, Parent);
    }

	ASTCall *CreateCall(llvm::StringRef Name, llvm::SmallVector<ASTExpr *, 8> &Args, ASTCallKind Kind, ASTRef *Parent = nullptr) {
    	ASTCall *Call = Builder.CreateCall(SourceLocation(), Name, Args, Kind, Parent);
    	return Call;
    }

    ASTCall *CreateCall(ASTFunction *Function, llvm::SmallVector<ASTExpr *, 8> &Args, ASTCallKind Kind, ASTRef *Parent = nullptr) {
        ASTCall *Call = Builder.CreateCall(SourceLocation(), Function->getName(), Args, Kind, Parent);
        return Call;
    }

};

#endif //FLY_CODEGENTEST_H
