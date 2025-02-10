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
#include "Sema/SemaBuilder.h"
#include "Sema/Sema.h"
#include "Sema/SemaBuilderScopes.h"
#include "AST/ASTCall.h"
#include "AST/ASTFunction.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTValue.h"
#include "AST/ASTExpr.h"

// third party
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
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
                    VoidType(Builder.CreateVoidType(SourceLoc)),
                    BoolType(Builder.CreateBoolType(SourceLoc)),
                    ByteType(Builder.CreateByteType(SourceLoc)),
                    ShortType(Builder.CreateShortType(SourceLoc)),
                    UShortType(Builder.CreateUShortType(SourceLoc)),
                    IntType(Builder.CreateIntType(SourceLoc)),
                    UIntType(Builder.CreateUIntType(SourceLoc)),
                    LongType(Builder.CreateLongType(SourceLoc)),
                    ULongType(Builder.CreateULongType(SourceLoc)),
                    FloatType(Builder.CreateFloatType(SourceLoc)),
                    DoubleType(Builder.CreateDoubleType(SourceLoc)),
                    ArrayInt0Type(Builder.CreateArrayType(SourceLoc, IntType,
                                                          Builder.CreateExpr(Builder.CreateIntegerValue(SourceLoc, "0")))),
                    ErrorType(Builder.CreateErrorType(SourceLoc)),
                    TopScopes(SemaBuilderScopes::Create()
                                      ->addVisibility(SourceLocation(), ASTVisibilityKind::V_DEFAULT)->getScopes()),
                    EmptyScopes(SemaBuilderScopes::Create()->getScopes()) {
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();
    }

    ASTModule *CreateModule(std::string Name = "test") {
        Diags.getClient()->BeginSourceFile();
        auto *Module = Builder.CreateModule(Name);
        Diags.getClient()->EndSourceFile();
        return Module;
    }

    virtual ~CodeGenTest() {
        llvm::outs().flush();
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
        ASTRef *Identifier = Builder.CreateIdentifier(SourceLoc, Var->getName());
        return Builder.CreateVarRef(Identifier, Parent);
    }

    ASTCall *CreateCall(ASTFunction *Function, llvm::SmallVector<ASTExpr *, 8> &Args, ASTRef *Parent = nullptr) {
        ASTRef *Identifier = Builder.CreateIdentifier(SourceLoc, Function->getName());
        ASTCall *Call = Builder.CreateCall(Identifier, Args, ASTCallKind::CALL_FUNCTION, Parent);
        return Call;
    }

    ASTCall *CreateCall(ASTClassMethod *Function, llvm::SmallVector<ASTExpr *, 8> &Args, ASTRef *Parent = nullptr) {
        ASTRef *Identifier = Builder.CreateIdentifier(SourceLoc, Function->getName());
        ASTCall *Call = Builder.CreateCall(Identifier, Args, ASTCallKind::CALL_FUNCTION, Parent);
        return Call;
    }

    ASTCall *CreateNew(ASTClassMethod *Function, llvm::SmallVector<ASTExpr *, 8> &Args, ASTRef *Parent = nullptr) {
        ASTRef *Identifier = Builder.CreateIdentifier(SourceLoc, Function->getName());
        ASTCall *Call = Builder.CreateCall(Identifier, Args, ASTCallKind::CALL_NEW, Parent);
        return Call;
    }

};

#endif //FLY_CODEGENTEST_H
