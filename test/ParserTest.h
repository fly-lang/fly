//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

// fly
#include "TestUtils.h"
#include "Parser/Parser.h"
#include "Sema/Sema.h"
#include "Sema/SemaBuilder.h"

// third party
#include <gtest/gtest.h>

#ifndef FLY_PARSERTEST_H
#define FLY_PARSERTEST_H

using namespace fly;

class ParserTest : public ::testing::Test {

public:
    const CompilerInstance CI;
    ASTContext *Context;
    Sema *S;
    DiagnosticsEngine &Diags;
    bool Success = false;

    ParserTest() : CI(*TestUtils::CreateCompilerInstance()),
                   Diags(CI.getDiagnostics()) {
        S = Sema::CreateSema(CI.getDiagnostics());
    }

    virtual ~ParserTest() {
        delete S;
    }

    ASTModule *Parse(std::string FileName, llvm::StringRef Source) {
        Diags.getClient()->BeginSourceFile();
        InputFile Input(Diags, CI.getSourceManager(), FileName);
        Input.Load(Source);
        Parser *P = new Parser(Input, CI.getSourceManager(), Diags, S->getBuilder());
        ASTModule *M = P->ParseModule();
        Diags.getClient()->EndSourceFile();
        return M;
    }

    bool Resolve() {
        return S->Resolve();
    }

    bool isSuccess() const {
        return Success && !Diags.hasErrorOccurred();
    }

};

#endif //FLY_PARSERTEST_H
